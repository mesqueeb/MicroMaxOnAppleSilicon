#include <cctype>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <functional>
#include "ProtocolParser.hpp"

void ProtocolParser::parse(const std::string& line, Context& context)
{
    context.clear();
    std::string token;
    Tokens tokens;
    
    std::istringstream iss(line, std::istringstream::in);
    
    while (iss >> token)
    {
        tokens.push_back(token);
    }
    
    parseKeyword(tokens, context);
}

void ProtocolParser::parseKeyword(Tokens& tokens, Context& context)
{
    if (tokens.size() > 0)
    {
        const std::string keyword = tokens.front();
        
        if (_keywords.count(keyword) != 0 && _keywords[keyword] != NULL)
        {
            context["Keyword"] = keyword;
            tokens.pop_front();
            
            // Invoke keyword delegate
            _keywords[keyword](tokens, context);
                              
            // Parse remaining subkeywords
            parseSubKeyword(tokens, context);
        }
        else
        {
            parseUnknownLine(tokens, context);
        }
    }
}

void ProtocolParser::parseSubKeyword(Tokens& tokens, Context& context)
{
    while (tokens.size() > 0)
    {   
        std::string subKeyword = tokens.front();    
        tokens.pop_front();
        
        // Check if the first token is a keyword
        if (_subKeywords.count(subKeyword) != 0)
        {            
            if (_subKeywords[subKeyword] == NULL)
            {
                return;
            }
            
            // Invoke the keyword delegate
            _subKeywords[subKeyword](tokens, context);
        }
    }
}
