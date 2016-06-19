// A Bison parser, made by GNU Bison 3.0.2.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2013 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.


// First part of user declarations.

#line 37 "parser.cpp" // lalr1.cc:399

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

#include "parser.hpp"

// User implementation prologue.

#line 51 "parser.cpp" // lalr1.cc:407
// Unqualified %code blocks.
#line 51 "parser.y" // lalr1.cc:408

#include "driver.hpp"
#include "scanner.hpp"

#undef yylex
#define yylex driver.scanner.lex

using namespace stream::ast;
using op_type = stream::primitive_op;

#line 64 "parser.cpp" // lalr1.cc:408


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (/*CONSTCOND*/ false)
# endif


// Suppress unused-variable warnings by "using" E.
#define YYUSE(E) ((void) (E))

// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << std::endl;                  \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yystack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE(Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void>(0)
# define YY_STACK_PRINT()                static_cast<void>(0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyempty = true)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

#line 13 "parser.y" // lalr1.cc:474
namespace stream { namespace parsing {
#line 150 "parser.cpp" // lalr1.cc:474

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr = "";
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              // Fall through.
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }


  /// Build a parser object.
  parser::parser (class stream::parsing::driver& driver_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      driver (driver_yyarg)
  {}

  parser::~parser ()
  {}


  /*---------------.
  | Symbol types.  |
  `---------------*/

  inline
  parser::syntax_error::syntax_error (const location_type& l, const std::string& m)
    : std::runtime_error (m)
    , location (l)
  {}

  // basic_symbol.
  template <typename Base>
  inline
  parser::basic_symbol<Base>::basic_symbol ()
    : value ()
  {}

  template <typename Base>
  inline
  parser::basic_symbol<Base>::basic_symbol (const basic_symbol& other)
    : Base (other)
    , value ()
    , location (other.location)
  {
    value = other.value;
  }


  template <typename Base>
  inline
  parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, const semantic_type& v, const location_type& l)
    : Base (t)
    , value (v)
    , location (l)
  {}


  /// Constructor for valueless symbols.
  template <typename Base>
  inline
  parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, const location_type& l)
    : Base (t)
    , value ()
    , location (l)
  {}

  template <typename Base>
  inline
  parser::basic_symbol<Base>::~basic_symbol ()
  {
  }

  template <typename Base>
  inline
  void
  parser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move(s);
    value = s.value;
    location = s.location;
  }

  // by_type.
  inline
  parser::by_type::by_type ()
     : type (empty)
  {}

  inline
  parser::by_type::by_type (const by_type& other)
    : type (other.type)
  {}

  inline
  parser::by_type::by_type (token_type t)
    : type (yytranslate_ (t))
  {}

  inline
  void
  parser::by_type::move (by_type& that)
  {
    type = that.type;
    that.type = empty;
  }

  inline
  int
  parser::by_type::type_get () const
  {
    return type;
  }


  // by_state.
  inline
  parser::by_state::by_state ()
    : state (empty)
  {}

  inline
  parser::by_state::by_state (const by_state& other)
    : state (other.state)
  {}

  inline
  void
  parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.state = empty;
  }

  inline
  parser::by_state::by_state (state_type s)
    : state (s)
  {}

  inline
  parser::symbol_number_type
  parser::by_state::type_get () const
  {
    return state == empty ? 0 : yystos_[state];
  }

  inline
  parser::stack_symbol_type::stack_symbol_type ()
  {}


  inline
  parser::stack_symbol_type::stack_symbol_type (state_type s, symbol_type& that)
    : super_type (s, that.location)
  {
    value = that.value;
    // that is emptied.
    that.type = empty;
  }

  inline
  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    return *this;
  }


  template <typename Base>
  inline
  void
  parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);

    // User destructor.
    YYUSE (yysym.type_get ());
  }

#if YYDEBUG
  template <typename Base>
  void
  parser::yy_print_ (std::ostream& yyo,
                                     const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    symbol_number_type yytype = yysym.type_get ();
    yyo << (yytype < yyntokens_ ? "token" : "nterm")
        << ' ' << yytname_[yytype] << " ("
        << yysym.location << ": ";
    YYUSE (yytype);
    yyo << ')';
  }
