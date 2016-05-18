%code requires
{
  #include "../common/ast.hpp"
  namespace stream { namespace parsing { class driver; } }
}

%skeleton "lalr1.cc"

%defines

%locations

%define api.namespace {stream::parsing}
%define api.value.type {stream::ast::semantic_value_type}
%parse-param { class stream::parsing::driver& driver }

%define parse.error verbose

%define lr.type ielr

%token END 0  "end of file"
%token INVALID "invalid token"
%token INT REAL ID TRUE FALSE STRING
%token IF THEN CASE THIS
%token LET
%token MODULE IMPORT AS

%left '='
%right RIGHT_ARROW
%right ELSE
%left LOGIC_OR
%left LOGIC_AND
%left EQ NEQ LESS MORE LESS_EQ MORE_EQ
%left PLUSPLUS
%left '+' '-'
%left '*' '/' INT_DIV '%'
%left '^'
%left DOTDOT
%right LOGIC_NOT
%right UMINUS '#'
// FIXME: review precedence and association
%left '[' '{' '('
%right '@'

%start program

%code
{
#include "driver.hpp"
#include "scanner.hpp"

#undef yylex
#define yylex driver.scanner.lex

using namespace stream::ast;
using op_type = stream::primitive_op;
}

%%


program:
  module_decl imports statements
  {
    $$ = make_list(program, @$, { $1, $2, $3 });
    driver.m_ast = $$;
  }
;

module_decl:
  // empty
  { $$ = nullptr; }
  |
  MODULE id ';'
  { $$ = $2; }
;

imports:
  // empty
  { $$ = nullptr; }
  |
  import_list ';'
;

import_list:
  import
  {
    $$ = make_list( @$, { $1 } );
  }
  |
  import_list ';' import
  {
    $$ = $1;
    $$->as_list()->append( $3 );
    $$->location = @$;
  }
;

statements:
  // empty
  { $$ = nullptr; }
  |
  stmt_list optional_semicolon
;

import:
  IMPORT id
  {
  $$ = make_list( @$, { $2, nullptr } );
  }
  |
  IMPORT id AS id
  {
  $$ = make_list( @$, { $2, $4 } );
  }
;

stmt_list:
  stmt
  {
    $$ = make_list( @$, { $1 } );
  }
  |
  stmt_list ';' stmt
  {
    $$ = $1;
    $$->as_list()->append( $3 );
    $$->location = @$;
  }
;

stmt:
  id '(' param_list ')' '=' expr_block
  {
    $$ = make_list( func_def, @$, {$1, $3, $6} );
  }
  |
  id '=' expr_block
  {
    $$ = make_list( func_def, @$, {$1, nullptr, $3} );
  }
;

param_list:
  // empty
  { $$ = make_list( @$, {} ); }
  |
  id
  { $$ = make_list( @$, {$1} ); }
  |
  param_list ',' id
  {
    $$ = $1;
    $$->as_list()->append( $3 );
    $$->location = @$;
  }
;

expr_block:
  expr
  {
    $$ = make_list( @$, {nullptr, $1} );
  }
  |
  '{' expr optional_semicolon '}'
  {
    $$ = make_list( @$, {nullptr, $2} );
  }
  |
  '{' let_block_list ';' expr optional_semicolon '}'
  {
    $$ = make_list( @$, {$2, $4} );
  }
;

let_block_list:
  let_block
  |
  let_block_list ';' let_block
  {
    $$ = $1;
    $$->as_list()->append( $3->as_list()->elements );
    $$->location = @$;
  }
;

let_block:
  LET stmt
  {
    $$ = make_list( @$, {$2} );
  }
  |
  LET '{' stmt_list optional_semicolon '}'
  {
    $$ = $3;
    $$->location = @$;
  }
;

