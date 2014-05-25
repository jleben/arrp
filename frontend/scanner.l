%filenames = scanner
%implementation-header = "scanner_impl.hpp"
%namespace stream

%x one_line_comment
%x multi_line_comment

%%

"//" { begin(StartCondition__::one_line_comment); }
<one_line_comment>{
\n { begin(StartCondition__::INITIAL); }
}

"/*" { begin(StartCondition__::multi_line_comment); }
<multi_line_comment>{
"*/" { begin(StartCondition__::INITIAL); }
}

[ \t\n]+ // Skip whitespace

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
\+ { return '+'; }
\- { return '-'; }
\* { return '*'; }
\/ { return '/'; }
= { return '='; }
"==" { return Parser::EQ; }
"!=" { return Parser::NEQ; }
"<" { return Parser::LESS; }
">" { return Parser::MORE; }

"for" { return Parser::FOR; }
"each" { return Parser::EACH; }
"every" { return Parser::EVERY; }
"at" { return Parser::AT; }
"in" { return Parser::IN; }

(\+|\-)?[0-9]+\.[0-9]* { return Parser::REAL; }

(\+|\-)?[0-9]+ { return Parser::INT; }

[[:alpha:]_]+[[:alpha:][:digit:]_]* { return Parser::ID; }


. {
  /*std::cerr << "ERROR:" << lineNr()
  << ": Unrecognized text: " << matched() << std::endl;*/
  return Parser::SCANNER_ERROR;
}
