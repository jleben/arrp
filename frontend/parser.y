%filenames parser
%implementation-header parser_impl.hpp
%scanner scanner.h
%namespace stream

%token SCANNER_ERROR
%token INT REAL ID
%token FOR EACH TAKES EVERY IN DOTDOT

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
  FOR EACH '(' for_spec_list ')' '{' for_body '}'
;

for_spec_list:
  for_spec_list ';' for_spec
  |
  for_spec_list ';'
  |
  for_spec
;

for_spec:
  for_iterator for_size for_hop IN for_domain
;

for_iterator:
  //empty
  |
  ID
;

for_size:
  // empty
  |
  TAKES int_or_id
;

for_hop:
  // empty
  |
  EVERY int_or_id
;

for_domain:
  int_range
  |
  for_source
;

for_source:
  ID
  |
  ID '{' INT '}'
  |
  ID '{' INT ':' int_range '}'
;

for_body:
  expr
;

int_or_id: INT | ID
;

int_range:
  int_or_id DOTDOT int_or_id
  |
  int_or_id DOTDOT
  |
  DOTDOT int_or_id
;
