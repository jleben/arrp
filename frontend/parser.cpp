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
#line 45 "parser.y" // lalr1.cc:408

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
#line 61 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[1].value);
    (yylhs.value)->type = program;
    (yylhs.value)->location = yylhs.location;
    driver.m_ast = (yylhs.value);
  }
#line 604 "parser.cpp" // lalr1.cc:847
    break;

  case 3:
#line 69 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list(program, location_type(), {});
    driver.m_ast = (yylhs.value);
  }
#line 613 "parser.cpp" // lalr1.cc:847
    break;

  case 4:
#line 77 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value) } );
  }
#line 621 "parser.cpp" // lalr1.cc:847
    break;

  case 5:
#line 82 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 631 "parser.cpp" // lalr1.cc:847
    break;

  case 6:
#line 91 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( func_def, yylhs.location, {(yystack_[5].value), (yystack_[3].value), (yystack_[0].value)} );
  }
#line 639 "parser.cpp" // lalr1.cc:847
    break;

  case 7:
#line 96 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( func_def, yylhs.location, {(yystack_[2].value), nullptr, (yystack_[0].value)} );
  }
#line 647 "parser.cpp" // lalr1.cc:847
    break;

  case 8:
#line 103 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( yylhs.location, {} ); }
#line 653 "parser.cpp" // lalr1.cc:847
    break;

  case 9:
#line 106 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 659 "parser.cpp" // lalr1.cc:847
    break;

  case 10:
#line 109 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 669 "parser.cpp" // lalr1.cc:847
    break;

  case 11:
#line 118 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( yylhs.location, {nullptr, (yystack_[0].value)} );
  }
#line 677 "parser.cpp" // lalr1.cc:847
    break;

  case 12:
#line 123 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( yylhs.location, {nullptr, (yystack_[2].value)} );
  }
#line 685 "parser.cpp" // lalr1.cc:847
    break;

  case 13:
#line 128 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( yylhs.location, {(yystack_[4].value), (yystack_[2].value)} );
  }
#line 693 "parser.cpp" // lalr1.cc:847
    break;

  case 15:
#line 137 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value)->as_list()->elements );
    (yylhs.value)->location = yylhs.location;
  }
#line 703 "parser.cpp" // lalr1.cc:847
    break;

  case 16:
#line 146 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} );
  }
#line 711 "parser.cpp" // lalr1.cc:847
    break;

  case 17:
#line 151 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->location = yylhs.location;
  }
#line 720 "parser.cpp" // lalr1.cc:847
    break;

  case 25:
#line 173 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::negate), (yystack_[0].value)} ); }
#line 726 "parser.cpp" // lalr1.cc:847
    break;

  case 26:
#line 176 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::logic_or), (yystack_[2].value), (yystack_[0].value)} ); }
#line 732 "parser.cpp" // lalr1.cc:847
    break;

  case 27:
#line 179 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::logic_and), (yystack_[2].value), (yystack_[0].value)} ); }
#line 738 "parser.cpp" // lalr1.cc:847
    break;

  case 28:
#line 182 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_eq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 744 "parser.cpp" // lalr1.cc:847
    break;

  case 29:
#line 185 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_neq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 750 "parser.cpp" // lalr1.cc:847
    break;

  case 30:
#line 188 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_l), (yystack_[2].value), (yystack_[0].value)} ); }
#line 756 "parser.cpp" // lalr1.cc:847
    break;

  case 31:
#line 191 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_leq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 762 "parser.cpp" // lalr1.cc:847
    break;

  case 32:
#line 194 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_g), (yystack_[2].value), (yystack_[0].value)} ); }
#line 768 "parser.cpp" // lalr1.cc:847
    break;

  case 33:
#line 197 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_geq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 774 "parser.cpp" // lalr1.cc:847
    break;

  case 34:
#line 200 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::add), (yystack_[2].value), (yystack_[0].value)} ); }
#line 780 "parser.cpp" // lalr1.cc:847
    break;

  case 35:
#line 203 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::subtract), (yystack_[2].value), (yystack_[0].value)} ); }
#line 786 "parser.cpp" // lalr1.cc:847
    break;

  case 36:
