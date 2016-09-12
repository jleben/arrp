// A Bison parser, made by GNU Bison 3.0.4.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015 Free Software Foundation, Inc.

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

#line 37 "parser.cpp" // lalr1.cc:404

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

#include "parser.hpp"

// User implementation prologue.

#line 51 "parser.cpp" // lalr1.cc:412
// Unqualified %code blocks.
#line 52 "parser.y" // lalr1.cc:413

#include "driver.hpp"
#include "scanner.hpp"

#undef yylex
#define yylex driver.scanner.lex

using namespace stream::ast;
using op_type = stream::primitive_op;

#line 64 "parser.cpp" // lalr1.cc:413


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
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

#line 13 "parser.y" // lalr1.cc:479
namespace stream { namespace parsing {
#line 150 "parser.cpp" // lalr1.cc:479

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
    clear ();
  }

  template <typename Base>
  inline
  void
  parser::basic_symbol<Base>::clear ()
  {
    Base::clear ();
  }

  template <typename Base>
  inline
  bool
  parser::basic_symbol<Base>::empty () const
  {
    return Base::type_get () == empty_symbol;
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
    : type (empty_symbol)
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
  parser::by_type::clear ()
  {
    type = empty_symbol;
  }

  inline
  void
  parser::by_type::move (by_type& that)
  {
    type = that.type;
    that.clear ();
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
    : state (empty_state)
  {}

  inline
  parser::by_state::by_state (const by_state& other)
    : state (other.state)
  {}

  inline
  void
  parser::by_state::clear ()
  {
    state = empty_state;
  }

  inline
  void
  parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  inline
  parser::by_state::by_state (state_type s)
    : state (s)
  {}

  inline
  parser::symbol_number_type
  parser::by_state::type_get () const
  {
    if (state == empty_state)
      return empty_symbol;
    else
      return yystos_[state];
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
    that.type = empty_symbol;
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
    // Avoid a (spurious) G++ 4.8 warning about "array subscript is
    // below array bounds".
    if (yysym.empty ())
      std::abort ();
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
    if (yyla.empty ())
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
#line 68 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(program, yylhs.location, { (yystack_[2].value), (yystack_[1].value), (yystack_[0].value) });
    driver.m_ast = (yylhs.value);
  }
#line 633 "parser.cpp" // lalr1.cc:859
    break;

  case 3:
#line 76 "parser.y" // lalr1.cc:859
    { (yylhs.value) = nullptr; }
#line 639 "parser.cpp" // lalr1.cc:859
    break;

  case 4:
#line 79 "parser.y" // lalr1.cc:859
    { (yylhs.value) = (yystack_[1].value); }
#line 645 "parser.cpp" // lalr1.cc:859
    break;

  case 5:
#line 84 "parser.y" // lalr1.cc:859
    { (yylhs.value) = nullptr; }
#line 651 "parser.cpp" // lalr1.cc:859
    break;

  case 7:
#line 91 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value) } );
  }
#line 659 "parser.cpp" // lalr1.cc:859
    break;

  case 8:
#line 96 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 669 "parser.cpp" // lalr1.cc:859
    break;

  case 9:
#line 105 "parser.y" // lalr1.cc:859
    {
  (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value), nullptr } );
  }
#line 677 "parser.cpp" // lalr1.cc:859
    break;

  case 10:
#line 110 "parser.y" // lalr1.cc:859
    {
  (yylhs.value) = make_list( yylhs.location, { (yystack_[2].value), (yystack_[0].value) } );
  }
#line 685 "parser.cpp" // lalr1.cc:859
    break;

  case 11:
#line 117 "parser.y" // lalr1.cc:859
    { (yylhs.value) = nullptr; }
#line 691 "parser.cpp" // lalr1.cc:859
    break;

  case 13:
#line 124 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value) } );
  }
#line 699 "parser.cpp" // lalr1.cc:859
    break;

  case 14:
#line 129 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 709 "parser.cpp" // lalr1.cc:859
    break;

  case 15:
#line 138 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::binding, yylhs.location, {(yystack_[5].value), (yystack_[3].value), (yystack_[0].value)} );
  }
#line 717 "parser.cpp" // lalr1.cc:859
    break;

  case 16:
#line 143 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::binding, yylhs.location, {(yystack_[2].value), nullptr, (yystack_[0].value)} );
  }
