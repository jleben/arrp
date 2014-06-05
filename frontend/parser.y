%filenames parser
%implementation-header parser_impl.hpp
%scanner scanner.h
%namespace stream

%token SCANNER_ERROR
%token INT REAL ID
%token LET REDUCE FOR EACH TAKES EVERY IN

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

program: stmt_list
;

stmt_list:
  stmt
  |
  stmt_list ';' stmt
;

stmt:
  // empty
  |
  assignment
;

assignment:
  ID '(' param_list ')' '=' assignment_body
  |
  ID '=' assignment_body
;

param_list:
  // empty
  |
  ID
  |
  param_list ',' ID
;

assignment_body:
  complex_expr
  |
  '{' complex_expr '}'
  |
  '{' let_stmt_list ';' complex_expr '}'
;

let_stmt_list:
  let_stmt
  |
  let_stmt_list ';' let_stmt
;

let_stmt:
  // empty
  |
  LET stmt
  |
  LET '{' stmt_list '}'
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
  hash // get index of iterator or size of stream
  |
  range
  |
  call
  |
  literal
;

hash:
  '#' ID // implied dimension 1
  |
  '#' '(' ID ')' // implied dimension 1
  |
  '#' '(' ID ',' expr ')' // second arg = dimension
;

simple_expr:
  expr
;

complex_expr:
  simple_expr
  |
  for
  |
  reduce
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
  ID call_args call_dim call_range
;

call_args:
  // empty
  |
  '(' arg_list ')'
;

call_dim:
  // empty
  |
  '{' call_dim_list '}'
;

call_dim_list:
  '.'
  |
  INT
  |
  call_dim_list ',' INT
;

call_range:
  // empty
  |
  '[' call_range_list ']'
;

call_range_list:
  expr
  |
  call_range_list ',' expr
;

arg_list:
  arg_list ',' complex_expr
  |
  complex_expr
  |
  // empty
;

for:
  FOR EACH '(' for_spec_list ')' for_body
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
  assignment_body
;

reduce:
  REDUCE '(' ID ',' ID IN call ')' reduce_body
;

reduce_body: assignment_body
;

int_or_id: INT | ID
;