expr:
  id
  |
  id '.' id
  { $$ = make_list( qualified_id, @$, {$1, $3} ); }
  |
  number
  |
  boolean
  |
  if_expr
  |
  func_apply
  |
  array_func
  |
  array_enum
  |
  array_apply
  |
  array_self_apply
  |
  array_size
  |
  expr PLUSPLUS expr
  { $$ = make_list( array_concat, @$, {$1, $3} ); }
  |
  LOGIC_NOT expr
  { $$ = make_list( primitive, @$, {make_const(@1,op_type::negate), $2} ); }
  |
  expr LOGIC_OR expr
  { $$ = make_list( primitive, @$, {make_const(@2,op_type::logic_or), $1, $3} ); }
  |
  expr LOGIC_AND expr
  { $$ = make_list( primitive, @$, {make_const(@2,op_type::logic_and), $1, $3} ); }
  |
  expr EQ expr
  { $$ = make_list( primitive, @$, {make_const(@2,op_type::compare_eq), $1, $3} ); }
  |
  expr NEQ expr
  { $$ = make_list( primitive, @$, {make_const(@2,op_type::compare_neq), $1, $3} ); }
  |
  expr LESS expr
  { $$ = make_list( primitive, @$, {make_const(@2,op_type::compare_l), $1, $3} ); }
  |
  expr LESS_EQ expr
  { $$ = make_list( primitive, @$, {make_const(@2,op_type::compare_leq), $1, $3} ); }
  |
  expr MORE expr
  { $$ = make_list( primitive, @$, {make_const(@2,op_type::compare_g), $1, $3} ); }
  |
  expr MORE_EQ expr
  { $$ = make_list( primitive, @$, {make_const(@2,op_type::compare_geq), $1, $3} ); }
  |
  expr '+' expr
  { $$ = make_list( primitive, @$, {make_const(@2,op_type::add), $1, $3} ); }
  |
  expr '-' expr
  { $$ = make_list( primitive, @$, {make_const(@2,op_type::subtract), $1, $3} ); }
  |
  '-' expr %prec UMINUS
  { $$ = make_list( primitive, @$, {make_const(@1,op_type::negate), $2} ); }
  |
  expr '*' expr
  { $$ = make_list( primitive, @$, {make_const(@2,op_type::multiply), $1, $3} ); }
  |
  expr '/' expr
    { $$ = make_list( primitive, @$, {make_const(@2,op_type::divide), $1, $3} ); }
  |
  expr INT_DIV expr
  { $$ = make_list( primitive, @$, {make_const(@2,op_type::divide_integer), $1, $3} ); }
  |
  expr '%' expr
  { $$ = make_list( primitive, @$, {make_const(@2,op_type::modulo), $1, $3} ); }
  |
  expr '^' expr
  { $$ = make_list( primitive, @$, {make_const(@2,op_type::raise), $1, $3} ); }
  |
  '(' expr ')'
  { $$ = $2; }
;

array_apply:
  expr '[' expr_list ']'
  { $$ = make_list( ast::array_apply, @$, {$1, $3} ); }
;

array_self_apply:
  THIS '[' expr_list ']'
  { $$ = make_list( ast::array_apply, @$, {$1, $3} ); }
;

array_func:
  '[' array_arg_list RIGHT_ARROW array_body ']'
  { $$ = make_list( ast::array_def, @$, {$2, $4} ); }
;

array_enum:
  '[' array_elem_list ']'
  {
    $$ = $2;
    $$->type = ast::array_enum;
    $$->location = @$;
  }
;

array_elem_list:
  expr
  { $$ = make_list( @$, {$1} ); }
  |
  array_elem_list ';' expr
  {
    $$ = $1;
    $$->as_list()->append( $3 );
    $$->location = @$;
  }
;

array_body:
  expr
  |
  case_expr
;

array_arg_list:
  array_arg
  { $$ = make_list( array_params, @$, {$1} ); }
  |
  array_arg_list ',' array_arg
  {
    $$ = $1;
    $$->as_list()->append( $3 );
  }
;

array_arg:
  id
  { $$ = make_list( array_param, @$, {$1, nullptr} ); }
  |
  id ':' expr
  { $$ = make_list( array_param, @$, {$1, $3} ); }
;

array_size:
  '#' expr
  { $$ = make_list( array_size, @$, { $2, nullptr } ); }
  |
  '#' expr '@' expr
  { $$ = make_list( array_size, @$, { $2, $4 } ); }
;

func_apply:
  expr '(' expr_list ')'
  {
    $$ = make_list( ast::func_apply, @$, {$1, $3} );
  }
;

expr_list:
  expr
  { $$ = make_list( @$, {$1} ); }
  |
  expr_list ',' expr
  {
    $$ = $1;
    $$->as_list()->append( $3 );
  }
;

if_expr:
  IF expr THEN expr ELSE expr
  { $$ = make_list( primitive, @$, {make_const(@1,op_type::conditional), $2, $4, $6} ); }
;

case_expr:
  CASE expr_in_domain_list ';' ELSE ':' expr %prec ELSE
  { $$ = make_list( ast::case_expr, @$, { $2, $6 } ); }
;

expr_in_domain_list:
  expr_in_domain
  { $$ = make_list( @$, {$1} ); }
  |
  expr_in_domain_list ';' expr_in_domain
  {
    $$ = $1;
    $$->location = @$;
    $$->as_list()->append( $3 );
  }
;

expr_in_domain:
  expr ':' expr
  { $$ = make_list( @$, { $1, $3 } ); }
;

number:
  int
  |
  real
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
