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
%left DOTDOT

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

%%

/*
TODO: extend 'for' to express reduction
Perhaps use 'map' and 'reduce' instead of 'for'?
Well, 'for' with multi-one-single-one function is already a kind of reduction.
*/

program: stmt_list
;

stmt_list:
  // empty
  { $$ = ast::statement_list( d_scanner.lineNr() ); }
  |
  stmt
  {
    ast::statement_list l = { $1 };
    l.line = l.front().line;
    $$ = std::move(l);
  }
  |
  stmt_list ';' stmt
  {
    ast::statement_list & l = $$ = std::move($1);
    l.emplace_back($3);
  }
  |
  stmt_list ';'
  { $$ = std::move($1); }
;

stmt:
  id '(' param_list ')' '=' expr_block
  {
    ast::statement & s = $$;
    s.id = std::move($1);
    s.params = std::move($3);
    s.body = std::move($6);
    s.line = s.id.line;
  }
  |
  id '=' expr_block
  {
    ast::statement & s = $$;
    s.id = std::move($1);
    s.body = std::move($3);
    s.line = s.id.line;
  }
;

param_list:
  // empty
  { $$ = ast::id_list( d_scanner.lineNr() ); }
  |
  id
  {
    ast::id_list & l = $$;
    l.emplace_back($1);
  }
  |
  param_list ',' id
  {
    ast::id_list & l = $$ = std::move($1);
    l.emplace_back($3);
  }
;

expr_block:
  complex_expr
  {
    ast::expression_block & b = $$;
    b.expr = $1;
    // FIXME:
    b.line = b.expr->line;
  }
  |
  '{' complex_expr '}'
  {
    ast::expression_block & b = $$;
    b.expr = $2;
    // FIXME:
    b.line = b.expr->line;
  }
  |
  '{' let_stmt_list ';' complex_expr '}'
  {
    ast::expression_block & b = $$;
    b.environment = std::move($2);
    b.expr = $4;
    // FIXME:
    b.line = b.expr->line;
  }
;

let_stmt_list:
  let_stmt
  |
  let_stmt_list ';' let_stmt
  {
    $$ = std::move($1);
    ast::statement_list & add = $3;
    std::move( add.begin(), add.end(), std::back_inserter($$) );
  }
;

let: LET { $$ = ast::node( d_scanner.lineNr() ); };

let_stmt:
  // empty
  {
    $$ = ast::statement_list( d_scanner.lineNr() );
  }
  |
  let stmt
  {
    ast::statement_list & l = $$;
    l.emplace_back( std::move($2) );
    l.line = $<NODE>1.line;
  }
  |
  let '{' stmt_list '}'
  {
    ast::statement_list & l = $$;
    l = std::move($3);
    l.line = $<NODE>1.line;
  }
;

expr:
  expr EQ expr
  { $$ = sp<ast::expression>( new ast::binop_expression( $1, ast::equals, $3 ) ); }
  |
  expr NEQ expr
  { $$ = sp<ast::expression>( new ast::binop_expression( $1, ast::not_equals, $3 ) ); }
  |
  expr LESS expr
  { $$ = sp<ast::expression>( new ast::binop_expression( $1, ast::less_than, $3 ) ); }
  |
  expr MORE expr
  { $$ = sp<ast::expression>( new ast::binop_expression( $1, ast::more_than, $3 ) ); }
  |
  expr '+' expr
  { $$ = sp<ast::expression>( new ast::binop_expression( $1, ast::add, $3 ) ); }
  |
  expr '-' expr
  { $$ = sp<ast::expression>( new ast::binop_expression( $1, ast::subtract, $3 ) ); }
  |
  expr '*' expr
  { $$ = sp<ast::expression>( new ast::binop_expression( $1, ast::multiply, $3 ) ); }
  |
  expr '/' expr
  { $$ = sp<ast::expression>( new ast::binop_expression( $1, ast::divide, $3 ) ); }
  |
  '(' expr ')'
  { $$ = $2; }
  |
  hash // get index of iterator or size of stream
  { $$ = sp<ast::expression>( new ast::hash_expression( std::move($1) ) ); }
  |
  range
  { $$ = sp<ast::expression>( new ast::range_expression($1) ); }
  |
  call
  { $$ = sp<ast::expression>( new ast::call( std::move($1) ) ); }
  |
  number_expr
;

hash:
  '#' id // implied dimension 1
  { $$ = ast::hash_expression($2); }
  |
  '#' '(' id ')' // implied dimension 1
  { $$ = ast::hash_expression($3); }
  |
  '#' '(' id ',' expr ')' // second arg = dimension
  { $$ = ast::hash_expression($3, $5); }
;

simple_expr:
  expr
;

