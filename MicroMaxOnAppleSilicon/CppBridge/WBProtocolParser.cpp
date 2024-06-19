#include "WBProtocolParser.hpp"

using namespace Framework;

static void parseMove(Tokens& tokens, Context& context)
{
    context["Move"] = tokens.front();
    tokens.pop_front();
}

WBProtocolParser::WBProtocolParser() : ProtocolParser()
{
    _keywords["Hint:"] = parseMove;
    _keywords["move"] = parseMove;
}