#line 725 "parser.cpp" // lalr1.cc:859
    break;

  case 17:
#line 150 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {} ); }
#line 731 "parser.cpp" // lalr1.cc:859
    break;

  case 18:
#line 153 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 737 "parser.cpp" // lalr1.cc:859
    break;

  case 19:
#line 156 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 747 "parser.cpp" // lalr1.cc:859
    break;

  case 34:
#line 193 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( array_concat, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} ); }
#line 753 "parser.cpp" // lalr1.cc:859
    break;

  case 35:
#line 196 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::negate), (yystack_[0].value)} ); }
#line 759 "parser.cpp" // lalr1.cc:859
    break;

  case 36:
#line 199 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::logic_or), (yystack_[2].value), (yystack_[0].value)} ); }
#line 765 "parser.cpp" // lalr1.cc:859
    break;

  case 37:
#line 202 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::logic_and), (yystack_[2].value), (yystack_[0].value)} ); }
#line 771 "parser.cpp" // lalr1.cc:859
    break;

  case 38:
#line 205 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_eq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 777 "parser.cpp" // lalr1.cc:859
    break;

  case 39:
#line 208 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_neq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 783 "parser.cpp" // lalr1.cc:859
    break;

  case 40:
#line 211 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_l), (yystack_[2].value), (yystack_[0].value)} ); }
#line 789 "parser.cpp" // lalr1.cc:859
    break;

  case 41:
#line 214 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_leq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 795 "parser.cpp" // lalr1.cc:859
    break;

  case 42:
#line 217 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_g), (yystack_[2].value), (yystack_[0].value)} ); }
#line 801 "parser.cpp" // lalr1.cc:859
    break;

  case 43:
#line 220 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_geq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 807 "parser.cpp" // lalr1.cc:859
    break;

  case 44:
#line 223 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::add), (yystack_[2].value), (yystack_[0].value)} ); }
#line 813 "parser.cpp" // lalr1.cc:859
    break;

  case 45:
#line 226 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::subtract), (yystack_[2].value), (yystack_[0].value)} ); }
#line 819 "parser.cpp" // lalr1.cc:859
    break;

  case 46:
#line 229 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::negate), (yystack_[0].value)} ); }
#line 825 "parser.cpp" // lalr1.cc:859
    break;

  case 47:
#line 232 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::multiply), (yystack_[2].value), (yystack_[0].value)} ); }
#line 831 "parser.cpp" // lalr1.cc:859
    break;

  case 48:
#line 235 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::divide), (yystack_[2].value), (yystack_[0].value)} ); }
#line 837 "parser.cpp" // lalr1.cc:859
    break;

  case 49:
#line 238 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::divide_integer), (yystack_[2].value), (yystack_[0].value)} ); }
#line 843 "parser.cpp" // lalr1.cc:859
    break;

  case 50:
#line 241 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::modulo), (yystack_[2].value), (yystack_[0].value)} ); }
#line 849 "parser.cpp" // lalr1.cc:859
    break;

  case 51:
#line 244 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::raise), (yystack_[2].value), (yystack_[0].value)} ); }
#line 855 "parser.cpp" // lalr1.cc:859
    break;

  case 52:
#line 247 "parser.y" // lalr1.cc:859
    { (yylhs.value) = (yystack_[1].value); }
#line 861 "parser.cpp" // lalr1.cc:859
    break;

  case 55:
#line 254 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::binding, yylhs.location, {(yystack_[2].value), nullptr, (yystack_[0].value)} );
  }
#line 869 "parser.cpp" // lalr1.cc:859
    break;

  case 56:
#line 262 "parser.y" // lalr1.cc:859
    {
    auto bnd_list = make_list(yystack_[2].location, {(yystack_[2].value)});
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { bnd_list, (yystack_[0].value) } );
  }
#line 878 "parser.cpp" // lalr1.cc:859
    break;

  case 57:
#line 268 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { (yystack_[4].value), (yystack_[0].value) } );
  }
#line 886 "parser.cpp" // lalr1.cc:859
    break;

  case 58:
#line 275 "parser.y" // lalr1.cc:859
    {
    auto bnd_list = make_list(yystack_[0].location, {(yystack_[0].value)});
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { bnd_list, (yystack_[2].value) } );
  }
