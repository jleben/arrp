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
%token INT REAL COMPLEX TRUE FALSE STRING
%token ID QUALIFIED_ID
%token IF THEN CASE THIS
%token WHERE
%token MODULE IMPORT AS

%left '='
%right LET IN
%right WHERE
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
%left '.'

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
  module_decl imports bindings
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

bindings:
  // empty
  { $$ = nullptr; }
  |
  binding_list optional_semicolon
;

binding_list:
  binding
  {
    $$ = make_list( @$, { $1 } );
  }
  |
  binding_list ';' binding
  {
    $$ = $1;
    $$->as_list()->append( $3 );
    $$->location = @$;
  }
;

binding:
  id '(' param_list ')' '=' expr
  {
    $$ = make_list( ast::binding, @$, {$1, $3, $6} );
  }
  |
  id '=' expr
  {
    $$ = make_list( ast::binding, @$, {$1, nullptr, $3} );
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

expr:
  id
  |
  qualified_id
  |
  number
  |
  inf
  |
  boolean
  |
  if_expr
  |
  lambda
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
  |
  let_expr
  |
  where_expr
  |
  id '=' expr
  {
    $$ = make_list( ast::binding, @$, {$1, nullptr, $3} );
  }
;


let_expr:
  LET binding IN expr
  {
    auto bnd_list = make_list(@2, {$2});
    $$ = make_list(ast::local_scope, @$, { bnd_list, $4 } );
  }
  |
  LET '{' binding_list optional_semicolon '}' IN expr
  {
    $$ = make_list(ast::local_scope, @$, { $3, $7 } );
  }
;

where_expr:
  expr WHERE binding
  {
    auto bnd_list = make_list(@3, {$3});
    $$ = make_list(ast::local_scope, @$, { bnd_list, $1 } );
  }
  |
  expr WHERE '{' binding_list optional_semicolon '}'
  {
    $$ = make_list(ast::local_scope, @$, { $4, $1 } );
  }
;

lambda:
  '\\'  param_list RIGHT_ARROW expr
  {
    $$ = make_list(ast::lambda, @$, {$2, $4} );
  }
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
  '[' array_ranges ':' array_pattern_list optional_semicolon ']'
  { $$ = make_list( ast::array_def, @$, {$2, $4} ); }
  |
  '[' array_pattern_list optional_semicolon ']'
  { $$ = make_list( ast::array_def, @$, {nullptr, $2} ); }
;

array_ranges:
  expr_list
;


array_pattern_list:
  array_pattern
  { $$ = make_list( @$, {$1} ); }
  |
  array_pattern_list ';' array_pattern
  {
    $$ = $1;
    $$->as_list()->append( $3 );
    $$->location = @$;
  }
;

array_pattern:
  expr_list RIGHT_ARROW expr
  { $$ = make_list( @$, { $1, nullptr, $3 } ); }
  |
  expr_list array_domain_list '|' expr
  { $$ = make_list( @$, { $1, $2, $4 } ); }
;


array_domain_list:
  array_domain
  { $$ = make_list( @$, {$1} ); }
  |
  array_domain_list array_domain
  {
    $$ = $1;
    $$->as_list()->append( $2 );
    $$->location = @$;
  }
;
array_domain:
  '|' expr RIGHT_ARROW expr
  { $$ = make_list( @$, { $2, $4 } ); }
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

number:
  int
  |
  real
  |
  complex
;

int: INT
;

real: REAL
;

complex: COMPLEX
;

boolean:
  TRUE
  |
  FALSE
;

id: ID
;

qualified_id: QUALIFIED_ID
;

inf: '~'
  { $$ = make_node(infinity, @$); }
;

optional_semicolon: ';' | // empty
;

%%

void
stream::parsing::parser::error (const location_type& l, const std::string& m)
{
  driver.error (l, m);
}
