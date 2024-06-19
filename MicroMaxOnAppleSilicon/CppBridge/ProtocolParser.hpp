#ifndef PROTOCOL_PARSER_HPP
#define PROTOCOL_PARSER_HPP

#include <map>
#include <list>
#include <string>

// A keyword is simply a string
#define Keyword const std::string

// Tokens is a list of immutable strings
#define Tokens std::list<std::string>

// TokenFunction parses tokens into context
#define TokenFunction void (*)(Tokens& tokens, Context &context)

// Similarly to TokenFunction, but refers to exact matches
#define ExactMatchFunction void (*)(Context &context)

// KeywordMapping defines mapping between a keyword and its delegate
#define KeywordMapping std:: map<Keyword, TokenFunction>

// Similarly to KeywordMapping, but refers to exact matches
#define ExactMatchMapping std:: map<Keyword, ExactMatchFunction>

// Context maps from a keyword to its value
#define Context std::map<std::string, std::string>

#define FRONT_AND_POP(x) x.front(); x.pop_front();

class ProtocolParser
{
    public:
		void parse(const std::string& line, Context& context);

	protected:
		void parseKeyword(Tokens& tokens, Context& context);
		void parseSubKeyword(Tokens& tokens, Context& context);
        virtual void parseUnknownLine(Tokens& tokens, Context& context) {}

		KeywordMapping    _keywords;
		KeywordMapping    _subKeywords;
        ExactMatchMapping _exacts;
};

#endif