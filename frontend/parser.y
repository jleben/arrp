%code requires
{
  #include "../common/ast.hpp"
  namespace stream { namespace parsing { class driver; } }
}

%skeleton "lalr1.cc"

%defines

%locations

%define api.namespace {stream::parsing}
%define api.value.type {stream::ast::semantic_value}
%parse-param { class stream::parsing::driver& driver }

%define parse.error verbose

%define lr.type ielr

%token END 0  "end of file"
%token INVALID "invalid token"
%token INT REAL ID TRUE FALSE
%token IF THEN
%token LET

%left '='
%right RIGHT_ARROW
%right ELSE
%left LOGIC_OR
%left LOGIC_AND
%left EQ NEQ LESS MORE LESS_EQ MORE_EQ
%left '+' '-'
%left '*' '/' ':' '%'
%left '^'
%left DOTDOT
%right LOGIC_NOT
%right UMINUS '#'
// FIXME: review precedence and association
%left '[' '{' '('

%start program

%code
{
#include "driver.hpp"
#include "scanner.hpp"

#undef yylex
#define yylex driver.scanner.lex
}

%%


program:
  stmt_list optional_semicolon
  {
    $$ = $1;
    $$->type = ast::program;
    $$->location = @$;
    driver.m_ast = $$;
  }
  |
  // empty
  {
    $$ = new ast::list_node(ast::program, location_type());
    driver.m_ast = $$;
  }
;

stmt_list:
  stmt
  {
    $$ = new ast::list_node( ast::statement_list, @$, { $1 } );
  }
  |
  stmt_list ';' stmt
  {
    $$ = $1;
    $$.as<ast::list_node>()->append( $3 );
    $$->location = @$;
  }
;

stmt:
  id '(' param_list ')' '=' expr_block
  {
    $$ = new ast::list_node( ast::statement, @$, {$1, $3, $6} );
  }
  |
  id '=' expr_block
  {
    $$ = new ast::list_node( ast::statement, @$, {$1, nullptr, $3} );
  }
;

param_list:
  // empty
  { $$ = new ast::list_node( ast::id_list, @$ ); }
  |
  id
  { $$ = new ast::list_node( ast::id_list, @$, {$1} ); }
  |
  param_list ',' id
  {
    $$ = $1;
    $$.as<ast::list_node>()->append( $3 );
    $$->location = @$;
  }
;

expr_block:
  expr
  {
    $$ = new ast::list_node( ast::expression_block, @$, {nullptr, $1} );
  }
  |
  '{' expr optional_semicolon '}'
  {
    $$ = new ast::list_node( ast::expression_block, @$, {nullptr, $2} );
  }
  |
  '{' let_block_list ';' expr optional_semicolon '}'
  {
    $$ = new ast::list_node( ast::expression_block, @$, {$2, $4} );
  }
;

let_block_list:
  let_block
  |
  let_block_list ';' let_block
  {
    $$ = $1;
    $$.as<ast::list_node>()->append( $3.as<ast::list_node>()->elements );
    $$->location = @$;
  }
;

let_block:
  let stmt
  {
    $$ = new ast::list_node( ast::statement_list, @$, {$2} );
  }
  |
  let '{' stmt_list optional_semicolon '}'
  {
    $$ = $3;
    $$->location = @$;
  }
;

let: LET { $$ = new ast::node( ast::kwd_let, @$ ); }
;

expr:
  array_func
  |
  array_apply
  |
  LOGIC_NOT expr
  { $$ = new ast::list_node( ast::oppose, @$, {$2} ); }
  |
  expr LOGIC_OR expr
  { $$ = new ast::binary_op_expression( $1, ast::logic_or, $3, @$ ); }
  |
  expr LOGIC_AND expr
  { $$ = new ast::binary_op_expression( $1, ast::logic_and, $3, @$ ); }
  |
  expr EQ expr
  { $$ = new ast::binary_op_expression( $1, ast::equal, $3, @$ ); }
  |
  expr NEQ expr
  { $$ = new ast::binary_op_expression( $1, ast::not_equal, $3, @$ ); }
  |
  expr LESS expr
  { $$ = new ast::binary_op_expression( $1, ast::lesser, $3, @$ ); }
  |
  expr LESS_EQ expr
  { $$ = new ast::binary_op_expression( $1, ast::lesser_or_equal, $3, @$ ); }
  |
  expr MORE expr
  { $$ = new ast::binary_op_expression( $1, ast::greater, $3, @$ ); }
  |
  expr MORE_EQ expr
  { $$ = new ast::binary_op_expression( $1, ast::greater_or_equal, $3, @$ ); }
  |
  expr '+' expr
  { $$ = new ast::binary_op_expression( $1, ast::add, $3, @$ ); }
  |
  expr '-' expr
  { $$ = new ast::binary_op_expression( $1, ast::subtract, $3, @$ ); }
  |
  '-' expr %prec UMINUS
  { $$ = new ast::list_node( ast::negate, @$, {$2} ); }
  |
  expr '*' expr
  { $$ = new ast::binary_op_expression( $1, ast::multiply, $3, @$ ); }
  |
  expr '/' expr
  { $$ = new ast::binary_op_expression( $1, ast::divide, $3, @$ ); }
  |
  expr ':' expr
  { $$ = new ast::binary_op_expression( $1, ast::divide_integer, $3, @$ ); }
  |
  expr '%' expr
  { $$ = new ast::binary_op_expression( $1, ast::modulo, $3, @$ ); }
  |
  expr '^' expr
  { $$ = new ast::binary_op_expression( $1, ast::raise, $3, @$ ); }
  |
  '(' expr ')'
  { $$ = $2; $$->location = @$; }
  |
  call
  |
  hash
  |
  if_expr
  |
  id
  |
  number
  |
  boolean
;

hash:
  '#' expr // implied dimension 1
  { $$ = new ast::list_node( ast::hash_expression, @$, {$2, nullptr} ); }
;

array_apply:
  expr '[' expr_list ']'
  { $$ = new ast::list_node( ast::array_application, @$, {$1, $3} ); }
;

array_func:
  '\\' array_arg_list RIGHT_ARROW expr
  { $$ = new ast::list_node( ast::array_function, @$, {$2, $4} ); }
;

array_arg_list:
  array_arg
  { $$ = new ast::list_node( ast::anonymous, @$, {$1} ); }
  |
  array_arg_list ',' array_arg
  {
    $$ = $1;
    $$.as<ast::list_node>()->append( $3 );
  }
;

array_arg:
  id
  { $$ = new ast::list_node( ast::anonymous, @$, {$1, nullptr} ); }
  |
  id '=' expr
  { $$ = new ast::list_node( ast::anonymous, @$, {$1, $3} ); }
;

number:
  int
  |
  real
;


call:
  expr '(' expr_list ')'
  {
    $$ = new ast::list_node( ast::call_expression, @$, {$1, $3} );
  }
;

expr_list:
  expr
  { $$ = new ast::list_node( ast::expression_list, @$, {$1} ); }
  |
  expr_list ',' expr
  {
    $$ = $1;
    $$.as<ast::list_node>()->append( $3 );
  }
;

if_expr:
  IF expr THEN expr ELSE expr
  {
    $$ = new ast::list_node( ast::if_expression, @$, {$2, $4,$6} );
  }
;


int: INT
;

real: REAL
;

boolean:
  TRUE
  |
  FALSE
;

id: ID
;

optional_semicolon: ';' | // empty
;

%%

void
stream::parsing::parser::error (const location_type& l, const std::string& m)
{
  driver.error (l, m);
}
