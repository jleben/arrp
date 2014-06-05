%filenames parser
%implementation-header parser_impl.hpp
%scanner scanner.h
%namespace stream

%token SCANNER_ERROR
%token INT REAL ID
%token FOR EACH TAKES EVERY IN

%left '='
%left EQ NEQ LESS MORE
%left '+' '-'
%left '*' '/'
%left DOTDOT

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
  range
  |
  for
  |
  call
  |
  literal
;

literal: INT | REAL
;

range:
  expr DOTDOT expr
  |
  expr DOTDOT
  |
  DOTDOT expr
;

call:
  ID call_args call_range
;

call_args:
  // empty
  |
  '(' arg_list ')'
;

call_range:
  // empty
  |
  '[' call_range_list ']'
;

call_range_list:
  // empty
  |
  call_range_dim
  |
  call_range_list '|' call_range_dim
;

call_range_dim:
  '#' INT
  |
  '#' INT ':' expr
  |
  expr
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

for_domain: range | call
;

for_body:
  // empty
  |
  expr
;

int_or_id: INT | ID
;
