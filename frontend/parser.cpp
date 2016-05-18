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
#line 48 "parser.y" // lalr1.cc:408

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
#line 64 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list(program, yylhs.location, { (yystack_[2].value), (yystack_[1].value), (yystack_[0].value) });
    driver.m_ast = (yylhs.value);
  }
#line 602 "parser.cpp" // lalr1.cc:847
    break;

  case 3:
#line 72 "parser.y" // lalr1.cc:847
    { (yylhs.value) = nullptr; }
#line 608 "parser.cpp" // lalr1.cc:847
    break;

  case 4:
#line 75 "parser.y" // lalr1.cc:847
    { (yylhs.value) = (yystack_[1].value); }
#line 614 "parser.cpp" // lalr1.cc:847
    break;

  case 5:
#line 80 "parser.y" // lalr1.cc:847
    { (yylhs.value) = nullptr; }
#line 620 "parser.cpp" // lalr1.cc:847
    break;

  case 7:
#line 87 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value) } );
  }
#line 628 "parser.cpp" // lalr1.cc:847
    break;

  case 8:
#line 92 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 638 "parser.cpp" // lalr1.cc:847
    break;

  case 9:
#line 101 "parser.y" // lalr1.cc:847
    { (yylhs.value) = nullptr; }
#line 644 "parser.cpp" // lalr1.cc:847
    break;

  case 11:
#line 108 "parser.y" // lalr1.cc:847
    {
  (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value), nullptr } );
  }
#line 652 "parser.cpp" // lalr1.cc:847
    break;

  case 12:
#line 113 "parser.y" // lalr1.cc:847
    {
  (yylhs.value) = make_list( yylhs.location, { (yystack_[2].value), (yystack_[0].value) } );
  }
#line 660 "parser.cpp" // lalr1.cc:847
    break;

  case 13:
#line 120 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value) } );
  }
#line 668 "parser.cpp" // lalr1.cc:847
    break;

  case 14:
#line 125 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 678 "parser.cpp" // lalr1.cc:847
    break;

  case 15:
#line 134 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( func_def, yylhs.location, {(yystack_[5].value), (yystack_[3].value), (yystack_[0].value)} );
  }
#line 686 "parser.cpp" // lalr1.cc:847
    break;

  case 16:
#line 139 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( func_def, yylhs.location, {(yystack_[2].value), nullptr, (yystack_[0].value)} );
  }
#line 694 "parser.cpp" // lalr1.cc:847
    break;

  case 17:
#line 146 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( yylhs.location, {} ); }
#line 700 "parser.cpp" // lalr1.cc:847
    break;

  case 18:
#line 149 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 706 "parser.cpp" // lalr1.cc:847
    break;

  case 19:
#line 152 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 716 "parser.cpp" // lalr1.cc:847
    break;

  case 20:
#line 161 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( yylhs.location, {nullptr, (yystack_[0].value)} );
  }
#line 724 "parser.cpp" // lalr1.cc:847
    break;

  case 21:
#line 166 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( yylhs.location, {nullptr, (yystack_[2].value)} );
  }
#line 732 "parser.cpp" // lalr1.cc:847
    break;

  case 22:
#line 171 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( yylhs.location, {(yystack_[4].value), (yystack_[2].value)} );
  }
#line 740 "parser.cpp" // lalr1.cc:847
    break;

  case 24:
#line 180 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value)->as_list()->elements );
    (yylhs.value)->location = yylhs.location;
  }
#line 750 "parser.cpp" // lalr1.cc:847
    break;

  case 25:
#line 189 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} );
  }
#line 758 "parser.cpp" // lalr1.cc:847
    break;

  case 26:
#line 194 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->location = yylhs.location;
  }
#line 767 "parser.cpp" // lalr1.cc:847
    break;

  case 28:
#line 204 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( qualified_id, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} ); }
#line 773 "parser.cpp" // lalr1.cc:847
    break;

  case 38:
#line 225 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( array_concat, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} ); }
#line 779 "parser.cpp" // lalr1.cc:847
    break;

  case 39:
#line 228 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::negate), (yystack_[0].value)} ); }
#line 785 "parser.cpp" // lalr1.cc:847
    break;

  case 40:
#line 231 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::logic_or), (yystack_[2].value), (yystack_[0].value)} ); }
#line 791 "parser.cpp" // lalr1.cc:847
    break;

  case 41:
#line 234 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::logic_and), (yystack_[2].value), (yystack_[0].value)} ); }
#line 797 "parser.cpp" // lalr1.cc:847
    break;

  case 42:
