#include "scanner.hpp"
#include "driver.hpp"

#define YY_DECL                                         \
parser::token_type                         \
scanner::lex(                              \
parser::semantic_type* yylval,         \
parser::location_type* yylloc          \
)

using namespace stream::parsing;

#define yyterminate() return parser::token::END

#define YY_NO_UNISTD_H

// Should never be called (parser calls scanner::lex instead)
int StreamLangFlexLexer::yylex()
{
    std::cerr << "StreamLangFlexLexer::yylex called!" << std::endl;
    return 0;
}

// Called by scanner at EOF.
// Returning 1 means there is no next file, and the scanner will terminate.
int StreamLangFlexLexer::yywrap()
{
    return 1;
}
