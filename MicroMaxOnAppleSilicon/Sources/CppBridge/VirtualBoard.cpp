#include "VirtualNode.hpp"
#include "VirtualBoard.hpp"
//#include "oreFactory.hpp"

using namespace Framework;

VirtualBoard::~VirtualBoard()
{
    detach();

//    if (_history)
    {
  //      delete _history;
    }
}

// Inherited From CoreReceiver
std::shared_ptr<Core> VirtualBoard::copy() const
{
    throw "Not Implemented";
//    std::shared_ptr<Core> core = CoreFactory::getCore();
///*
//    core->open();
//    core->start();
//    _history->copyTo(core->getHistory());
//    core->mo
//
//    assert(core->getHistory()->count() == _history->count());
//*/
//    return core;
}