#line 895 "parser.cpp" // lalr1.cc:859
    break;

  case 59:
#line 281 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { (yystack_[2].value), (yystack_[5].value) } );
  }
#line 903 "parser.cpp" // lalr1.cc:859
    break;

  case 60:
#line 288 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(ast::lambda, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} );
  }
#line 911 "parser.cpp" // lalr1.cc:859
    break;

  case 61:
#line 295 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} ); }
#line 917 "parser.cpp" // lalr1.cc:859
    break;

  case 62:
#line 300 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} ); }
#line 923 "parser.cpp" // lalr1.cc:859
    break;

  case 63:
#line 305 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_def, yylhs.location, {(yystack_[4].value), (yystack_[2].value)} ); }
#line 929 "parser.cpp" // lalr1.cc:859
    break;

  case 64:
#line 308 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_def, yylhs.location, {nullptr, (yystack_[2].value)} ); }
#line 935 "parser.cpp" // lalr1.cc:859
    break;

  case 66:
#line 318 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 941 "parser.cpp" // lalr1.cc:859
    break;

  case 67:
#line 321 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 951 "parser.cpp" // lalr1.cc:859
    break;

  case 68:
#line 330 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[2].value), nullptr, (yystack_[0].value) } ); }
#line 957 "parser.cpp" // lalr1.cc:859
    break;

  case 69:
#line 333 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[3].value), (yystack_[2].value), (yystack_[0].value) } ); }
#line 963 "parser.cpp" // lalr1.cc:859
    break;

  case 70:
#line 339 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 969 "parser.cpp" // lalr1.cc:859
    break;

  case 71:
#line 342 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[1].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 979 "parser.cpp" // lalr1.cc:859
    break;

  case 72:
#line 350 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[2].value), (yystack_[0].value) } ); }
#line 985 "parser.cpp" // lalr1.cc:859
    break;

  case 73:
#line 355 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[1].value);
    (yylhs.value)->type = ast::array_enum;
    (yylhs.value)->location = yylhs.location;
  }
#line 995 "parser.cpp" // lalr1.cc:859
    break;

  case 74:
#line 364 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1001 "parser.cpp" // lalr1.cc:859
    break;

  case 75:
#line 367 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1011 "parser.cpp" // lalr1.cc:859
    break;

  case 76:
#line 376 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( array_size, yylhs.location, { (yystack_[0].value), nullptr } ); }
#line 1017 "parser.cpp" // lalr1.cc:859
    break;

  case 77:
#line 379 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( array_size, yylhs.location, { (yystack_[2].value), (yystack_[0].value) } ); }
#line 1023 "parser.cpp" // lalr1.cc:859
    break;

  case 78:
#line 384 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::func_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} );
  }
#line 1031 "parser.cpp" // lalr1.cc:859
    break;

  case 79:
#line 391 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::func_compose, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} );
  }
#line 1039 "parser.cpp" // lalr1.cc:859
    break;

  case 80:
#line 398 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1045 "parser.cpp" // lalr1.cc:859
    break;

  case 81:
#line 401 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
  }
#line 1054 "parser.cpp" // lalr1.cc:859
    break;

  case 82:
#line 409 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[5].location,op_type::conditional), (yystack_[4].value), (yystack_[2].value), (yystack_[0].value)} ); }
#line 1060 "parser.cpp" // lalr1.cc:859
    break;

  case 93:
#line 442 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_node(infinity, yylhs.location); }
#line 1066 "parser.cpp" // lalr1.cc:859
    break;