#endif

  inline
  void
  parser::yypush_ (const char* m, state_type s, symbol_type& sym)
  {
    stack_symbol_type t (s, sym);
    yypush_ (m, t);
  }

  inline
  void
  parser::yypush_ (const char* m, stack_symbol_type& s)
  {
    if (m)
      YY_SYMBOL_PRINT (m, s);
    yystack_.push (s);
  }

  inline
  void
  parser::yypop_ (unsigned int n)
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  parser::debug_level_type
  parser::debug_level () const
  {
    return yydebug_;
  }

  void
  parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  inline parser::state_type
  parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  inline bool
  parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  inline bool
  parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  parser::parse ()
  {
    /// Whether yyla contains a lookahead.
    bool yyempty = true;

    // State.
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

    // FIXME: This shoud be completely indented.  It is not yet to
    // avoid gratuitous conflicts when merging into the master branch.
    try
      {
    YYCDEBUG << "Starting parse" << std::endl;


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, yyla);

    // A new symbol was pushed on the stack.
  yynewstate:
    YYCDEBUG << "Entering state " << yystack_[0].state << std::endl;

    // Accept?
    if (yystack_[0].state == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    // Backup.
  yybackup:

    // Try to take a decision without lookahead.
    yyn = yypact_[yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyempty)
      {
        YYCDEBUG << "Reading a token: ";
        try
          {
            yyla.type = yytranslate_ (yylex (&yyla.value, &yyla.location));
          }
        catch (const syntax_error& yyexc)
          {
            error (yyexc);
            goto yyerrlab1;
          }
        yyempty = false;
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.type_get ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.type_get ())
      goto yydefault;

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Discard the token being shifted.
    yyempty = true;

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", yyn, yyla);
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_(yystack_[yylen].state, yyr1_[yyn]);
      /* If YYLEN is nonzero, implement the default value of the
         action: '$$ = $1'.  Otherwise, use the top of the stack.

         Otherwise, the following line sets YYLHS.VALUE to garbage.
         This behavior is undocumented and Bison users should not rely
         upon it.  */
      if (yylen)
        yylhs.value = yystack_[yylen - 1].value;
      else
        yylhs.value = yystack_[0].value;

      // Compute the default @$.
      {
        slice<stack_symbol_type, stack_type> slice (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, slice, yylen);
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
      try
        {
          switch (yyn)
            {
  case 2:
#line 67 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list(program, yylhs.location, { (yystack_[2].value), (yystack_[1].value), (yystack_[0].value) });
    driver.m_ast = (yylhs.value);
  }
#line 602 "parser.cpp" // lalr1.cc:847
    break;

  case 3:
#line 75 "parser.y" // lalr1.cc:847
    { (yylhs.value) = nullptr; }
#line 608 "parser.cpp" // lalr1.cc:847
    break;

  case 4:
#line 78 "parser.y" // lalr1.cc:847
    { (yylhs.value) = (yystack_[1].value); }
#line 614 "parser.cpp" // lalr1.cc:847
    break;

  case 5:
#line 83 "parser.y" // lalr1.cc:847
    { (yylhs.value) = nullptr; }
#line 620 "parser.cpp" // lalr1.cc:847
    break;

  case 7:
#line 90 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value) } );
  }
#line 628 "parser.cpp" // lalr1.cc:847
    break;

  case 8:
#line 95 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 638 "parser.cpp" // lalr1.cc:847
    break;

  case 9:
#line 104 "parser.y" // lalr1.cc:847
    {
  (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value), nullptr } );
  }
#line 646 "parser.cpp" // lalr1.cc:847
    break;

  case 10:
#line 109 "parser.y" // lalr1.cc:847
    {
  (yylhs.value) = make_list( yylhs.location, { (yystack_[2].value), (yystack_[0].value) } );
  }
#line 654 "parser.cpp" // lalr1.cc:847
    break;

  case 11:
#line 116 "parser.y" // lalr1.cc:847
    { (yylhs.value) = nullptr; }
#line 660 "parser.cpp" // lalr1.cc:847
    break;

  case 13:
#line 123 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value) } );
  }
#line 668 "parser.cpp" // lalr1.cc:847
    break;

  case 14:
