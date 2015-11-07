%filenames = scanner
%implementation-header = "scanner_impl.hpp"
%namespace stream

%x one_line_comment
%x multi_line_comment

%%

"//" { begin(StartCondition__::one_line_comment); }
<one_line_comment>{
\n { begin(StartCondition__::INITIAL); }
.
}

"/*" { begin(StartCondition__::multi_line_comment); }
<multi_line_comment>{
"*/" { begin(StartCondition__::INITIAL); }
\n
.
}

[ \t\n]+ // Skip whitespace

"->" { return Parser::RIGHT_ARROW; }
\\ { return '\\'; }
\{ { return '{'; }
\} { return '}'; }
\[ { return '['; }
\] { return ']'; }
\( { return '('; }
\) { return ')'; }
\. { return '.'; }
, { return ','; }
; { return ';'; }
: { return ':'; }
\? { return '?'; }
\+ { return '+'; }
\- { return '-'; }
\* { return '*'; }
\/ { return '/'; }
\^ { return '^'; }
\% { return '%'; }
= { return '='; }
# { return '#'; }
"==" { return Parser::EQ; }
"!=" { return Parser::NEQ; }
"<" { return Parser::LESS; }
">" { return Parser::MORE; }
"<=" { return Parser::LESS_EQ; }
">=" { return Parser::MORE_EQ; }
"not" { return Parser::LOGIC_NOT; }
"or" { return Parser::LOGIC_OR; }
"and" { return Parser::LOGIC_AND; }

"true" { return Parser::TRUE; }
"false" { return Parser::FALSE; }
"if" { return Parser::IF; }
"then" { return Parser::THEN; }
"else" { return Parser::ELSE; }
"let" { return Parser::LET; }
".." { return Parser::DOTDOT; }

[0-9]+\.[0-9]+ { return Parser::REAL; }

[0-9]+ { return Parser::INT; }

[[:alpha:]_]+[[:alpha:][:digit:]_]* { return Parser::ID; }

. {
  /*std::cerr << "ERROR:" << lineNr()
  << ": Unrecognized text: " << matched() << std::endl;*/
  return Parser::SCANNER_ERROR;
}