#line 1070 "parser.cpp" // lalr1.cc:859
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
        error (yyla.location, yysyntax_error_ (yystack_[0].state, yyla));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.type_get () == yyeof_)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
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
    if (!yyla.empty ())
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
        if (!yyla.empty ())
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
  parser::yysyntax_error_ (state_type yystate, const symbol_type& yyla) const
  {
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
       - The only way there can be no lookahead present (in yyla) is
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
    if (!yyla.empty ())
      {
        int yytoken = yyla.type_get ();
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

    std::string yyres;
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


  const signed char parser::yypact_ninf_ = -73;

  const signed char parser::yytable_ninf_ = -75;

  const short int
  parser::yypact_[] =
  {
      -9,    -1,    13,    17,   -73,   -19,   -73,    -1,    -1,    -4,
     -73,   -73,    26,   -73,     5,   -73,   -14,    17,    -1,    -1,
     -73,   105,    -1,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   105,     8,    -6,   105,   105,   105,   105,   105,
      -1,   -73,   274,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,    37,
     -73,   -73,   -23,   -73,   170,   105,    -1,    60,   -24,   -24,
      54,   139,    20,    39,   -73,   -43,   -13,   195,   -20,    -5,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   105,
      67,    -1,   105,   274,   -25,     5,   105,   105,   105,   105,
      36,   105,   -73,   105,   105,   105,    47,   -73,   -73,   105,
      -1,   -73,   321,   343,   359,   359,   359,   359,   359,   359,
     374,   387,   387,     3,     3,     3,     3,   -24,   -24,    -2,
      44,   274,   105,   -73,   223,   -73,    48,   274,   -73,    39,
     -13,   -73,   -73,   274,   274,   274,   249,   105,   -73,   298,
       5,   -73,   -73,   274,   105,    83,    51,   105,   249,    55,
     298,   105,   -73,   274,   -73,   274
  };

  const unsigned char
  parser::yydefact_[] =
  {
       3,     0,     0,     5,    91,     0,     1,     0,    11,     0,
       7,     4,     9,     2,    95,    13,     0,     6,     0,    94,
      12,     0,    17,     8,    10,    14,    86,    87,    88,    89,
      90,    92,     0,     0,     0,     0,     0,     0,     0,     0,
      17,    93,    16,    53,    54,    26,    31,    32,    29,    30,
      33,    27,    28,    25,    22,    83,    84,    85,    24,    20,
      21,    23,     0,    18,     0,     0,     0,     0,    46,    35,
      76,    80,     0,    95,    66,     0,    65,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    80,     0,    95,     0,     0,     0,    94,
       0,     0,    73,     0,     0,     0,     0,    70,    52,     0,
       0,    58,    36,    37,    38,    39,    40,    42,    41,    43,
      34,    44,    45,    47,    48,    49,    50,    51,    79,     0,
       0,    55,     0,    19,     0,    62,     0,    56,    77,    95,
       0,    67,    64,    75,    68,    81,     0,     0,    71,    60,
      95,    61,    78,    15,     0,     0,     0,     0,    69,     0,
      82,     0,    63,    72,    59,    57
  };

  const signed char
  parser::yypgoto_[] =
  {
     -73,   -73,   -73,   -73,   -73,    90,   -73,   -64,     4,    74,
     -21,   -73,   -73,   -73,   -73,   -73,   -73,   -73,    10,    14,
     -73,     6,   -73,   -73,   -73,   -73,   -73,   -18,   -73,   -73,
     -73,   -73,   -73,   -73,    18,   -73,   -73,   -72
  };

  const short int
  parser::yydefgoto_[] =
  {
      -1,     2,     3,     8,     9,    10,    13,    14,    15,    62,
     103,    43,    44,    45,    46,    47,    48,    72,    73,    74,
     116,   117,    49,    75,    50,    51,    52,   150,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    20
  };

  const short int
  parser::yytable_[] =
  {
      42,   110,   105,   119,     4,     4,    21,   111,     1,     4,
     113,    64,   112,     6,    68,    69,    70,    71,    77,     5,
      76,    96,    97,    25,    98,    12,    16,   114,   100,   101,
     145,    11,   101,   146,    22,     7,    24,    16,    67,   114,
      63,    66,   120,    95,   115,    18,    17,   104,    96,    97,
     114,    98,    16,   161,    65,    19,   160,    99,    63,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   108,   166,   141,   139,
     140,   144,   106,   121,    16,   147,   148,   142,   169,   109,
     153,   152,   154,   155,   156,   162,   114,    16,   159,    96,
      97,   165,    98,   107,   157,   171,   172,    23,   174,    26,
      27,    28,    29,    30,    78,     4,    31,    32,   149,   143,
      33,   163,   158,   151,     0,     0,    34,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   168,     0,    16,     0,
      35,     0,     0,   170,     0,     0,   173,    36,     0,    37,
     175,    38,     0,    39,     0,    79,     0,     0,     0,    40,
       0,     0,     0,    41,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
       0,     0,     0,   102,    96,    97,    79,    98,     0,   -74,
       0,     0,     0,     0,   -74,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    79,     0,     0,     0,    96,    97,     0,    98,     0,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,     0,     0,     0,    79,
      96,    97,     0,    98,     0,     0,   118,   164,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,     0,    79,     0,     0,    96,    97,
       0,    98,   167,     0,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      79,     0,     0,     0,    96,    97,     0,    98,     0,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,     0,     0,     0,     0,    96,
      97,     0,    98,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,     0,
       0,     0,     0,    96,    97,     0,    98,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,     0,     0,     0,     0,    96,    97,     0,    98,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,     0,     0,     0,     0,    96,    97,
       0,    98,    88,    89,    90,    91,    92,    93,    94,    95,
       0,     0,     0,     0,    96,    97,     0,    98,    89,    90,
      91,    92,    93,    94,    95,     0,     0,     0,     0,    96,
      97,     0,    98,    91,    92,    93,    94,    95,     0,     0,
       0,     0,    96,    97,     0,    98
  };

  const short int
  parser::yycheck_[] =
  {
      21,    73,    66,    23,    10,    10,    20,    50,    17,    10,
      23,    32,    55,     0,    35,    36,    37,    38,    39,     1,
      38,    45,    46,    19,    48,     7,     8,    52,    51,    52,
      55,    50,    52,   105,    48,    18,    18,    19,    34,    52,
      22,    47,    47,    40,    57,    19,    50,    65,    45,    46,
      52,    48,    34,    55,    46,    50,   120,    20,    40,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    56,   149,    99,    97,
      98,   102,    22,    79,    66,   106,   107,    20,   160,    50,
     111,    55,   113,   114,   115,    51,    52,    79,   119,    45,
      46,    53,    48,    49,    57,    22,    55,    17,    53,     4,
       5,     6,     7,     8,    40,    10,    11,    12,   108,   101,
      15,   142,   116,   109,    -1,    -1,    21,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,   120,    -1,
      35,    -1,    -1,   164,    -1,    -1,   167,    42,    -1,    44,
     171,    46,    -1,    48,    -1,    16,    -1,    -1,    -1,    54,
      -1,    -1,    -1,    58,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      -1,    -1,    -1,    13,    45,    46,    16,    48,    -1,    50,
      -1,    -1,    -1,    -1,    55,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    16,    -1,    -1,    -1,    45,    46,    -1,    48,    -1,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    -1,    -1,    -1,    16,
      45,    46,    -1,    48,    -1,    -1,    51,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    -1,    16,    -1,    -1,    45,    46,
      -1,    48,    23,    -1,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      16,    -1,    -1,    -1,    45,    46,    -1,    48,    -1,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    -1,    -1,    -1,    -1,    45,
      46,    -1,    48,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    -1,
      -1,    -1,    -1,    45,    46,    -1,    48,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    -1,    -1,    -1,    -1,    45,    46,    -1,    48,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    -1,    -1,    -1,    -1,    45,    46,
      -1,    48,    33,    34,    35,    36,    37,    38,    39,    40,
      -1,    -1,    -1,    -1,    45,    46,    -1,    48,    34,    35,
      36,    37,    38,    39,    40,    -1,    -1,    -1,    -1,    45,
      46,    -1,    48,    36,    37,    38,    39,    40,    -1,    -1,
      -1,    -1,    45,    46,    -1,    48
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,    17,    60,    61,    10,    93,     0,    18,    62,    63,
      64,    50,    93,    65,    66,    67,    93,    50,    19,    50,
      96,    20,    48,    64,    93,    67,     4,     5,     6,     7,
       8,    11,    12,    15,    21,    35,    42,    44,    46,    48,
      54,    58,    69,    70,    71,    72,    73,    74,    75,    81,
      83,    84,    85,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    68,    93,    69,    46,    47,    67,    69,    69,
      69,    69,    76,    77,    78,    82,    86,    69,    68,    16,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    45,    46,    48,    20,
      51,    52,    13,    69,    86,    66,    22,    49,    56,    50,
      96,    50,    55,    23,    52,    57,    79,    80,    51,    23,
      47,    67,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    86,
      86,    69,    20,    93,    69,    55,    96,    69,    69,    77,
      86,    78,    55,    69,    69,    69,    69,    57,    80,    69,
      66,    55,    51,    69,    24,    53,    96,    23,    69,    96,
      69,    22,    55,    69,    53,    69
  };

  const unsigned char
  parser::yyr1_[] =
  {
       0,    59,    60,    61,    61,    62,    62,    63,    63,    64,
      64,    65,    65,    66,    66,    67,    67,    68,    68,    68,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    70,    70,    71,    71,
      72,    73,    74,    75,    75,    76,    77,    77,    78,    78,
      79,    79,    80,    81,    82,    82,    83,    83,    84,    85,
      86,    86,    87,    88,    88,    88,    89,    90,    91,    92,
      92,    93,    94,    95,    96,    96
  };

  const unsigned char
  parser::yyr2_[] =
  {
       0,     2,     3,     0,     3,     0,     2,     1,     3,     2,
       4,     0,     2,     1,     3,     6,     3,     0,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     3,     3,     3,
       3,     3,     3,     1,     1,     3,     4,     7,     3,     6,
       4,     4,     4,     6,     4,     1,     1,     3,     3,     4,
       1,     2,     4,     3,     1,     3,     2,     4,     4,     3,
       1,     3,     6,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     0
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const parser::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "\"invalid token\"", "INT",
  "REAL", "COMPLEX", "TRUE", "FALSE", "STRING", "ID", "QUALIFIED_ID", "IF",
  "THEN", "CASE", "THIS", "WHERE", "MODULE", "IMPORT", "AS", "'='", "LET",
  "IN", "RIGHT_ARROW", "ELSE", "LOGIC_OR", "LOGIC_AND", "EQ", "NEQ",
  "LESS", "MORE", "LESS_EQ", "MORE_EQ", "PLUSPLUS", "'+'", "'-'", "'*'",
  "'/'", "INT_DIV", "'%'", "'^'", "DOTDOT", "LOGIC_NOT", "UMINUS", "'#'",
  "'.'", "'['", "'{'", "'('", "'@'", "';'", "')'", "','", "'}'", "'\\\\'",
  "']'", "':'", "'|'", "'~'", "$accept", "program", "module_decl",
  "imports", "import_list", "import", "bindings", "binding_list",
  "binding", "param_list", "expr", "let_expr", "where_expr", "lambda",
  "array_apply", "array_self_apply", "array_func", "array_ranges",
  "array_pattern_list", "array_pattern", "array_domain_list",
  "array_domain", "array_enum", "array_elem_list", "array_size",
  "func_apply", "func_composition", "expr_list", "if_expr", "number",
  "int", "real", "complex", "boolean", "id", "qualified_id", "inf",
  "optional_semicolon", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned short int
  parser::yyrline_[] =
  {
       0,    67,    67,    76,    78,    84,    86,    90,    95,   104,
     109,   117,   119,   123,   128,   137,   142,   150,   152,   155,
     164,   166,   168,   170,   172,   174,   176,   178,   180,   182,
     184,   186,   188,   190,   192,   195,   198,   201,   204,   207,
     210,   213,   216,   219,   222,   225,   228,   231,   234,   237,
     240,   243,   246,   249,   251,   253,   261,   267,   274,   280,
     287,   294,   299,   304,   307,   312,   317,   320,   329,   332,
     338,   341,   349,   354,   363,   366,   375,   378,   383,   390,
     397,   400,   408,   413,   415,   417,   420,   423,   426,   430,
     432,   435,   438,   441,   445,   445
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
       2,     2,     2,     2,     2,    44,     2,    39,     2,     2,
      48,    51,    36,    34,    52,    35,    45,    37,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    56,    50,
       2,    20,     2,     2,    49,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    46,    54,    55,    40,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    47,    57,    53,    58,     2,     2,     2,
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
      15,    16,    17,    18,    19,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    38,    41,
      42,    43
    };
    const unsigned int user_token_number_max_ = 291;
    const token_number_type undef_token_ = 2;

    if (static_cast<int>(t) <= yyeof_)
      return yyeof_;
    else if (static_cast<unsigned int> (t) <= user_token_number_max_)
      return translate_table[t];
    else
      return undef_token_;
  }

#line 13 "parser.y" // lalr1.cc:1167
} } // stream::parsing
#line 1667 "parser.cpp" // lalr1.cc:1167
#line 448 "parser.y" // lalr1.cc:1168


void
stream::parsing::parser::error (const location_type& l, const std::string& m)
{
  driver.error (l, m);
}