#line 128 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 678 "parser.cpp" // lalr1.cc:847
    break;

  case 15:
#line 137 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( ast::binding, yylhs.location, {(yystack_[5].value), (yystack_[3].value), (yystack_[0].value)} );
  }
#line 686 "parser.cpp" // lalr1.cc:847
    break;

  case 16:
#line 142 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( ast::binding, yylhs.location, {(yystack_[2].value), nullptr, (yystack_[0].value)} );
  }
#line 694 "parser.cpp" // lalr1.cc:847
    break;

  case 17:
#line 149 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( yylhs.location, {} ); }
#line 700 "parser.cpp" // lalr1.cc:847
    break;

  case 18:
#line 152 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 706 "parser.cpp" // lalr1.cc:847
    break;

  case 19:
#line 155 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 716 "parser.cpp" // lalr1.cc:847
    break;

  case 21:
#line 166 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( qualified_id, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} ); }
#line 722 "parser.cpp" // lalr1.cc:847
    break;

  case 32:
#line 189 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( array_concat, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} ); }
#line 728 "parser.cpp" // lalr1.cc:847
    break;

  case 33:
#line 192 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::negate), (yystack_[0].value)} ); }
#line 734 "parser.cpp" // lalr1.cc:847
    break;

  case 34:
#line 195 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::logic_or), (yystack_[2].value), (yystack_[0].value)} ); }
#line 740 "parser.cpp" // lalr1.cc:847
    break;

  case 35:
#line 198 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::logic_and), (yystack_[2].value), (yystack_[0].value)} ); }
#line 746 "parser.cpp" // lalr1.cc:847
    break;

  case 36:
#line 201 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_eq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 752 "parser.cpp" // lalr1.cc:847
    break;

  case 37:
#line 204 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_neq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 758 "parser.cpp" // lalr1.cc:847
    break;

  case 38:
#line 207 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_l), (yystack_[2].value), (yystack_[0].value)} ); }
#line 764 "parser.cpp" // lalr1.cc:847
    break;

  case 39:
#line 210 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_leq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 770 "parser.cpp" // lalr1.cc:847
    break;

  case 40:
#line 213 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_g), (yystack_[2].value), (yystack_[0].value)} ); }
#line 776 "parser.cpp" // lalr1.cc:847
    break;

  case 41:
#line 216 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_geq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 782 "parser.cpp" // lalr1.cc:847
    break;

  case 42:
#line 219 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::add), (yystack_[2].value), (yystack_[0].value)} ); }
#line 788 "parser.cpp" // lalr1.cc:847
    break;

  case 43:
#line 222 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::subtract), (yystack_[2].value), (yystack_[0].value)} ); }
#line 794 "parser.cpp" // lalr1.cc:847
    break;

  case 44:
#line 225 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::negate), (yystack_[0].value)} ); }
#line 800 "parser.cpp" // lalr1.cc:847
    break;

  case 45:
#line 228 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::multiply), (yystack_[2].value), (yystack_[0].value)} ); }
#line 806 "parser.cpp" // lalr1.cc:847
    break;

  case 46:
#line 231 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::divide), (yystack_[2].value), (yystack_[0].value)} ); }
#line 812 "parser.cpp" // lalr1.cc:847
    break;

  case 47:
#line 234 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::divide_integer), (yystack_[2].value), (yystack_[0].value)} ); }
#line 818 "parser.cpp" // lalr1.cc:847
    break;

  case 48:
#line 237 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::modulo), (yystack_[2].value), (yystack_[0].value)} ); }
#line 824 "parser.cpp" // lalr1.cc:847
    break;

  case 49:
#line 240 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::raise), (yystack_[2].value), (yystack_[0].value)} ); }
#line 830 "parser.cpp" // lalr1.cc:847
    break;

  case 50:
#line 243 "parser.y" // lalr1.cc:847
    { (yylhs.value) = (yystack_[1].value); }
#line 836 "parser.cpp" // lalr1.cc:847
    break;

  case 54:
#line 255 "parser.y" // lalr1.cc:847
    {
    auto bnd_list = make_list(yystack_[2].location, {(yystack_[2].value)});
    (yylhs.value) = make_list(ast::local_binding, yylhs.location, { bnd_list, (yystack_[0].value) } );
  }
