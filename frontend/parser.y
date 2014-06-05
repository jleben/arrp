%filenames parser
%implementation-header parser_impl.hpp
%scanner scanner.h
%namespace stream

%token SCANNER_ERROR
%token INT REAL ID INDEX
%token FOR EACH EVERY AT IN

%left '='
%left EQ NEQ LESS MORE
%left '+' '-'
%left '*' '/'

%%

/*
TODO: extend 'for' to express reduction
Perhaps use 'map' and 'reduce' instead of 'for'?
Well, 'for' with multi-one-single-one function is already a kind of reduction.
*/

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
  expr
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

atom: INT | REAL | call | for
;

call:
  ID
  |
  ID '(' arg_list ')'
;

arg_list:
  arg_list ',' expr
  |
  expr
  |
  // empty
;

for:
  FOR '(' for_spec_list ')' '{' for_body '}'
;

for_spec_list:
  for_spec_list ';' for_spec
  |
  for_spec_list ';'
  |
  for_spec
;

for_spec:
  ID '=' for_iter_spec IN for_domain
  |
  for_iter_spec IN for_domain
;

for_iter_spec:
  EACH for_iter_size for_iter_hop for_iter_offset
;

for_iter_size:
  // empty
  |
  INT
;

for_iter_hop:
  // empty
  |
  EVERY INT
  |
  EVERY ID
;

for_iter_offset:
  // empty
  |
  AT INT
  |
  AT ID
;

for_domain: stream_domain | INT
;

stream_domain:
  ID
  |
  ID '{' INT '}'
;

for_body:
  expr
;