complex_expr:
  simple_expr
  |
  for_expr
  { $$ = sp<ast::expression>( new ast::for_expression( std::move($1) ) ); }
  |
  reduce_expr
  { $$ = sp<ast::expression>( new ast::reduce_expression( std::move($1) ) ); }
;

number_expr:
  int
  { $$ = sp<ast::expression>( new ast::numerical_expression($1, d_scanner.lineNr()) ); }
  |
  real
  { $$ = sp<ast::expression>( new ast::numerical_expression($1, d_scanner.lineNr()) ); }
;

range:
  expr DOTDOT expr
  { $$ = ast::range($1, $3); }
  |
  expr DOTDOT
  { $$ = ast::range($1, nullptr); }
  |
  DOTDOT expr
  { $$ = ast::range(nullptr, $2); }
;

call:
  id call_args call_dim call_range
  {
    ast::call & c = $$;
    c.id = std::move($1);
    c.args = std::move($2);
    c.dimensions = std::move($3);
    c.range = std::move($4);
    c.line = c.id.line;
  }
;

call_args:
  // empty
  { $$ = ast::expression_list( d_scanner.lineNr() ); }
  |
  '(' ')'
  { $$ = ast::expression_list( d_scanner.lineNr() ); }
  |
  '(' complex_expr_list ')'
  { $$ = std::move($2); }
;

call_dim:
  // empty
  { $$ = ast::int_list( d_scanner.lineNr() ); }
  |
  '{' '.' '}'
  { /* FIXME */ $$ = ast::int_list( d_scanner.lineNr() ); }
  |
  '{' int_list '}'
  { $$ = std::move($2); }
;

int_list:
  int
  {
    ast::int_list l { $1 };
    l.line = d_scanner.lineNr();
    $$ = l;
  }
  |
  int_list ',' int
  { $$ = std::move($1); $<INT_LIST>$.push_back($3); }
;

call_range:
  // empty
  { $$ = ast::expression_list( d_scanner.lineNr() ); }
  |
  '[' simple_expr_list ']'
  { $$ = std::move($2); }
;

complex_expr_list:
  complex_expr
  {
     ast::expression_list l { $1 };
     l.line = $<EXPR>1->line;
     $$ = l;
  }
  |
  complex_expr_list ',' complex_expr
  { $$ = std::move($1); $<EXPR_LIST>$.emplace_back($3); }
;

simple_expr_list:
  simple_expr
  {
    ast::expression_list & l = $$;
    l.emplace_back($1);
    l.line = l.front()->line;
  }
  |
  simple_expr_list ',' simple_expr
  {
    ast::expression_list & l = $$ = std::move($1);
    l.emplace_back($3);
  }
;

for: FOR { $$ = ast::node(d_scanner.lineNr()); };

for_expr:
  for EACH '(' for_spec_list ')' for_body
  {
    ast::for_expression &f = $$;
    f.iterations = std::move($4);
    f.body = std::move($6);
    f.line = f.iterations.line;
  }
;

for_spec_list:
  for_spec
  {
    ast::for_iteration_list l = { std::move($1) };
    l.line = l.front().line;
    $$ = std::move(l);
  }
  |
  for_spec_list ';'
  { $$ = std::move($1); }
  |
  for_spec_list ';' for_spec
  {
    ast::for_iteration_list & out = $$;
    out = std::move($1);
    out.emplace_back(std::move($3));
  }
;

for_spec:
  for_iterator for_size for_hop IN for_domain
  {
    ast::for_iteration & i = $$;
    i.iterator = std::move($1);
    i.size = $2;
    i.hop = $3;
    i.domain = $5;
    i.line = i.iterator.line;
  }
;

for_iterator:
  //empty
  { $$ = ast::identifier( d_scanner.lineNr() ); }
  |
  id
  { $$ = std::move($1); }
;

for_size:
  // empty
  { $$ = nullptr; }
  |
  TAKES expr
  { $$ = $2; }
;

for_hop:
  // empty
  { $$ = nullptr; }
  |
  EVERY expr
  { $$ = $2; }
;

for_domain: expr
;

for_body:
  expr_block
;

reduce: REDUCE { $$ = ast::node(d_scanner.lineNr()); };

reduce_expr:
  reduce '(' id ',' id IN expr ')' reduce_body
  {
    ast::reduce_expression & r = $$;
    r.id1 = $3;
    r.id2 = $5;
    r.domain = $7;
    r.body = $9;
    r.line = r.id1.line;
  }
;

reduce_body: expr_block
;

int: INT { $$ = std::stoi( d_scanner.matched() ); }
;

real: REAL { $$ = std::stod( d_scanner.matched() ); }
;

id: ID
{
  ast::identifier & id = $$;
  id.name = d_scanner.matched();
  id.line = d_scanner.lineNr();
}
;