#line 237 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_eq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 803 "parser.cpp" // lalr1.cc:847
    break;

  case 43:
#line 240 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_neq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 809 "parser.cpp" // lalr1.cc:847
    break;

  case 44:
#line 243 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_l), (yystack_[2].value), (yystack_[0].value)} ); }
#line 815 "parser.cpp" // lalr1.cc:847
    break;

  case 45:
#line 246 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_leq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 821 "parser.cpp" // lalr1.cc:847
    break;

  case 46:
#line 249 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_g), (yystack_[2].value), (yystack_[0].value)} ); }
#line 827 "parser.cpp" // lalr1.cc:847
    break;

  case 47:
#line 252 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_geq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 833 "parser.cpp" // lalr1.cc:847
    break;

  case 48:
#line 255 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::add), (yystack_[2].value), (yystack_[0].value)} ); }
#line 839 "parser.cpp" // lalr1.cc:847
    break;

  case 49:
#line 258 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::subtract), (yystack_[2].value), (yystack_[0].value)} ); }
#line 845 "parser.cpp" // lalr1.cc:847
    break;

  case 50:
#line 261 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::negate), (yystack_[0].value)} ); }
#line 851 "parser.cpp" // lalr1.cc:847
    break;

  case 51:
#line 264 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::multiply), (yystack_[2].value), (yystack_[0].value)} ); }
#line 857 "parser.cpp" // lalr1.cc:847
    break;

  case 52:
#line 267 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::divide), (yystack_[2].value), (yystack_[0].value)} ); }
#line 863 "parser.cpp" // lalr1.cc:847
    break;

  case 53:
#line 270 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::divide_integer), (yystack_[2].value), (yystack_[0].value)} ); }
#line 869 "parser.cpp" // lalr1.cc:847
    break;

  case 54:
#line 273 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::modulo), (yystack_[2].value), (yystack_[0].value)} ); }
#line 875 "parser.cpp" // lalr1.cc:847
    break;

  case 55:
#line 276 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::raise), (yystack_[2].value), (yystack_[0].value)} ); }
#line 881 "parser.cpp" // lalr1.cc:847
    break;

  case 56:
#line 279 "parser.y" // lalr1.cc:847
    { (yylhs.value) = (yystack_[1].value); }
#line 887 "parser.cpp" // lalr1.cc:847
    break;

  case 57:
#line 284 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( ast::array_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} ); }
#line 893 "parser.cpp" // lalr1.cc:847
    break;

  case 58:
#line 289 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( ast::array_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} ); }
#line 899 "parser.cpp" // lalr1.cc:847
    break;

  case 59:
#line 294 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( ast::array_def, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} ); }
#line 905 "parser.cpp" // lalr1.cc:847
    break;

  case 60:
#line 299 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[1].value);
    (yylhs.value)->type = ast::array_enum;
    (yylhs.value)->location = yylhs.location;
  }
#line 915 "parser.cpp" // lalr1.cc:847
    break;

  case 61:
#line 308 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 921 "parser.cpp" // lalr1.cc:847
    break;

  case 62:
#line 311 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 931 "parser.cpp" // lalr1.cc:847
    break;

  case 65:
#line 326 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( array_params, yylhs.location, {(yystack_[0].value)} ); }
#line 937 "parser.cpp" // lalr1.cc:847
    break;

  case 66:
#line 329 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
  }
#line 946 "parser.cpp" // lalr1.cc:847
    break;

  case 67:
#line 337 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( array_param, yylhs.location, {(yystack_[0].value), nullptr} ); }
#line 952 "parser.cpp" // lalr1.cc:847
    break;

  case 68:
#line 340 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( array_param, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} ); }
#line 958 "parser.cpp" // lalr1.cc:847
    break;

  case 69:
#line 345 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( array_size, yylhs.location, { (yystack_[0].value), nullptr } ); }
#line 964 "parser.cpp" // lalr1.cc:847
    break;

  case 70:
#line 348 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( array_size, yylhs.location, { (yystack_[2].value), (yystack_[0].value) } ); }
#line 970 "parser.cpp" // lalr1.cc:847
    break;

  case 71:
#line 353 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( ast::func_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} );
  }
#line 978 "parser.cpp" // lalr1.cc:847
    break;

  case 72:
#line 360 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 984 "parser.cpp" // lalr1.cc:847
    break;

  case 73:
#line 363 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
  }
#line 993 "parser.cpp" // lalr1.cc:847
    break;

  case 74:
