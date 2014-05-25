%filenames parser
%implementation-header parser_impl.hpp
%scanner scanner.h
%namespace stream

%token SCANNER_ERROR
%token INT REAL ID
%token FOR EACH EVERY AT IN

%left '='
%left COMPARE EQ NEQ LESS MORE
%left MATH1 '+' '-'
%left MATH2 '*' '/'

%%

program: func_def_list
;

func_def_list:
  // empty
  |
  func_def
  |
  func_def ';' func_def_list
;

func_def:
  ID func_param_spec '=' func_body
;

func_param_spec:
  // empty
  |
  '(' func_param_list ')'
;

func_param_list:
  ID
  |
  ID ',' func_param_list
;

func_body:
  expr | for_stmt
;

expr:
  expr EQ expr
  |
  expr NEQ expr
  |
  expr LESS expr
  |
  expr MORE expr
  |
  expr '+' expr
  |
  expr '-' expr
  |
  expr '*' expr
  |
  expr '/' expr
  |
  '(' expr ')'
  |
  atom
;

atom: INT | REAL | ID
;

for_stmt:
  FOR '(' for_spec ')' '{' for_body '}'
;

for_spec:
  for_iter_spec for_domain_spec
;

for_iter_spec:
  EACH ID
  |
  INT ID for_iter_hop for_iter_offset
;

for_iter_hop:
  // empty
  |
  EVERY INT
;

for_iter_offset:
  // empty
  |
  AT INT
;

for_domain_spec:
  // empty
  |
  IN for_domain_list
;

for_domain_list:
  ID
  |
  ID ',' for_domain_list
;

for_body:
  expr
  |
  for_stmt
;