#line 845 "parser.cpp" // lalr1.cc:847
    break;

  case 55:
#line 261 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list(ast::local_binding, yylhs.location, { (yystack_[4].value), (yystack_[0].value) } );
  }
#line 853 "parser.cpp" // lalr1.cc:847
    break;

  case 56:
#line 268 "parser.y" // lalr1.cc:847
    {
    auto bnd_list = make_list(yystack_[0].location, {(yystack_[0].value)});
    (yylhs.value) = make_list(ast::local_binding, yylhs.location, { bnd_list, (yystack_[2].value) } );
  }
#line 862 "parser.cpp" // lalr1.cc:847
    break;

  case 57:
#line 274 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list(ast::local_binding, yylhs.location, { (yystack_[2].value), (yystack_[5].value) } );
  }
#line 870 "parser.cpp" // lalr1.cc:847
    break;

  case 58:
#line 281 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list(ast::lambda, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} );
  }
#line 878 "parser.cpp" // lalr1.cc:847
    break;

  case 59:
#line 288 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( ast::array_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} ); }
#line 884 "parser.cpp" // lalr1.cc:847
    break;

  case 60:
#line 293 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( ast::array_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} ); }
#line 890 "parser.cpp" // lalr1.cc:847
    break;

  case 61:
#line 298 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( ast::array_def, yylhs.location, {(yystack_[4].value), (yystack_[2].value)} ); }
#line 896 "parser.cpp" // lalr1.cc:847
    break;

  case 62:
#line 303 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 902 "parser.cpp" // lalr1.cc:847
    break;

  case 63:
#line 306 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 912 "parser.cpp" // lalr1.cc:847
    break;

  case 65:
#line 317 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_node(star, yylhs.location); }
#line 918 "parser.cpp" // lalr1.cc:847
    break;

  case 66:
#line 322 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 924 "parser.cpp" // lalr1.cc:847
    break;

  case 67:
#line 325 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 934 "parser.cpp" // lalr1.cc:847
    break;

  case 68:
#line 334 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[2].value), nullptr, (yystack_[0].value) } ); }
#line 940 "parser.cpp" // lalr1.cc:847
    break;

  case 69:
#line 337 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[3].value), (yystack_[2].value), (yystack_[0].value) } ); }
#line 946 "parser.cpp" // lalr1.cc:847
    break;

  case 70:
#line 342 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 952 "parser.cpp" // lalr1.cc:847
    break;

  case 71:
#line 345 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 962 "parser.cpp" // lalr1.cc:847
    break;

  case 74:
#line 356 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 968 "parser.cpp" // lalr1.cc:847
    break;

  case 75:
#line 359 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[1].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 978 "parser.cpp" // lalr1.cc:847
    break;

  case 76:
#line 367 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[2].value), (yystack_[0].value) } ); }
#line 984 "parser.cpp" // lalr1.cc:847
    break;

  case 77:
#line 372 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[1].value);
    (yylhs.value)->type = ast::array_enum;
    (yylhs.value)->location = yylhs.location;
  }
#line 994 "parser.cpp" // lalr1.cc:847
    break;

  case 78:
#line 381 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1000 "parser.cpp" // lalr1.cc:847
    break;

  case 79:
#line 384 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1010 "parser.cpp" // lalr1.cc:847
    break;

  case 80:
#line 393 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( array_size, yylhs.location, { (yystack_[0].value), nullptr } ); }
#line 1016 "parser.cpp" // lalr1.cc:847
    break;

  case 81:
#line 396 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( array_size, yylhs.location, { (yystack_[2].value), (yystack_[0].value) } ); }
#line 1022 "parser.cpp" // lalr1.cc:847
    break;

  case 82:
#line 401 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( ast::func_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} );
  }
#line 1030 "parser.cpp" // lalr1.cc:847
    break;

  case 83:
#line 408 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1036 "parser.cpp" // lalr1.cc:847
    break;

  case 84:
#line 411 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
  }
#line 1045 "parser.cpp" // lalr1.cc:847
    break;

  case 85:
#line 419 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[5].location,op_type::conditional), (yystack_[4].value), (yystack_[2].value), (yystack_[0].value)} ); }
#line 1051 "parser.cpp" // lalr1.cc:847
    break;


