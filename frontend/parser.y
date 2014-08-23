%filenames parser
%implementation-header parser_impl.hpp
%baseclass-preinclude ast.hpp
%scanner scanner.h
%namespace stream

%token SCANNER_ERROR
%token INT REAL ID
%token LET REDUCE FOR EACH TAKES EVERY IN

%left '='
%left EQ NEQ LESS MORE
%left '+' '-'
%left '*' '/'
%left '^'
%left DOTDOT
%right '#'
%left '[' '{' '('

%stype ast::semantic_value

/*
%polymorphic
  PROGRAM: ast::program;
  STMT_LIST: ast::statement_list;
  STMT: ast::statement;
  ID_LIST: ast::id_list;
  EXPR_LIST: ast::expression_list;
  INT_LIST: ast::int_list;
  EXPR_BLOCK: ast::expression_block;
  EXPR: sp<ast::expression>;
  HASH: ast::hash_expression;
  //NUM_EXPR: ast::numerical_expression;
  RANGE: ast::range;
  //RANGE_LIST: ast::range_list;
  CALL: ast::call;
  FOR: ast::for_expression;
  FOR_ITER: ast::for_iteration;
  FOR_ITER_LIST: ast::for_iteration_list;
  REDUCE: ast::reduce_expression;
  INT: int;
  REAL: double;
  ID: ast::identifier;
  NODE: ast::node;

%type <STMT> stmt
%type <STMT_LIST> stmt_list, let_stmt_list, let_stmt
%type <EXPR> expr, simple_expr, complex_expr, number_expr, for_size, for_hop, for_domain
%type <EXPR_BLOCK> expr_block, reduce_body, for_body
%type <HASH> hash
%type <RANGE> range
//%type <RANGE_LIST> range_list
%type <CALL> call
%type <EXPR_LIST> call_args, complex_expr_list, simple_expr_list, call_range
%type <INT_LIST> int_list, call_dim
%type <ID_LIST> param_list
%type <FOR> for_expr
%type <FOR_ITER> for_spec
%type <FOR_ITER_LIST> for_spec_list
%type <REDUCE> reduce_expr
%type <ID> id for_iterator
%type <NODE> let for reduce
%type <INT> int
%type <REAL> real
*/

%%

/*
TODO: extend 'for' to express reduction
Perhaps use 'map' and 'reduce' instead of 'for'?
Well, 'for' with multi-one-single-one function is already a kind of reduction.
*/

program:
  stmt_list optional_semicolon
  {
    $$ = $1; $$->type = ast::program;
    d_ast = $$;
  }
  |
  // empty
  {
    $$ = new ast::list_node(ast::program, 0);
    d_ast = $$;
  }
;

stmt_list:
  stmt
  {
    $$ = new ast::list_node( ast::statement_list, $1->line, { $1 } );
  }
  |
  stmt_list ';' stmt
  {
    $$ = $1;
    $$.as<ast::list_node>()->append( $3 );
  }
;

stmt:
  id '(' param_list ')' '=' expr_block
  {
    $$ = new ast::list_node( ast::statement, $1->line, {$1, $3, $6} );
  }
  |
  id '=' expr_block
  {
    $$ = new ast::list_node( ast::statement, $1->line, {$1, nullptr, $3} );
  }
;

param_list:
  // empty
  { $$ = new ast::list_node( ast::id_list, d_scanner.lineNr() ); }
  |
  id
  { $$ = new ast::list_node( ast::id_list, d_scanner.lineNr(), {$1} ); }
  |
  param_list ',' id
  {
    $$ = $1;
    $$.as<ast::list_node>()->append( $3 );
  }
;

expr_block:
  complex_expr
  {
    $$ = new ast::list_node( ast::expression_block, $1->line, {nullptr, $1} );
  }
  |
  '{' complex_expr optional_semicolon '}'
  {
    $$ = new ast::list_node( ast::expression_block, $2->line, {nullptr, $2} );
  }
  |
  '{' let_block_list ';' complex_expr optional_semicolon '}'
  {
    $$ = new ast::list_node( ast::expression_block, $2->line, {$2, $4} );
  }
;

let_block_list:
  let_block
  |
  let_block_list ';' let_block
  {
    $$ = $1;
    $$.as<ast::list_node>()->append( $3.as<ast::list_node>()->elements );
  }
;

let_block:
  let stmt
  {
    $$ = new ast::list_node( ast::statement_list, $1->line, {$2} );
  }
  |
  let '{' stmt_list '}'
  {
    $$ = $3;
    $$->line = $1->line;
  }
;

let: LET { $$ = new ast::node( ast::kwd_let, d_scanner.lineNr() ); }
;