#line 371 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[5].location,op_type::conditional), (yystack_[4].value), (yystack_[2].value), (yystack_[0].value)} ); }
#line 999 "parser.cpp" // lalr1.cc:847
    break;

  case 75:
#line 376 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( ast::case_expr, yylhs.location, { (yystack_[4].value), (yystack_[0].value) } ); }
#line 1005 "parser.cpp" // lalr1.cc:847
    break;

  case 76:
#line 381 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1011 "parser.cpp" // lalr1.cc:847
    break;

  case 77:
#line 384 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->location = yylhs.location;
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
  }
#line 1021 "parser.cpp" // lalr1.cc:847
    break;

  case 78:
#line 393 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[2].value), (yystack_[0].value) } ); }
#line 1027 "parser.cpp" // lalr1.cc:847
    break;


#line 1031 "parser.cpp" // lalr1.cc:847
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


  const signed char parser::yypact_ninf_ = -65;

  const signed char parser::yytable_ninf_ = -68;

  const short int
  parser::yypact_[] =
  {
      -5,    14,    46,    65,   -65,    45,   -65,    14,    14,    47,
     -65,   -65,    79,   -65,    56,   -65,    -1,    65,    14,    14,
     -65,    78,    14,   -65,   -65,   -65,   -65,   -65,   -65,   -65,
     180,    61,   180,   180,   180,   180,   122,   180,   -65,   307,
     -65,   -65,   -65,   -65,   -65,   -65,   -65,   -65,   -65,   -65,
     -65,    55,    32,   -65,   203,   180,     2,     2,   -20,   307,
      20,   -14,   -65,   -13,    -2,    60,   -65,   174,   257,   180,
     180,   180,   180,   180,   180,   180,   180,   180,   180,   180,
     180,   180,   180,   180,   180,   180,   180,    14,    89,    14,
     180,   307,    -6,   180,   180,   -65,   137,    14,   180,    14,
     -65,   122,   -65,    63,   -65,   329,   350,    -4,    -4,    -4,
      -4,    -4,    -4,   364,   376,   376,    57,    57,    57,    57,
       2,    21,    48,   -65,    78,   -65,   284,   180,   -65,   -65,
     307,   180,   307,    58,   -65,   -65,    62,   307,    56,   -65,
     174,   -65,   -65,   -65,   -65,   180,   307,   226,    67,   -65,
     -65,    66,    69,   307,   180,   151,   -65,   -65,   307,    64,
     -65,   180,   307
  };

  const unsigned char
  parser::yydefact_[] =
  {
       3,     0,     0,     5,    85,     0,     1,     0,     9,     0,
       7,     4,    11,     2,    87,    13,     0,     6,     0,    86,
      10,     0,    17,     8,    12,    14,    81,    82,    83,    84,
       0,     0,     0,     0,     0,     0,     0,     0,    16,    20,
      35,    36,    33,    34,    37,    32,    31,    29,    79,    80,
      30,    27,     0,    18,     0,     0,    50,    39,    69,    61,
       0,     0,    65,    27,     0,     0,    23,    87,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    72,     0,     0,     0,    60,     0,     0,     0,     0,
      25,     0,    86,     0,    56,    40,    41,    42,    43,    44,
      46,    45,    47,    38,    48,    49,    51,    52,    53,    54,
      55,     0,     0,    28,     0,    19,     0,     0,    58,    70,
      62,     0,    63,     0,    64,    66,    67,    68,    87,    24,
      87,    21,    57,    71,    15,     0,    73,     0,     0,    76,
      59,     0,     0,    74,     0,     0,    26,    22,    78,     0,
      77,     0,    75
  };

  const signed char
  parser::yypgoto_[] =
  {
     -65,   -65,   -65,   -65,   -65,   -65,   105,    24,   -17,   -65,
       1,   -65,    30,   -21,   -65,   -65,   -65,   -65,   -65,   -65,
     -65,    40,   -65,   -65,   -19,   -65,   -65,   -65,   -16,   -65,
     -65,   -65,   -65,     0,   -64
  };

  const short int
  parser::yydefgoto_[] =
  {
      -1,     2,     3,     8,     9,    13,    10,    14,    15,    52,
      38,    65,    66,    91,    40,    41,    42,    43,    60,   133,
      61,    62,    44,    45,    92,    46,   134,   148,   149,    47,
      48,    49,    50,    51,    20
  };

  const short int
  parser::yytable_[] =
  {
      39,     5,    25,   103,     4,    96,   -67,    12,    16,    54,
       1,    56,    57,    58,    59,    67,    68,    21,    24,    16,
       4,    85,    53,    86,    93,    77,    78,    79,    80,    81,
      82,    83,    84,    97,   -67,    63,    87,    85,    98,    86,
      99,   127,    22,    85,   128,    86,     6,   100,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,    16,    94,   121,   122,   127,   126,
      95,   142,   129,   130,   151,   132,   152,   137,    88,    89,
     140,     7,    26,    27,     4,    28,    29,   123,    30,   125,
      11,    31,    17,    84,   143,   127,    18,   136,    85,    16,
      86,    19,    55,    39,    87,   101,   146,   124,   150,    32,
     147,   141,   155,    98,   156,   161,    33,   157,    34,    35,
      36,    37,    23,   138,   153,   144,    26,    27,     4,    28,
      29,   139,    30,   158,   147,    31,    64,   135,     0,   160,
     162,    26,    27,     4,    28,    29,     0,    30,     0,   131,
      31,     0,     0,    32,     0,    26,    27,     4,    28,    29,
      33,    30,    34,    35,    31,    37,     0,     0,    32,     0,
       0,   159,     0,     0,     0,    33,     0,    34,    35,     0,
      37,     0,    32,     0,    26,    27,     4,    28,    29,    33,
      30,    34,    35,    31,    37,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    32,     0,     0,    90,    85,     0,    86,    33,   102,
      34,    35,     0,    37,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
       0,     0,     0,     0,    85,     0,    86,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,     0,     0,     0,     0,    85,     0,    86,
       0,     0,     0,     0,     0,     0,     0,   154,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,     0,     0,     0,     0,    85,     0,
      86,     0,     0,   104,   145,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,     0,     0,     0,     0,    85,     0,    86,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,     0,     0,     0,     0,    85,     0,
      86,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,     0,     0,     0,     0,
      85,     0,    86,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,     0,     0,     0,
       0,    85,     0,    86,    78,    79,    80,    81,    82,    83,
      84,     0,     0,     0,     0,    85,     0,    86,    80,    81,
      82,    83,    84,     0,     0,     0,     0,    85,     0,    86
  };

  const short int
  parser::yycheck_[] =
  {
      21,     1,    19,    67,     6,    19,    19,     7,     8,    30,
      15,    32,    33,    34,    35,    36,    37,    18,    18,    19,
       6,    41,    22,    43,    44,    29,    30,    31,    32,    33,
      34,    35,    36,    47,    47,    35,    49,    41,    51,    43,
      42,    47,    43,    41,    50,    43,     0,    64,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    64,    45,    85,    86,    47,    90,
      50,    50,    93,    94,   138,    96,   140,    98,    46,    47,
     101,    16,     4,     5,     6,     7,     8,    87,    10,    89,
      45,    13,    45,    36,    46,    47,    17,    97,    41,    99,
      43,    45,    41,   124,    49,    45,   127,    18,    50,    31,
     131,    48,    45,    51,    48,    51,    38,    48,    40,    41,
      42,    43,    17,    99,   145,   124,     4,     5,     6,     7,
       8,   101,    10,   154,   155,    13,    14,    97,    -1,   155,
     161,     4,     5,     6,     7,     8,    -1,    10,    -1,    12,
      13,    -1,    -1,    31,    -1,     4,     5,     6,     7,     8,
      38,    10,    40,    41,    13,    43,    -1,    -1,    31,    -1,
      -1,    20,    -1,    -1,    -1,    38,    -1,    40,    41,    -1,
      43,    -1,    31,    -1,     4,     5,     6,     7,     8,    38,
      10,    40,    41,    13,    43,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    31,    -1,    -1,    11,    41,    -1,    43,    38,    45,
      40,    41,    -1,    43,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      -1,    -1,    -1,    -1,    41,    -1,    43,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    -1,    -1,    -1,    -1,    41,    -1,    43,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    -1,    -1,    -1,    -1,    41,    -1,
      43,    -1,    -1,    46,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    -1,    -1,    -1,    -1,    41,    -1,    43,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    -1,    -1,    -1,    -1,    41,    -1,
      43,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    -1,    -1,    -1,    -1,
      41,    -1,    43,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    -1,    -1,    -1,
      -1,    41,    -1,    43,    30,    31,    32,    33,    34,    35,
      36,    -1,    -1,    -1,    -1,    41,    -1,    43,    32,    33,
      34,    35,    36,    -1,    -1,    -1,    -1,    41,    -1,    43
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,    15,    53,    54,     6,    85,     0,    16,    55,    56,
      58,    45,    85,    57,    59,    60,    85,    45,    17,    45,
      86,    18,    43,    58,    85,    60,     4,     5,     7,     8,
      10,    13,    31,    38,    40,    41,    42,    43,    62,    65,
      66,    67,    68,    69,    74,    75,    77,    81,    82,    83,
      84,    85,    61,    85,    65,    41,    65,    65,    65,    65,
      70,    72,    73,    85,    14,    63,    64,    65,    65,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    41,    43,    49,    46,    47,
      11,    65,    76,    44,    45,    50,    19,    47,    51,    42,
      60,    45,    45,    86,    46,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    76,    76,    85,    18,    85,    65,    47,    50,    65,
      65,    12,    65,    71,    78,    73,    85,    65,    59,    64,
      65,    48,    50,    46,    62,    20,    65,    65,    79,    80,
      50,    86,    86,    65,    51,    45,    48,    48,    65,    20,
      80,    51,    65
  };

  const unsigned char
  parser::yyr1_[] =
  {
       0,    52,    53,    54,    54,    55,    55,    56,    56,    57,
      57,    58,    58,    59,    59,    60,    60,    61,    61,    61,
      62,    62,    62,    63,    63,    64,    64,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    66,    67,    68,
      69,    70,    70,    71,    71,    72,    72,    73,    73,    74,
      74,    75,    76,    76,    77,    78,    79,    79,    80,    81,
      81,    82,    83,    84,    84,    85,    86,    86
  };

  const unsigned char
  parser::yyr2_[] =
  {
       0,     2,     3,     0,     3,     0,     2,     1,     3,     0,
       2,     2,     4,     1,     3,     6,     3,     0,     1,     3,
       1,     4,     6,     1,     3,     2,     5,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     3,     3,     3,     3,     3,     3,     4,     4,     5,
       3,     1,     3,     1,     1,     1,     3,     1,     3,     2,
       4,     4,     1,     3,     6,     6,     1,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     0
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const parser::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "\"invalid token\"", "INT",
  "REAL", "ID", "TRUE", "FALSE", "STRING", "IF", "THEN", "CASE", "THIS",
  "LET", "MODULE", "IMPORT", "AS", "'='", "RIGHT_ARROW", "ELSE",
  "LOGIC_OR", "LOGIC_AND", "EQ", "NEQ", "LESS", "MORE", "LESS_EQ",
  "MORE_EQ", "PLUSPLUS", "'+'", "'-'", "'*'", "'/'", "INT_DIV", "'%'",
  "'^'", "DOTDOT", "LOGIC_NOT", "UMINUS", "'#'", "'['", "'{'", "'('",
  "'@'", "';'", "')'", "','", "'}'", "'.'", "']'", "':'", "$accept",
  "program", "module_decl", "imports", "import_list", "statements",
  "import", "stmt_list", "stmt", "param_list", "expr_block",
  "let_block_list", "let_block", "expr", "array_apply", "array_self_apply",
  "array_func", "array_enum", "array_elem_list", "array_body",
  "array_arg_list", "array_arg", "array_size", "func_apply", "expr_list",
  "if_expr", "case_expr", "expr_in_domain_list", "expr_in_domain",
  "number", "int", "real", "boolean", "id", "optional_semicolon", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned short int
  parser::yyrline_[] =
  {
       0,    63,    63,    72,    74,    80,    82,    86,    91,   101,
     103,   107,   112,   119,   124,   133,   138,   146,   148,   151,
     160,   165,   170,   177,   179,   188,   193,   201,   203,   206,
     208,   210,   212,   214,   216,   218,   220,   222,   224,   227,
     230,   233,   236,   239,   242,   245,   248,   251,   254,   257,
     260,   263,   266,   269,   272,   275,   278,   283,   288,   293,
     298,   307,   310,   319,   321,   325,   328,   336,   339,   344,
     347,   352,   359,   362,   370,   375,   380,   383,   392,   397,
     399,   402,   405,   409,   411,   414,   417,   417
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
       2,     2,     2,     2,     2,    40,     2,    35,     2,     2,
      43,    46,    32,    30,    47,    31,    49,    33,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    51,    45,
       2,    18,     2,     2,    44,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    41,     2,    50,    36,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    42,     2,    48,     2,     2,     2,     2,
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
      15,    16,    17,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    34,    37,    38,    39
    };
    const unsigned int user_token_number_max_ = 287;
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
#line 1615 "parser.cpp" // lalr1.cc:1155
#line 420 "parser.y" // lalr1.cc:1156


void
stream::parsing::parser::error (const location_type& l, const std::string& m)
{
  driver.error (l, m);
}