#line 206 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::negate), (yystack_[0].value)} ); }
#line 792 "parser.cpp" // lalr1.cc:847
    break;

  case 37:
#line 209 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::multiply), (yystack_[2].value), (yystack_[0].value)} ); }
#line 798 "parser.cpp" // lalr1.cc:847
    break;

  case 38:
#line 212 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::divide), (yystack_[2].value), (yystack_[0].value)} ); }
#line 804 "parser.cpp" // lalr1.cc:847
    break;

  case 39:
#line 215 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::divide_integer), (yystack_[2].value), (yystack_[0].value)} ); }
#line 810 "parser.cpp" // lalr1.cc:847
    break;

  case 40:
#line 218 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::modulo), (yystack_[2].value), (yystack_[0].value)} ); }
#line 816 "parser.cpp" // lalr1.cc:847
    break;

  case 41:
#line 221 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::raise), (yystack_[2].value), (yystack_[0].value)} ); }
#line 822 "parser.cpp" // lalr1.cc:847
    break;

  case 42:
#line 224 "parser.y" // lalr1.cc:847
    { (yylhs.value) = (yystack_[1].value); }
#line 828 "parser.cpp" // lalr1.cc:847
    break;

  case 43:
#line 229 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( ast::array_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} ); }
#line 834 "parser.cpp" // lalr1.cc:847
    break;

  case 44:
#line 234 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( ast::array_def, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} ); }
#line 840 "parser.cpp" // lalr1.cc:847
    break;

  case 45:
#line 239 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( array_params, yylhs.location, {(yystack_[0].value)} ); }
#line 846 "parser.cpp" // lalr1.cc:847
    break;

  case 46:
#line 242 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
  }
#line 855 "parser.cpp" // lalr1.cc:847
    break;

  case 47:
#line 250 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( array_param, yylhs.location, {(yystack_[0].value), nullptr} ); }
#line 861 "parser.cpp" // lalr1.cc:847
    break;

  case 48:
#line 253 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( array_param, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} ); }
#line 867 "parser.cpp" // lalr1.cc:847
    break;

  case 49:
#line 258 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = make_list( ast::func_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} );
  }
#line 875 "parser.cpp" // lalr1.cc:847
    break;

  case 50:
#line 265 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 881 "parser.cpp" // lalr1.cc:847
    break;

  case 51:
#line 268 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
  }
#line 890 "parser.cpp" // lalr1.cc:847
    break;

  case 52:
#line 276 "parser.y" // lalr1.cc:847
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[5].location,op_type::conditional), (yystack_[4].value), (yystack_[2].value), (yystack_[0].value)} ); }
#line 896 "parser.cpp" // lalr1.cc:847
    break;


