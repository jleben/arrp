%{
#include "scanner_internal.hpp"
%}

%option c++
%option prefix="StreamLang"

%option batch
%option yywrap nounput

id    [a-zA-Z][a-zA-Z_0-9]*
int   [0-9]+
blank [ \t]

%{
  // Code run each time a pattern is matched.
  # define YY_USER_ACTION  yylloc->columns (yyleng);
%}

%%

%{
  // Code run each time yylex is called.
  yylloc->step ();
%}

{blank}+   yylloc->step ();
[\n]+      yylloc->lines (yyleng); yylloc->step ();
##[^\n]*\n    yylloc->lines (1); yylloc->step ();

\\ { return (parser::token_type) '\\'; }
\{ { return (parser::token_type) '{'; }
\} { return (parser::token_type) '}'; }
\[ { return (parser::token_type) '['; }
\] { return (parser::token_type) ']'; }
\( { return (parser::token_type) '('; }
\) { return (parser::token_type) ')'; }
\. { return (parser::token_type) '.'; }
, { return (parser::token_type) ','; }
; { return (parser::token_type) ';'; }
: { return (parser::token_type) ':'; }
\? { return (parser::token_type) '?'; }
\+ { return (parser::token_type) '+'; }
\- { return (parser::token_type) '-'; }
\* { return (parser::token_type) '*'; }
\/ { return (parser::token_type) '/'; }
\^ { return (parser::token_type) '^'; }
\% { return (parser::token_type) '%'; }
= { return (parser::token_type) '='; }
# { return (parser::token_type) '#'; }
@ { return (parser::token_type) '@'; }
"//" { return parser::token::INT_DIV; }
"->" { return parser::token::RIGHT_ARROW; }
"==" { return parser::token::EQ; }
"!=" { return parser::token::NEQ; }
"<" { return parser::token::LESS; }
">" { return parser::token::MORE; }
"<=" { return parser::token::LESS_EQ; }
">=" { return parser::token::MORE_EQ; }
"not" { return parser::token::LOGIC_NOT; }
"or" { return parser::token::LOGIC_OR; }
"and" { return parser::token::LOGIC_AND; }
"if" { return parser::token::IF; }
"then" { return parser::token::THEN; }
"else" { return parser::token::ELSE; }
"case" { return parser::token::CASE; }
"let" { return parser::token::LET; }
".." { return parser::token::DOTDOT; }
"++" { return parser::token::PLUSPLUS; }
"true" {
  *yylval = ast::make_const( *yylloc, true );
  return parser::token::TRUE;
}
"false" {
  *yylval = ast::make_const( *yylloc, false );
  return parser::token::FALSE;
}
"this" {
  *yylval = ast::make_node(ast::array_self_ref, *yylloc);
  return parser::token::THIS;
}
"module" { return parser::token::MODULE; }
"import" { return parser::token::IMPORT; }
"as" { return parser::token::AS; }
[0-9]+\.[0-9]+ {
  *yylval = ast::make_const( *yylloc, std::stod(yytext) );
  return parser::token::REAL;
}
[0-9]+ {
  *yylval = ast::make_const( *yylloc, std::stoi(yytext) );
  return parser::token::INT;
}
[[:alpha:]_]+[[:alpha:][:digit:]_]* {
  *yylval = ast::make_id( *yylloc, yytext );
  return parser::token::ID;
}
\"[^"\n]\" {
  *yylval = ast::make_const( *yylloc, std::string(yytext) );
  return parser::token::STRING;
}

. { return parser::token::INVALID; }
