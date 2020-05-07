%{
#include "scanner_internal.hpp"
#include <complex>
%}

%option c++
%option prefix="StreamLang"

%option batch
%option yywrap nounput

id    [[:alpha:]]+[[:alpha:][:digit:]_]*
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
"..."[^\n]*\n    yylloc->lines (1); yylloc->step ();
"--"[^\n]*\n    yylloc->lines (1); yylloc->step ();

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
\| { return (parser::token_type) '|'; }
\? { return (parser::token_type) '?'; }
\+ { return (parser::token_type) '+'; }
\- { return (parser::token_type) '-'; }
\* { return (parser::token_type) '*'; }
\/ { return (parser::token_type) '/'; }
\^ { return (parser::token_type) '^'; }
\% { return (parser::token_type) '%'; }
~ { return (parser::token_type) '~'; }
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
".not" { return parser::token::BIT_NOT; }
".and" { return parser::token::BIT_AND; }
".or" { return parser::token::BIT_OR; }
".xor" { return parser::token::BIT_XOR; }
".<<" { return parser::token::BIT_SHIFT_LEFT; }
".>>" { return parser::token::BIT_SHIFT_RIGHT; }
"if" { return parser::token::IF; }
"then" { return parser::token::THEN; }
"else" { return parser::token::ELSE; }
"case" { return parser::token::CASE; }
"let" { return parser::token::LET; }
"in" { return parser::token::IN; }
"where" { return parser::token::WHERE; }
"otherwise" { return parser::token::OTHERWISE; }
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
"module" { return parser::token::MODULE; }
"import" { return parser::token::IMPORT; }
"as" { return parser::token::AS; }
"input" { return parser::token::INPUT; }
"output" { return parser::token::OUTPUT; }
"external" { return parser::token::EXTERNAL; }

[0-9]+(\.[0-9]+)?i {
  double im = std::stod(std::string(yytext,strlen(yytext)-1));
  std::complex<double> v(0, im);
  *yylval = ast::make_const( *yylloc, v );
  return parser::token::COMPLEX;
}
[0-9]+\.[0-9]+ {
  *yylval = ast::make_const( *yylloc, std::stod(yytext) );
  return parser::token::REAL;
}
[0-9]+ {
  try {
    *yylval = ast::make_const( *yylloc, std::uint64_t(std::stoull(yytext)) );
  } catch (std::out_of_range &) {
    // FIXME: report error: value is too large.
    throw;
  }
  return parser::token::INT;
}
{id}\.{id} {
  *yylval = ast::make_qualified_id( *yylloc, yytext );
  return parser::token::QUALIFIED_ID;
}
{id} {
  *yylval = ast::make_id( *yylloc, yytext );
  return parser::token::ID;
}
\"[^"\n]\" {
  *yylval = ast::make_const( *yylloc, std::string(yytext) );
  return parser::token::STRING;
}

. { return parser::token::INVALID; }
