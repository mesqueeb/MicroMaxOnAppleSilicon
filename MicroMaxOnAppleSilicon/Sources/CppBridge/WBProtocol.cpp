#include <cassert>
#include <sstream>
#include <iostream>
#include "FENParser.hpp"
#include "WBProtocol.hpp"

using namespace Framework;

void WBProtocol::open()
{
    _adapter->write("xboard");
}

void WBProtocol::close()
{
    _adapter->write("quit");
}

void WBProtocol::start()
{
    _state = ReadyState;
    _adapter->write("new");
    _adapter->write("force");
}

void WBProtocol::start(const std::string &state)
{
    start();
    update(state);
}

void WBProtocol::think(unsigned timeLimit)
{
    _state = WaitingForMoveState;
    _atom = _atomFactory.getCurrent();

    std::stringstream ss;
    ss << "st " << timeLimit;

    _adapter->write(ss.str());
    _adapter->write("go");
}

void WBProtocol::update(const std::string& state)
{
    if (_ignoreUpdate)
    {
        return;
    }

	std::map<std::pair<unsigned, unsigned>, char> placement;
	VirtualSquare enpassant;
    
    FENParser::parse(state, placement, enpassant);
    edit(placement, state);
}

void WBProtocol::update(unsigned srcFile, unsigned srcRank, unsigned dstFile, unsigned dstRank, unsigned promote)
{
    if (_state != IgnoreUpdateState)
    {
        char buffer[6];
        convertCoordToStr(buffer, srcFile, srcRank, dstFile, dstRank, promote);
        _adapter->write(buffer);
    }
}

void WBProtocol::edit(const std::map<std::pair<unsigned, unsigned>, char> &placement, const std::string &state)
{
    // It's important to revert the state so that we can ignore any move request we had sent
    if (_state == WaitingForMoveState)
    {
        _state = ReadyState;
    }

    _adapter->write("new");
    
    if (state.find('w') != std::string::npos)
    {
        _adapter->write("white");
    }
    else
    {
        _adapter->write("black");
    }

	_adapter->write("random");
	_adapter->write("force");
	_adapter->write("edit");
	_adapter->write("#");

    // Editing for the White positon
	for (std::map<std::pair<unsigned, unsigned>, char>::const_iterator iter = placement.begin(); iter != placement.end(); iter++)
	{
        if (isupper(iter->second))
        {
            std::stringstream ss;
            ss << (char) toupper(iter->second);
            ss << (char) ('a' + iter->first.first);
            ss << (char) ('1' + iter->first.second);
            //_adapter->write((boost::format("%1%%2%%3%") % (char) toupper(iter->second) % (char) ('a' + iter->first.first) % (char) ('1' + iter->first.second)).str());
            _adapter->write(ss.str());
        }
	}
    
	_adapter->write("c");    

    // Editing for the Black position
	for (std::map<std::pair<unsigned, unsigned>, char>::const_iterator iter = placement.begin(); iter != placement.end(); iter++)
	{
        if (islower(iter->second))
        {
            std::stringstream ss;
            ss << (char) toupper(iter->second);
            ss << (char) ('a' + iter->first.first);
            ss << (char) ('1' + iter->first.second);
            //_adapter->write((boost::format("%1%%2%%3%") % (char) toupper(iter->second) % (char) ('a' + iter->first.first) % (char) ('1' + iter->first.second)).str());
            _adapter->write(ss.str());
        }
	}

	_adapter->write(".");
	_adapter->write("force");
}

void WBProtocol::process(const std::string& line)
{
    _context.clear();
    _parser.parse(line, _context);

    if (_context.count("Keyword") != 0)
    {
        if (_state == WaitingForMoveState && _atomFactory.isCurrent(_atom) && (_context["Keyword"] == "Hint:" || _context["Keyword"] == "move"))
        {
            /*
             * Winboard is a stated protocol, it remembers the current position. The upper layer always update the protocol
             * in receiveMove(). We need to prevent it, and the simplest way would be a boolean.
             */

            // Block the execution in update()
            _state = IgnoreUpdateState;

            _ignoreUpdate = true;
            
            receiveMove(_context["Move"], "");
            assert(_state == IgnoreUpdateState);

            _ignoreUpdate = false;
            
            // Unblock the execution in update()
            _state = ReadyState;

            // Waiting for the user move
            _adapter->write("force");
        }
    }
}