expr:
  expr EQ expr
  { $$ = new ast::binary_op_expression( $1, ast::equal, $3 ); }
  |
  expr NEQ expr
  { $$ = new ast::binary_op_expression( $1, ast::not_equal, $3 ); }
  |
  expr LESS expr
  { $$ = new ast::binary_op_expression( $1, ast::lesser, $3 ); }
  |
  expr MORE expr
  { $$ = new ast::binary_op_expression( $1, ast::greater, $3 ); }
  |
  expr '+' expr
  { $$ = new ast::binary_op_expression( $1, ast::add, $3 ); }
  |
  expr '-' expr
  { $$ = new ast::binary_op_expression( $1, ast::subtract, $3 ); }
  |
  expr '*' expr
  { $$ = new ast::binary_op_expression( $1, ast::multiply, $3 ); }
  |
  expr '/' expr
  { $$ = new ast::binary_op_expression( $1, ast::divide, $3 ); }
  |
  expr '^' expr
  { $$ = new ast::binary_op_expression( $1, ast::raise, $3 ); }
  |
  '(' expr ')'
  { $$ = $2; }
  |
  call
  |
  hash
  |
  transpose
  |
  slice
  |
  range
  |
  id
  |
  number
;

hash:
  '#' expr // implied dimension 1
  { $$ = new ast::list_node( ast::hash_expression, $2->line, {$2, nullptr} ); }
  |
  '#' '(' expr ')' // implied dimension 1
  { $$ = new ast::list_node( ast::hash_expression, $3->line, {$3, nullptr} ); }
  |
  '#' '(' expr ',' expr ')' // second arg = dimension
  { $$ = new ast::list_node( ast::hash_expression, $3->line, {$3, $5} ); }
;

simple_expr:
  expr
;

complex_expr:
  simple_expr
  |
  for_expr
  |
  reduce_expr
;

number:
  int
  |
  real
;

range:
  expr DOTDOT expr
  { $$ = new ast::list_node( ast::range, $1->line, {$1, $3} ); }
  |
  expr DOTDOT
  { $$ = new ast::list_node( ast::range, $1->line, {$1, nullptr} ); }
  |
  DOTDOT expr
  { $$ = new ast::list_node( ast::range, $2->line, {nullptr, $2} ); }
;

call:
  expr '(' complex_expr_list ')'
  {
    $$ = new ast::list_node( ast::call_expression, $1->line, {$1, $3} );
  }
;

transpose:
  expr '{' int_list '}'
  { $$ = new ast::list_node( ast::transpose_expression, $1->line, { $1, $3 } ); }
;

slice:
  expr '[' simple_expr_list ']'
  { $$ = new ast::list_node( ast::slice_expression, $1->line, { $1, $3 } ); }
;

int_list:
  int
  { $$ = new ast::list_node( ast::int_list, $1->line, {$1} ); }
  |
  int_list ',' int
  {
    $$ = $1;
    $$.as<ast::list_node>()->append( $3 );
  }
;

complex_expr_list:
  complex_expr
  { $$ = new ast::list_node( ast::expression_list, $1->line, {$1} ); }
  |
  complex_expr_list ',' complex_expr
  {
    $$ = $1;
    $$.as<ast::list_node>()->append( $3 );
  }
;

simple_expr_list:
  simple_expr
  { $$ = new ast::list_node( ast::expression_list, $1->line, {$1} ); }
  |
  simple_expr_list ',' simple_expr
  {
    $$ = $1;
    $$.as<ast::list_node>()->append( $3 );
  }
;

for: FOR
{ $$ = new ast::node( ast::kwd_for, d_scanner.lineNr() ); }
;

for_expr:
  for EACH '(' for_spec_list ')' for_body
  {
    $$ = new ast::list_node( ast::for_expression, $1->line, { $4, $6 } );
  }
;

for_spec_list:
  for_spec
  {
    $$ = new ast::list_node( ast::for_iteration_list, $1->line, {$1} );
  }
  |
  for_spec_list ';'
  { $$ = $1; }
  |
  for_spec_list ';' for_spec
  {
    $$ = $1;
    $$.as<ast::list_node>()->append( $3 );
  }
;

for_spec:
  for_iterator for_size for_hop IN for_domain
  {
    int line;
    if ($1)
      line = $1->line;
    else if ($2)
      line = $2->line;
    else if ($3)
      line = $3->line;
    else
      line = $5->line;

    $$ = new ast::list_node( ast::for_iteration, line, {$1,$2,$3,$5} );
  }
;

for_iterator:
  //empty
  |
  id
;

for_size:
  // empty
  |
  TAKES expr
  { $$ = $2; }
;

for_hop:
  // empty
  |
  EVERY expr
  { $$ = $2; }
;

for_domain: expr
;

for_body:
  expr_block
;

reduce: REDUCE
{ $$ = new ast::node( ast::kwd_reduce, d_scanner.lineNr() ); }
;

reduce_expr:
  reduce '(' id ',' id IN expr ')' reduce_body
  {
    $$ = new ast::list_node( ast::reduce_expression, $1->line, {$3,$5,$7,$9} );
  }
;

reduce_body: expr_block
;

int: INT
{
  $$ = new ast::leaf_node<int>(
    ast::integer_num,
    d_scanner.lineNr(),
    std::stoi( d_scanner.matched() )
  );
}
;

real: REAL
{
  $$ = new ast::leaf_node<double>(
    ast::real_num,
    d_scanner.lineNr(),
    std::stod( d_scanner.matched() )
  );
}
;

id: ID
{
  $$ = new ast::leaf_node<string>( ast::identifier, d_scanner.lineNr(), d_scanner.matched() );
}
;

optional_semicolon: ';' | // empty
;
