#ifndef STREAM_LANG_PARSING_SCANNER_INCLUDED
#define STREAM_LANG_PARSING_SCANNER_INCLUDED

#include "parser.hpp"

#ifndef yyFlexLexer
#define yyFlexLexer StreamLangFlexLexer
#include <FlexLexer.h>
#undef yyFlexLexer
#endif

#include <iostream>

namespace stream {
namespace parsing {

class driver;

class scanner : public StreamLangFlexLexer
{
public:
    scanner(driver & d, std::istream & in, std::ostream & out):
        StreamLangFlexLexer(&in, &out), m_driver(d) {}

    parser::token_type
    lex(parser::semantic_type* yylval, parser::location_type* yylloc);

private:
    driver & m_driver;
};

}
}

#endif // STREAM_LANG_PARSING_SCANNER_INCLUDED