#line 1055 "parser.cpp" // lalr1.cc:847
            default:
              break;
            }
        }
      catch (const syntax_error& yyexc)
        {
          error (yyexc);
          YYERROR;
        }
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;
      YY_STACK_PRINT ();

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, yylhs);
    }
    goto yynewstate;

  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        error (yyla.location, yysyntax_error_ (yystack_[0].state,
                                           yyempty ? yyempty_ : yyla.type_get ()));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.type_get () == yyeof_)
          YYABORT;
        else if (!yyempty)
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyempty = true;
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;
    yyerror_range[1].location = yystack_[yylen - 1].location;
    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    {
      stack_symbol_type error_token;
      for (;;)
        {
          yyn = yypact_[yystack_[0].state];
          if (!yy_pact_value_is_default_ (yyn))
            {
              yyn += yyterror_;
              if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
                {
                  yyn = yytable_[yyn];
                  if (0 < yyn)
                    break;
                }
            }

          // Pop the current state because it cannot handle the error token.
          if (yystack_.size () == 1)
            YYABORT;

          yyerror_range[1].location = yystack_[0].location;
          yy_destroy_ ("Error: popping", yystack_[0]);
          yypop_ ();
          YY_STACK_PRINT ();
        }

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = yyn;
      yypush_ ("Shifting", error_token);
    }
    goto yynewstate;

    // Accept.
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    // Abort.
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (!yyempty)
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack"
                 << std::endl;
        // Do not try to display the values of the reclaimed symbols,
        // as their printer might throw an exception.
        if (!yyempty)
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
  }

  void
  parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what());
  }

  // Generate an error message.
  std::string
  parser::yysyntax_error_ (state_type yystate, symbol_number_type yytoken) const
  {
    std::string yyres;
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    size_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yytoken) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state
         merging (from LALR or IELR) and default reductions corrupt the
         expected token list.  However, the list is correct for
         canonical LR with one exception: it will still contain any
         token that will not be accepted due to an error action in a
         later state.
    */
    if (yytoken != yyempty_)
      {
        yyarg[yycount++] = yytname_[yytoken];
        int yyn = yypact_[yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            // Stay within bounds of both yycheck and yytname.
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
        YYCASE_(0, YY_("syntax error"));
        YYCASE_(1, YY_("syntax error, unexpected %s"));
        YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    // Argument number.
    size_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char parser::yypact_ninf_ = -103;

  const signed char parser::yytable_ninf_ = -79;

  const short int
  parser::yypact_[] =
  {
      20,     6,    54,    30,  -103,     8,  -103,     6,     6,    15,
    -103,  -103,    52,  -103,    24,  -103,     3,    30,     6,     6,
    -103,   217,     6,  -103,  -103,  -103,  -103,  -103,  -103,  -103,
    -103,   217,    35,    -3,   217,   217,   217,   174,   217,     6,
     382,  -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,
    -103,  -103,  -103,  -103,  -103,  -103,    -8,    18,  -103,   307,
     217,     6,    68,    21,    21,   -13,  -103,   247,     7,  -103,
     -34,   279,   -10,     1,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,     6,    71,     6,   217,   382,    -6,    24,
     217,   217,   174,    48,   217,  -103,  -103,   217,     6,  -103,
     427,   448,    -9,    -9,    -9,    -9,    -9,    -9,    47,    60,
      60,   -30,   -30,   -30,   -30,    21,    12,    37,   382,  -103,
     217,  -103,   332,   217,  -103,    51,   382,  -103,   382,  -103,
      56,  -103,   -12,  -103,  -103,  -103,   382,   405,    24,  -103,
    -103,   382,   217,   382,    87,    48,    55,   217,    48,   217,
      57,  -103,    58,   405,   217,  -103,  -103,   382,  -103,   357,
     217,  -103,  -103,   382,   217,   357,   382
  };

  const unsigned char
  parser::yydefact_[] =
  {
       3,     0,     0,     5,    94,     0,     1,     0,    11,     0,
       7,     4,     9,     2,    96,    13,     0,     6,     0,    95,
      12,     0,    17,     8,    10,    14,    89,    90,    91,    92,
      93,     0,     0,     0,     0,     0,     0,     0,     0,    17,
      16,    51,    52,    25,    29,    30,    27,    28,    31,    26,
      24,    22,    86,    87,    88,    23,    20,     0,    18,     0,
       0,     0,     0,    44,    33,    80,    65,    64,     0,    62,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    83,     0,    96,
       0,     0,     0,     0,     0,    77,    50,     0,     0,    56,
      34,    35,    36,    37,    38,    40,    39,    41,    32,    42,
      43,    45,    46,    47,    48,    49,     0,     0,    53,    21,
       0,    19,     0,     0,    60,     0,    54,    81,    64,    63,
      96,    66,     0,    70,    73,    72,    79,    58,    96,    59,
      82,    15,     0,    84,     0,    95,     0,     0,     0,     0,
       0,    74,     0,    85,     0,    67,    61,    68,    71,     0,
       0,    75,    57,    55,     0,    69,    76
  };

  const signed char
  parser::yypgoto_[] =
  {
    -103,  -103,  -103,  -103,  -103,    94,  -103,   -58,   -14,    73,
      40,  -103,  -103,  -103,  -103,  -103,  -103,  -103,    28,  -103,
     -24,  -103,   -25,  -103,   -26,  -103,  -103,  -103,  -103,    10,
    -103,  -103,  -102,  -103,  -103,  -103,    -1,   -97
  };

  const short int
  parser::yydefgoto_[] =
  {
      -1,     2,     3,     8,     9,    10,    13,    14,    15,    57,
      97,    41,    42,    43,    44,    45,    46,    68,    69,   140,
     141,   142,   143,   160,   161,    47,    70,    48,    49,    98,
      50,    51,    52,    53,    54,    55,    56,    20
  };

  const short int
  parser::yytable_[] =
  {
       5,   144,   135,    99,     4,    25,    12,    16,     4,    89,
     157,    92,   107,     4,    90,   104,    91,    24,    16,    62,
     105,    58,    21,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    16,    91,   101,    90,     1,    91,    58,   158,
      93,    95,    61,   156,   159,   133,   108,     7,   134,    22,
     148,   162,    26,   144,     6,     4,   144,    11,   102,   109,
      16,    40,   103,   133,    17,    90,   149,    91,    94,    95,
      18,    59,    16,    19,    63,    64,    65,    67,    71,    60,
      83,    84,    85,    86,    87,    88,    89,   150,   133,   100,
     130,    90,   129,    91,   131,    85,    86,    87,    88,    89,
     126,   127,   145,   154,    90,   155,    91,    16,   164,   166,
     172,    23,    72,   170,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     139,   165,   128,   168,   171,     0,   132,     0,     0,     0,
     136,   137,   138,     0,   146,     0,     0,   147,     0,     0,
       0,     0,     0,     0,   145,     0,     0,   145,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     151,     0,     0,   153,     0,     0,     0,     0,    26,    27,
      28,     4,    29,    30,     0,    31,     0,     0,    32,     0,
       0,     0,   163,     0,    33,     0,     0,   167,     0,   169,
       0,     0,     0,     0,   173,     0,     0,     0,    34,    66,
     175,     0,     0,     0,   176,    35,     0,    36,    37,     0,
      38,    26,    27,    28,     4,    29,    30,    39,    31,     0,
       0,    32,     0,     0,     0,     0,     0,    33,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    34,     0,     0,     0,     0,     0,     0,    35,     0,
      36,    37,    73,    38,     0,     0,     0,     0,     0,     0,
      39,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,     0,     0,     0,
       0,    90,     0,    91,    73,     0,   -78,     0,     0,     0,
       0,   -78,     0,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    96,
       0,     0,    73,    90,     0,    91,     0,     0,     0,   106,
       0,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    73,     0,     0,
       0,    90,     0,    91,     0,   152,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    73,     0,     0,     0,    90,     0,    91,   174,
       0,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    73,     0,     0,
       0,    90,     0,    91,     0,     0,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,     0,     0,     0,     0,    90,     0,    91,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,     0,     0,     0,     0,    90,
       0,    91,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,     0,     0,     0,
       0,    90,     0,    91,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,     0,     0,
       0,     0,    90,     0,    91
  };

  const short int
  parser::yycheck_[] =
  {
       1,   103,    99,    61,     7,    19,     7,     8,     7,    39,
      22,    19,    22,     7,    44,    49,    46,    18,    19,    33,
      54,    22,    19,    32,    33,    34,    35,    36,    37,    38,
      39,    44,    33,    46,    47,    44,    16,    46,    39,    51,
      48,    51,    45,   140,    56,    51,    45,    17,    54,    46,
     108,   148,     4,   155,     0,     7,   158,    49,    51,    73,
      61,    21,    55,    51,    49,    44,    54,    46,    50,    51,
      18,    31,    73,    49,    34,    35,    36,    37,    38,    44,
      33,    34,    35,    36,    37,    38,    39,    50,    51,    21,
      19,    44,    93,    46,    95,    35,    36,    37,    38,    39,
      90,    91,   103,    52,    44,    49,    46,   108,    21,    54,
      52,    17,    39,    56,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
     102,   155,    92,   158,   160,    -1,    96,    -1,    -1,    -1,
     100,   101,   102,    -1,   104,    -1,    -1,   107,    -1,    -1,
      -1,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     130,    -1,    -1,   133,    -1,    -1,    -1,    -1,     4,     5,
       6,     7,     8,     9,    -1,    11,    -1,    -1,    14,    -1,
      -1,    -1,   152,    -1,    20,    -1,    -1,   157,    -1,   159,
      -1,    -1,    -1,    -1,   164,    -1,    -1,    -1,    34,    35,
     170,    -1,    -1,    -1,   174,    41,    -1,    43,    44,    -1,
      46,     4,     5,     6,     7,     8,     9,    53,    11,    -1,
      -1,    14,    -1,    -1,    -1,    -1,    -1,    20,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,
      43,    44,    15,    46,    -1,    -1,    -1,    -1,    -1,    -1,
      53,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    -1,
      -1,    44,    -1,    46,    15,    -1,    49,    -1,    -1,    -1,
      -1,    54,    -1,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    12,
      -1,    -1,    15,    44,    -1,    46,    -1,    -1,    -1,    50,
      -1,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    15,    -1,    -1,
      -1,    44,    -1,    46,    -1,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    15,    -1,    -1,    -1,    44,    -1,    46,    22,
      -1,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    15,    -1,    -1,
      -1,    44,    -1,    46,    -1,    -1,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    -1,    -1,    -1,    -1,    44,    -1,    46,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    -1,    -1,    -1,    -1,    44,
      -1,    46,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    -1,
      -1,    44,    -1,    46,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    -1,    -1,
      -1,    -1,    44,    -1,    46
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,    16,    58,    59,     7,    93,     0,    17,    60,    61,
      62,    49,    93,    63,    64,    65,    93,    49,    18,    49,
      94,    19,    46,    62,    93,    65,     4,     5,     6,     8,
       9,    11,    14,    20,    34,    41,    43,    44,    46,    53,
      67,    68,    69,    70,    71,    72,    73,    82,    84,    85,
      87,    88,    89,    90,    91,    92,    93,    66,    93,    67,
      44,    45,    65,    67,    67,    67,    35,    67,    74,    75,
      83,    67,    66,    15,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      44,    46,    19,    48,    50,    51,    12,    67,    86,    64,
      21,    47,    51,    55,    49,    54,    50,    22,    45,    65,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    86,    86,    67,    93,
      19,    93,    67,    51,    54,    94,    67,    67,    67,    75,
      76,    77,    78,    79,    89,    93,    67,    67,    64,    54,
      50,    67,    23,    67,    52,    49,    94,    22,    51,    56,
      80,    81,    94,    67,    21,    77,    54,    67,    79,    67,
      56,    81,    52,    67,    22,    67,    67
  };

  const unsigned char
  parser::yyr1_[] =
  {
       0,    57,    58,    59,    59,    60,    60,    61,    61,    62,
      62,    63,    63,    64,    64,    65,    65,    66,    66,    66,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    68,    68,    69,    69,    70,    71,
      72,    73,    74,    74,    75,    75,    76,    76,    77,    77,
      78,    78,    79,    79,    80,    80,    81,    82,    83,    83,
      84,    84,    85,    86,    86,    87,    88,    88,    88,    89,
      90,    91,    92,    92,    93,    94,    94
  };

  const unsigned char
  parser::yyr2_[] =
  {
       0,     2,     3,     0,     3,     0,     2,     1,     3,     2,
       4,     0,     2,     1,     3,     6,     3,     0,     1,     3,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     3,     3,     3,     3,     3,
       3,     1,     1,     3,     4,     7,     3,     6,     4,     4,
       4,     6,     1,     3,     1,     1,     1,     3,     3,     4,
       1,     3,     1,     1,     1,     2,     4,     3,     1,     3,
       2,     4,     4,     1,     3,     6,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     0
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const parser::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "\"invalid token\"", "INT",
  "REAL", "COMPLEX", "ID", "TRUE", "FALSE", "STRING", "IF", "THEN", "CASE",
  "THIS", "WHERE", "MODULE", "IMPORT", "AS", "'='", "LET", "IN",
  "RIGHT_ARROW", "ELSE", "LOGIC_OR", "LOGIC_AND", "EQ", "NEQ", "LESS",
  "MORE", "LESS_EQ", "MORE_EQ", "PLUSPLUS", "'+'", "'-'", "'*'", "'/'",
  "INT_DIV", "'%'", "'^'", "DOTDOT", "LOGIC_NOT", "UMINUS", "'#'", "'['",
  "'{'", "'('", "'@'", "'.'", "';'", "')'", "','", "'}'", "'\\\\'", "']'",
  "':'", "'|'", "$accept", "program", "module_decl", "imports",
  "import_list", "import", "bindings", "binding_list", "binding",
  "param_list", "expr", "let_expr", "where_expr", "lambda", "array_apply",
  "array_self_apply", "array_func", "array_range_list", "array_range",
  "array_pattern_list", "array_pattern", "array_pattern_vars",
  "array_pattern_var", "array_domain_list", "array_domain", "array_enum",
  "array_elem_list", "array_size", "func_apply", "expr_list", "if_expr",
  "number", "int", "real", "complex", "boolean", "id",
  "optional_semicolon", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned short int
  parser::yyrline_[] =
  {
       0,    66,    66,    75,    77,    83,    85,    89,    94,   103,
     108,   116,   118,   122,   127,   136,   141,   149,   151,   154,
     163,   165,   168,   170,   172,   174,   176,   178,   180,   182,
     184,   186,   188,   191,   194,   197,   200,   203,   206,   209,
     212,   215,   218,   221,   224,   227,   230,   233,   236,   239,
     242,   245,   247,   249,   254,   260,   267,   273,   280,   287,
     292,   297,   302,   305,   314,   316,   321,   324,   333,   336,
     341,   344,   352,   352,   355,   358,   366,   371,   380,   383,
     392,   395,   400,   407,   410,   418,   423,   425,   427,   430,
     433,   436,   440,   442,   445,   448,   448
  };

  // Print the state stack on the debug stream.
  void
  parser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << i->state;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  parser::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):" << std::endl;
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG

  // Symbol number corresponding to token number t.
  inline
  parser::token_number_type
  parser::yytranslate_ (int t)
  {
    static
    const token_number_type
    translate_table[] =
    {
     0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,    43,     2,    38,     2,     2,
      46,    50,    35,    33,    51,    34,    48,    36,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    55,    49,
       2,    19,     2,     2,    47,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    44,    53,    54,    39,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    45,    56,    52,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    37,    40,    41,
      42
    };
    const unsigned int user_token_number_max_ = 290;
    const token_number_type undef_token_ = 2;

    if (static_cast<int>(t) <= yyeof_)
      return yyeof_;
    else if (static_cast<unsigned int> (t) <= user_token_number_max_)
      return translate_table[t];
    else
      return undef_token_;
  }

#line 13 "parser.y" // lalr1.cc:1155
} } // stream::parsing
#line 1664 "parser.cpp" // lalr1.cc:1155
#line 451 "parser.y" // lalr1.cc:1156


void
stream::parsing::parser::error (const location_type& l, const std::string& m)
{
  driver.error (l, m);
}