#line 900 "parser.cpp" // lalr1.cc:847
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


  const signed char parser::yypact_ninf_ = -37;

  const signed char parser::yytable_ninf_ = -1;

  const short int
  parser::yypact_[] =
  {
       1,   -37,    23,    18,   -37,    17,   -37,     1,   -37,    65,
       1,   -37,   -37,   -37,   -37,   -37,   117,   117,   117,    73,
     117,     1,   -37,   237,   -37,   -37,   -37,   -37,   -37,   -37,
     -37,   -37,   -37,   -11,   -37,   145,   -28,   -28,    15,    22,
     -37,   191,   167,    12,   -37,    49,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,    53,     1,   117,     1,   -37,    73,   -37,
      26,   -37,   117,     1,   117,   111,   257,   -14,   -14,   -14,
     -14,   -14,   -14,   269,   269,   -10,   -10,   -10,   -10,   -28,
     237,   -23,    25,    65,   -37,   215,    18,   -37,   191,   -37,
     237,   -37,   237,   117,   -37,   -37,   -37,   117,    27,    35,
     237,   237,   -37,   -37
  };

  const unsigned char
  parser::yydefact_[] =
  {
       3,    59,     0,    61,     4,     0,     1,    60,     2,     0,
       8,     5,    55,    56,    57,    58,     0,     0,     0,     0,
       0,     0,     7,    11,    24,    23,    22,    21,    19,    53,
      54,    20,    18,     0,     9,     0,    36,    25,     0,     0,
      14,    61,     0,     0,    45,    47,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    16,     0,    60,
       0,    42,     0,     0,     0,    26,    27,    28,    29,    30,
      32,    31,    33,    34,    35,    37,    38,    39,    40,    41,
      50,     0,     0,     0,    10,     0,    61,    15,    61,    12,
      44,    46,    48,     0,    43,    49,     6,     0,     0,     0,
      51,    52,    17,    13
  };

  const signed char
  parser::yypgoto_[] =
  {
     -37,   -37,    20,    10,   -37,   -17,   -37,    24,   -16,   -37,
     -37,   -37,    21,   -37,    28,   -37,   -37,   -37,   -37,   -37,
      47,   -36
  };

  const signed char
  parser::yydefgoto_[] =
  {
      -1,     2,     3,     4,    33,    22,    39,    40,    23,    24,
      25,    43,    44,    26,    91,    27,    28,    29,    30,    31,
      32,     8
  };

  const unsigned char
  parser::yytable_[] =
  {
      35,    36,    37,    41,    42,    70,    61,     1,    62,    54,
      55,    56,    57,    58,    59,    60,   103,    11,   104,    60,
      61,     1,    62,     6,    61,    72,    62,    63,    64,     9,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    90,     5,    67,    95,
      66,    73,    98,    10,     5,     7,   100,    34,   102,    68,
     108,    74,   109,   105,   103,    93,    99,   112,    45,    12,
      13,     1,    14,    15,    16,   113,   106,    12,    13,     1,
      14,    15,    16,     0,    38,     5,    96,   110,     0,    17,
      92,   111,    97,     0,   101,     0,    18,    17,     0,     0,
      19,    20,     0,     0,    18,     0,     0,    21,     0,    20,
       0,    94,     0,     5,     0,    21,     0,     0,     0,     0,
      45,    12,    13,     1,    14,    15,    16,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    17,     0,     0,     0,    61,     0,    62,    18,     0,
       0,     0,     0,    20,     0,    65,     0,     0,     0,    21,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,     0,     0,     0,     0,    61,
       0,    62,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,     0,     0,     0,
       0,    61,     0,    62,     0,    71,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,     0,     0,     0,     0,    61,     0,    62,    69,   107,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,     0,     0,     0,     0,    61,
       0,    62,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,     0,     0,     0,
       0,    61,     0,    62,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,     0,     0,     0,
       0,    61,     0,    62,    56,    57,    58,    59,    60,     0,
       0,     0,     0,    61,     0,    62
  };

  const signed char
  parser::yycheck_[] =
  {
      16,    17,    18,    19,    20,    41,    34,     6,    36,    23,
      24,    25,    26,    27,    28,    29,    39,     7,    41,    29,
      34,     6,    36,     0,    34,    13,    36,    38,    39,    12,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,     0,    38,    65,
      35,    39,    68,    36,     7,    37,    72,    10,    74,    37,
      96,    12,    98,    38,    39,    12,    40,    40,    21,     4,
       5,     6,     7,     8,     9,    40,    93,     4,     5,     6,
       7,     8,     9,    -1,    11,    38,    66,   103,    -1,    24,
      62,   107,    68,    -1,    73,    -1,    31,    24,    -1,    -1,
      35,    36,    -1,    -1,    31,    -1,    -1,    42,    -1,    36,
      -1,    64,    -1,    66,    -1,    42,    -1,    -1,    -1,    -1,
      73,     4,     5,     6,     7,     8,     9,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    24,    -1,    -1,    -1,    34,    -1,    36,    31,    -1,
      -1,    -1,    -1,    36,    -1,    10,    -1,    -1,    -1,    42,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    -1,    -1,    -1,    -1,    34,
      -1,    36,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    -1,    -1,    -1,
      -1,    34,    -1,    36,    -1,    38,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    -1,    -1,    -1,    -1,    34,    -1,    36,    37,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    -1,    -1,    -1,    -1,    34,
      -1,    36,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    -1,    -1,    -1,
      -1,    34,    -1,    36,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    -1,    -1,    -1,
      -1,    34,    -1,    36,    25,    26,    27,    28,    29,    -1,
      -1,    -1,    -1,    34,    -1,    36
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,     6,    44,    45,    46,    63,     0,    37,    64,    12,
      36,    46,     4,     5,     7,     8,     9,    24,    31,    35,
      36,    42,    48,    51,    52,    53,    56,    58,    59,    60,
      61,    62,    63,    47,    63,    51,    51,    51,    11,    49,
      50,    51,    51,    54,    55,    63,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    34,    36,    38,    39,    10,    35,    46,    37,    37,
      64,    38,    13,    39,    12,    51,    51,    51,    51,    51,
      51,    51,    51,    51,    51,    51,    51,    51,    51,    51,
      51,    57,    57,    12,    63,    51,    45,    50,    51,    40,
      51,    55,    51,    39,    41,    38,    48,    14,    64,    64,
      51,    51,    40,    40
  };

  const unsigned char
  parser::yyr1_[] =
  {
       0,    43,    44,    44,    45,    45,    46,    46,    47,    47,
      47,    48,    48,    48,    49,    49,    50,    50,    51,    51,
      51,    51,    51,    51,    51,    51,    51,    51,    51,    51,
      51,    51,    51,    51,    51,    51,    51,    51,    51,    51,
      51,    51,    51,    52,    53,    54,    54,    55,    55,    56,
      57,    57,    58,    59,    59,    60,    61,    62,    62,    63,
      64,    64
  };

  const unsigned char
  parser::yyr2_[] =
  {
       0,     2,     2,     0,     1,     3,     6,     3,     0,     1,
       3,     1,     4,     6,     1,     3,     2,     5,     1,     1,
       1,     1,     1,     1,     1,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     3,     3,     3,
       3,     3,     3,     4,     4,     1,     3,     1,     3,     4,
       1,     3,     6,     1,     1,     1,     1,     1,     1,     1,
       1,     0
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const parser::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "\"invalid token\"", "INT",
  "REAL", "ID", "TRUE", "FALSE", "IF", "THEN", "LET", "'='", "RIGHT_ARROW",
  "ELSE", "LOGIC_OR", "LOGIC_AND", "EQ", "NEQ", "LESS", "MORE", "LESS_EQ",
  "MORE_EQ", "'+'", "'-'", "'*'", "'/'", "':'", "'%'", "'^'", "DOTDOT",
  "LOGIC_NOT", "UMINUS", "'#'", "'['", "'{'", "'('", "';'", "')'", "','",
  "'}'", "']'", "'\\\\'", "$accept", "program", "stmt_list", "stmt",
  "param_list", "expr_block", "let_block_list", "let_block", "expr",
  "array_apply", "array_func", "array_arg_list", "array_arg", "func_apply",
  "expr_list", "if_expr", "number", "int", "real", "boolean", "id",
  "optional_semicolon", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned short int
  parser::yyrline_[] =
  {
       0,    60,    60,    69,    76,    81,    90,    95,   103,   105,
     108,   117,   122,   127,   134,   136,   145,   150,   158,   160,
     162,   164,   166,   168,   170,   172,   175,   178,   181,   184,
     187,   190,   193,   196,   199,   202,   205,   208,   211,   214,
     217,   220,   223,   228,   233,   238,   241,   249,   252,   257,
     264,   267,   275,   280,   282,   285,   288,   292,   294,   297,
     300,   300
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
       2,     2,     2,     2,     2,    33,     2,    28,     2,     2,
      36,    38,    25,    23,    39,    24,     2,    26,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    27,    37,
       2,    12,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    34,    42,    41,    29,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    35,     2,    40,     2,     2,     2,     2,
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
       5,     6,     7,     8,     9,    10,    11,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    30,    31,    32
    };
    const unsigned int user_token_number_max_ = 279;
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
#line 1434 "parser.cpp" // lalr1.cc:1155
#line 303 "parser.y" // lalr1.cc:1156


void
stream::parsing::parser::error (const location_type& l, const std::string& m)
{
  driver.error (l, m);
}
