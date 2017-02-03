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
#line 53 "parser.y" // lalr1.cc:413

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
#line 69 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(program, yylhs.location, { (yystack_[2].value), (yystack_[1].value), (yystack_[0].value) });
    driver.m_ast = (yylhs.value);
  }
#line 633 "parser.cpp" // lalr1.cc:859
    break;

  case 3:
#line 77 "parser.y" // lalr1.cc:859
    { (yylhs.value) = nullptr; }
#line 639 "parser.cpp" // lalr1.cc:859
    break;

  case 4:
#line 80 "parser.y" // lalr1.cc:859
    { (yylhs.value) = (yystack_[1].value); }
#line 645 "parser.cpp" // lalr1.cc:859
    break;

  case 5:
#line 85 "parser.y" // lalr1.cc:859
    { (yylhs.value) = nullptr; }
#line 651 "parser.cpp" // lalr1.cc:859
    break;

  case 7:
#line 92 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value) } );
  }
#line 659 "parser.cpp" // lalr1.cc:859
    break;

  case 8:
#line 97 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 669 "parser.cpp" // lalr1.cc:859
    break;

  case 9:
#line 106 "parser.y" // lalr1.cc:859
    {
  (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value), nullptr } );
  }
#line 677 "parser.cpp" // lalr1.cc:859
    break;

  case 10:
#line 111 "parser.y" // lalr1.cc:859
    {
  (yylhs.value) = make_list( yylhs.location, { (yystack_[2].value), (yystack_[0].value) } );
  }
#line 685 "parser.cpp" // lalr1.cc:859
    break;

  case 11:
#line 118 "parser.y" // lalr1.cc:859
    { (yylhs.value) = nullptr; }
#line 691 "parser.cpp" // lalr1.cc:859
    break;

  case 13:
#line 125 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value) } );
  }
#line 699 "parser.cpp" // lalr1.cc:859
    break;

  case 14:
#line 130 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 709 "parser.cpp" // lalr1.cc:859
    break;

  case 17:
#line 143 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list(ast::input, yylhs.location, {(yystack_[2].value), (yystack_[0].value)}); }
#line 715 "parser.cpp" // lalr1.cc:859
    break;

  case 18:
#line 146 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list(ast::external, yylhs.location, {(yystack_[2].value), (yystack_[0].value)}); }
#line 721 "parser.cpp" // lalr1.cc:859
    break;

  case 19:
#line 152 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::binding, yylhs.location, {(yystack_[6].value), (yystack_[2].value), (yystack_[4].value), (yystack_[0].value)} );
  }
#line 729 "parser.cpp" // lalr1.cc:859
    break;

  case 20:
#line 157 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::binding, yylhs.location, {(yystack_[3].value), (yystack_[2].value), nullptr, (yystack_[0].value)} );
  }
#line 737 "parser.cpp" // lalr1.cc:859
    break;

  case 21:
#line 164 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value) } );
  }
#line 745 "parser.cpp" // lalr1.cc:859
    break;

  case 22:
#line 169 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 755 "parser.cpp" // lalr1.cc:859
    break;

  case 23:
#line 178 "parser.y" // lalr1.cc:859
    { (yylhs.value) = nullptr; }
#line 761 "parser.cpp" // lalr1.cc:859
    break;

  case 24:
#line 181 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[0].value);
  }
#line 769 "parser.cpp" // lalr1.cc:859
    break;

  case 25:
#line 188 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {} ); }
#line 775 "parser.cpp" // lalr1.cc:859
    break;

  case 26:
#line 191 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 781 "parser.cpp" // lalr1.cc:859
    break;

  case 27:
#line 194 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 791 "parser.cpp" // lalr1.cc:859
    break;

  case 30:
#line 207 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list(ast::function_type, yylhs.location, {(yystack_[2].value), (yystack_[0].value)}); }
#line 797 "parser.cpp" // lalr1.cc:859
    break;

  case 31:
#line 212 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 803 "parser.cpp" // lalr1.cc:859
    break;

  case 32:
#line 215 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 813 "parser.cpp" // lalr1.cc:859
    break;

  case 35:
#line 228 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list(ast::array_type, yylhs.location, {(yystack_[2].value), (yystack_[0].value)}); }
#line 819 "parser.cpp" // lalr1.cc:859
    break;

  case 51:
#line 266 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( array_concat, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} ); }
#line 825 "parser.cpp" // lalr1.cc:859
    break;

  case 52:
#line 269 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::negate), (yystack_[0].value)} ); }
#line 831 "parser.cpp" // lalr1.cc:859
    break;

  case 53:
#line 272 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::logic_or), (yystack_[2].value), (yystack_[0].value)} ); }
#line 837 "parser.cpp" // lalr1.cc:859
    break;

  case 54:
#line 275 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::logic_and), (yystack_[2].value), (yystack_[0].value)} ); }
#line 843 "parser.cpp" // lalr1.cc:859
    break;

  case 55:
#line 278 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_eq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 849 "parser.cpp" // lalr1.cc:859
    break;

  case 56:
#line 281 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_neq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 855 "parser.cpp" // lalr1.cc:859
    break;

  case 57:
#line 284 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_l), (yystack_[2].value), (yystack_[0].value)} ); }
#line 861 "parser.cpp" // lalr1.cc:859
    break;

  case 58:
#line 287 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_leq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 867 "parser.cpp" // lalr1.cc:859
    break;

  case 59:
#line 290 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_g), (yystack_[2].value), (yystack_[0].value)} ); }
#line 873 "parser.cpp" // lalr1.cc:859
    break;

  case 60:
#line 293 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_geq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 879 "parser.cpp" // lalr1.cc:859
    break;

  case 61:
#line 296 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::add), (yystack_[2].value), (yystack_[0].value)} ); }
#line 885 "parser.cpp" // lalr1.cc:859
    break;

  case 62:
#line 299 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::subtract), (yystack_[2].value), (yystack_[0].value)} ); }
#line 891 "parser.cpp" // lalr1.cc:859
    break;

  case 63:
#line 302 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::negate), (yystack_[0].value)} ); }
#line 897 "parser.cpp" // lalr1.cc:859
    break;

  case 64:
#line 305 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::multiply), (yystack_[2].value), (yystack_[0].value)} ); }
#line 903 "parser.cpp" // lalr1.cc:859
    break;

  case 65:
#line 308 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::divide), (yystack_[2].value), (yystack_[0].value)} ); }
#line 909 "parser.cpp" // lalr1.cc:859
    break;

  case 66:
#line 311 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::divide_integer), (yystack_[2].value), (yystack_[0].value)} ); }
#line 915 "parser.cpp" // lalr1.cc:859
    break;

  case 67:
#line 314 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::modulo), (yystack_[2].value), (yystack_[0].value)} ); }
#line 921 "parser.cpp" // lalr1.cc:859
    break;

  case 68:
#line 317 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::raise), (yystack_[2].value), (yystack_[0].value)} ); }
#line 927 "parser.cpp" // lalr1.cc:859
    break;

  case 69:
#line 320 "parser.y" // lalr1.cc:859
    { (yylhs.value) = (yystack_[1].value); }
#line 933 "parser.cpp" // lalr1.cc:859
    break;

  case 72:
#line 327 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::binding, yylhs.location, {(yystack_[4].value), (yystack_[2].value), nullptr, (yystack_[0].value)} );
  }
#line 941 "parser.cpp" // lalr1.cc:859
    break;

  case 73:
#line 335 "parser.y" // lalr1.cc:859
    {
    auto bnd_list = make_list(yystack_[2].location, {(yystack_[2].value)});
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { bnd_list, (yystack_[0].value) } );
  }
#line 950 "parser.cpp" // lalr1.cc:859
    break;

  case 74:
#line 341 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { (yystack_[4].value), (yystack_[0].value) } );
  }
#line 958 "parser.cpp" // lalr1.cc:859
    break;

  case 75:
#line 348 "parser.y" // lalr1.cc:859
    {
    auto bnd_list = make_list(yystack_[0].location, {(yystack_[0].value)});
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { bnd_list, (yystack_[2].value) } );
  }
#line 967 "parser.cpp" // lalr1.cc:859
    break;

  case 76:
#line 354 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { (yystack_[2].value), (yystack_[5].value) } );
  }
#line 975 "parser.cpp" // lalr1.cc:859
    break;

  case 77:
#line 361 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(ast::lambda, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} );
  }
#line 983 "parser.cpp" // lalr1.cc:859
    break;

  case 78:
#line 368 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} ); }
#line 989 "parser.cpp" // lalr1.cc:859
    break;

  case 79:
#line 373 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} ); }
#line 995 "parser.cpp" // lalr1.cc:859
    break;

  case 80:
#line 378 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_def, yylhs.location, {(yystack_[4].value), (yystack_[2].value)} ); }
#line 1001 "parser.cpp" // lalr1.cc:859
    break;

  case 81:
#line 381 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_def, yylhs.location, {nullptr, (yystack_[2].value)} ); }
#line 1007 "parser.cpp" // lalr1.cc:859
    break;

  case 83:
#line 391 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1013 "parser.cpp" // lalr1.cc:859
    break;

  case 84:
#line 394 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1023 "parser.cpp" // lalr1.cc:859
    break;

  case 85:
#line 403 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[2].value), nullptr, (yystack_[0].value) } ); }
#line 1029 "parser.cpp" // lalr1.cc:859
    break;

  case 86:
#line 406 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[3].value), (yystack_[2].value), (yystack_[0].value) } ); }
#line 1035 "parser.cpp" // lalr1.cc:859
    break;

  case 87:
#line 412 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1041 "parser.cpp" // lalr1.cc:859
    break;

  case 88:
#line 415 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[1].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1051 "parser.cpp" // lalr1.cc:859
    break;

  case 89:
#line 423 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[2].value), (yystack_[0].value) } ); }
#line 1057 "parser.cpp" // lalr1.cc:859
    break;

  case 90:
#line 428 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[1].value);
    (yylhs.value)->type = ast::array_enum;
    (yylhs.value)->location = yylhs.location;
  }
#line 1067 "parser.cpp" // lalr1.cc:859
    break;

  case 91:
#line 437 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1073 "parser.cpp" // lalr1.cc:859
    break;

  case 92:
#line 440 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1083 "parser.cpp" // lalr1.cc:859
    break;

  case 93:
#line 449 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( array_size, yylhs.location, { (yystack_[0].value), nullptr } ); }
#line 1089 "parser.cpp" // lalr1.cc:859
    break;

  case 94:
#line 452 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( array_size, yylhs.location, { (yystack_[2].value), (yystack_[0].value) } ); }
#line 1095 "parser.cpp" // lalr1.cc:859
    break;

  case 95:
#line 457 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::func_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} );
  }
#line 1103 "parser.cpp" // lalr1.cc:859
    break;

  case 96:
#line 464 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::func_compose, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} );
  }
#line 1111 "parser.cpp" // lalr1.cc:859
    break;

  case 97:
#line 471 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1117 "parser.cpp" // lalr1.cc:859
    break;

  case 98:
#line 474 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
  }
#line 1126 "parser.cpp" // lalr1.cc:859
    break;

  case 99:
#line 482 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[5].location,op_type::conditional), (yystack_[4].value), (yystack_[2].value), (yystack_[0].value)} ); }
#line 1132 "parser.cpp" // lalr1.cc:859
    break;

  case 110:
#line 515 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_node(infinity, yylhs.location); }
#line 1138 "parser.cpp" // lalr1.cc:859
    break;


#line 1142 "parser.cpp" // lalr1.cc:859
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


  const short int parser::yypact_ninf_ = -134;

  const signed char parser::yytable_ninf_ = -92;

  const short int
  parser::yypact_[] =
  {
      11,    41,    36,    35,  -134,    32,  -134,    41,    13,    34,
    -134,  -134,    43,    41,    41,  -134,    38,  -134,  -134,  -134,
       4,    35,    41,    74,    79,    13,    12,    41,    82,  -134,
    -134,    12,    12,  -134,   104,  -134,  -134,   -17,    -6,  -134,
    -134,  -134,    26,  -134,   104,  -134,  -134,  -134,  -134,  -134,
    -134,  -134,  -134,   104,    64,     0,   104,   104,   104,   104,
     104,    41,  -134,   290,  -134,  -134,  -134,  -134,  -134,  -134,
    -134,  -134,  -134,  -134,    51,  -134,  -134,  -134,  -134,  -134,
    -134,    99,  -134,  -134,    12,    12,   118,    41,   290,   178,
     104,    41,   129,    78,    78,    -4,   147,    84,   111,  -134,
      39,   -18,   206,     3,     9,   104,   104,   104,   104,   104,
     104,   104,   104,   104,   104,   104,   104,   104,   104,   104,
     104,   104,   104,   104,   104,    41,    12,  -134,  -134,   144,
    -134,   104,    62,  -134,   114,   104,   104,   104,   104,   112,
     104,  -134,   104,   104,   109,  -134,  -134,   104,    41,  -134,
     337,   359,    97,    97,    97,    97,    97,    97,   374,   387,
     387,    -8,    -8,    -8,    -8,    78,    78,    76,   103,   290,
    -134,   148,   104,   234,  -134,    41,   115,   290,  -134,   111,
     -18,  -134,  -134,   290,   290,   262,   104,  -134,   314,   114,
    -134,  -134,   104,   290,   104,  -134,   146,   117,   104,   262,
     135,   290,   314,   104,  -134,   290,  -134,   290
  };

  const unsigned char
  parser::yydefact_[] =
  {
       3,     0,     0,     5,   108,     0,     1,     0,    11,     0,
       7,     4,     9,     0,     0,     2,     0,    13,    15,    16,
      23,     6,     0,     0,     0,    12,     0,    25,     0,     8,
      10,     0,     0,    14,     0,    24,    29,     0,    28,    33,
      34,    36,     0,    26,     0,    17,    18,   103,   104,   105,
     106,   107,   109,     0,     0,     0,     0,     0,     0,     0,
       0,    25,   110,    97,    70,    71,    43,    48,    49,    46,
      47,    50,    44,    45,     0,    42,    39,   100,   101,   102,
      41,    37,    38,    40,     0,     0,    23,     0,    20,     0,
       0,     0,     0,    63,    52,    93,    97,     0,   112,    83,
       0,    82,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    30,    32,     0,
      27,     0,     0,    21,   112,     0,     0,     0,   111,     0,
       0,    90,     0,     0,     0,    87,    69,     0,     0,    75,
      53,    54,    55,    56,    57,    59,    58,    60,    51,    61,
      62,    64,    65,    66,    67,    68,    96,     0,     0,    98,
      35,     0,     0,     0,    79,   111,     0,    73,    94,   112,
       0,    84,    81,    92,    85,     0,     0,    88,    77,   112,
      78,    95,     0,    19,     0,    22,     0,     0,     0,    86,
       0,    72,    99,     0,    80,    89,    76,    74
  };

  const short int
  parser::yypgoto_[] =
  {
    -134,  -134,  -134,  -134,  -134,   172,  -134,  -134,   174,  -134,
     -52,    49,   116,   140,   -27,  -134,  -134,    75,  -134,    80,
     -42,  -134,  -134,  -134,  -134,  -134,  -134,  -134,    67,    85,
    -134,    81,  -134,  -134,  -134,  -134,  -134,    -2,  -134,  -134,
    -134,  -134,  -134,  -134,    -1,  -134,  -134,  -133
  };

  const short int
  parser::yydefgoto_[] =
  {
      -1,     2,     3,     8,     9,    10,    15,    16,    17,    18,
      19,   134,    28,    42,    35,    36,    37,    38,    39,    40,
      63,    64,    65,    66,    67,    68,    69,    97,    98,    99,
     144,   145,    70,   100,    71,    72,    73,   180,    75,    76,
      77,    78,    79,    80,    81,    82,    83,   139
  };

  const short int
  parser::yytable_[] =
  {
       5,   176,    88,    92,    45,    46,    12,    20,   142,    84,
       4,    89,    23,    24,    93,    94,    95,    96,   102,     4,
     -31,    30,     4,     4,    20,    41,    43,    26,     1,   147,
      41,    41,    74,    13,    14,   120,     6,   124,    85,   133,
     121,   122,   143,   123,   121,   122,   197,   123,   136,   -31,
      91,     4,   149,     7,    20,    27,   200,   101,    87,   148,
      43,    34,    22,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
      86,    87,   169,    41,    41,    11,   130,    21,   132,   173,
      20,    25,   140,   177,   178,   141,   133,    31,   183,   171,
     184,   185,    32,    20,    44,   188,   124,   125,    47,    48,
      49,    50,    51,    90,     4,    52,    53,   124,   174,    54,
     167,   168,   126,   195,    41,    41,   121,   122,    55,   123,
     193,   124,   190,   113,   114,   115,   116,   117,   118,   119,
     120,    26,    56,   137,   199,   121,   122,    20,   123,    57,
     201,    58,   202,    59,   135,    60,   205,   191,   124,   127,
     128,   207,    61,   104,   138,    62,   172,   175,   182,   186,
     192,   203,   196,   204,    20,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   131,   206,    29,   104,   121,   122,   189,   123,    33,
     -91,   103,   129,   -91,   179,   170,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   104,   181,     0,   187,   121,   122,     0,   123,
       0,     0,     0,     0,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     104,     0,     0,     0,   121,   122,     0,   123,     0,     0,
     146,   194,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   104,     0,
       0,     0,   121,   122,     0,   123,     0,     0,   198,     0,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   104,     0,     0,     0,
     121,   122,     0,   123,     0,     0,     0,     0,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,     0,     0,     0,     0,   121,   122,
       0,   123,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,     0,     0,
       0,     0,   121,   122,     0,   123,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,     0,     0,     0,     0,   121,   122,     0,   123,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,     0,     0,     0,     0,   121,   122,     0,
     123,   114,   115,   116,   117,   118,   119,   120,     0,     0,
       0,     0,   121,   122,     0,   123,   116,   117,   118,   119,
     120,     0,     0,     0,     0,   121,   122,     0,   123
  };

  const short int
  parser::yycheck_[] =
  {
       1,   134,    44,    55,    31,    32,     7,     8,    26,    26,
      10,    53,    13,    14,    56,    57,    58,    59,    60,    10,
      26,    22,    10,    10,    25,    26,    27,    23,    17,    26,
      31,    32,    34,    20,    21,    43,     0,    55,    55,    91,
      48,    49,    60,    51,    48,    49,   179,    51,    52,    55,
      50,    10,   104,    18,    55,    51,   189,    59,    55,    50,
      61,    49,    19,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
      54,    55,   124,    84,    85,    53,    87,    53,    90,   131,
      91,    53,    53,   135,   136,    56,   148,    23,   140,   126,
     142,   143,    23,   104,    22,   147,    55,    56,     4,     5,
       6,     7,     8,    49,    10,    11,    12,    55,    56,    15,
     122,   123,    23,   175,   125,   126,    48,    49,    24,    51,
     172,    55,    56,    36,    37,    38,    39,    40,    41,    42,
      43,    23,    38,    59,   186,    48,    49,   148,    51,    45,
     192,    47,   194,    49,    25,    51,   198,    54,    55,    84,
      85,   203,    58,    16,    53,    61,    22,    53,    56,    60,
      22,    25,    57,    56,   175,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    13,    57,    21,    16,    48,    49,   148,    51,    25,
      53,    61,    86,    56,   137,   125,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    16,   138,    -1,   144,    48,    49,    -1,    51,
      -1,    -1,    -1,    -1,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      16,    -1,    -1,    -1,    48,    49,    -1,    51,    -1,    -1,
      54,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    16,    -1,
      -1,    -1,    48,    49,    -1,    51,    -1,    -1,    26,    -1,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    16,    -1,    -1,    -1,
      48,    49,    -1,    51,    -1,    -1,    -1,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    49,
      -1,    51,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    -1,    -1,
      -1,    -1,    48,    49,    -1,    51,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    49,    -1,    51,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    -1,    -1,    -1,    -1,    48,    49,    -1,
      51,    37,    38,    39,    40,    41,    42,    43,    -1,    -1,
      -1,    -1,    48,    49,    -1,    51,    39,    40,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    49,    -1,    51
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,    17,    63,    64,    10,   106,     0,    18,    65,    66,
      67,    53,   106,    20,    21,    68,    69,    70,    71,    72,
     106,    53,    19,   106,   106,    53,    23,    51,    74,    67,
     106,    23,    23,    70,    49,    76,    77,    78,    79,    80,
      81,   106,    75,   106,    22,    76,    76,     4,     5,     6,
       7,     8,    11,    12,    15,    24,    38,    45,    47,    49,
      51,    58,    61,    82,    83,    84,    85,    86,    87,    88,
      94,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,    26,    55,    54,    55,    82,    82,
      49,    50,    72,    82,    82,    82,    82,    89,    90,    91,
      95,    99,    82,    75,    16,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    48,    49,    51,    55,    56,    23,    79,    79,    74,
     106,    13,    99,    72,    73,    25,    52,    59,    53,   109,
      53,    56,    26,    60,    92,    93,    54,    26,    50,    72,
      82,    82,    82,    82,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    82,    99,    99,    82,
      81,    76,    22,    82,    56,    53,   109,    82,    82,    90,
      99,    91,    56,    82,    82,    82,    60,    93,    82,    73,
      56,    54,    22,    82,    27,    72,    57,   109,    26,    82,
     109,    82,    82,    25,    56,    82,    57,    82
  };

  const unsigned char
  parser::yyr1_[] =
  {
       0,    62,    63,    64,    64,    65,    65,    66,    66,    67,
      67,    68,    68,    69,    69,    70,    70,    71,    71,    72,
      72,    73,    73,    74,    74,    75,    75,    75,    76,    76,
      77,    78,    78,    79,    79,    80,    81,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    83,    83,    84,    84,    85,    86,    87,
      88,    88,    89,    90,    90,    91,    91,    92,    92,    93,
      94,    95,    95,    96,    96,    97,    98,    99,    99,   100,
     101,   101,   101,   102,   103,   104,   105,   105,   106,   107,
     108,   109,   109
  };

  const unsigned char
  parser::yyr2_[] =
  {
       0,     2,     3,     0,     3,     0,     2,     1,     3,     2,
       4,     0,     2,     1,     3,     1,     1,     4,     4,     7,
       4,     1,     3,     0,     2,     0,     1,     3,     1,     1,
       3,     1,     3,     1,     1,     4,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     3,     3,     3,     3,     3,     3,
       1,     1,     5,     4,     7,     3,     6,     4,     4,     4,
       6,     4,     1,     1,     3,     3,     4,     1,     2,     4,
       3,     1,     3,     2,     4,     4,     3,     1,     3,     6,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     0
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const parser::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "\"invalid token\"", "INT",
  "REAL", "COMPLEX", "TRUE", "FALSE", "STRING", "ID", "QUALIFIED_ID", "IF",
  "THEN", "CASE", "THIS", "WHERE", "MODULE", "IMPORT", "AS", "INPUT",
  "EXTERNAL", "'='", "TYPE_EQ", "LET", "IN", "RIGHT_ARROW", "ELSE",
  "LOGIC_OR", "LOGIC_AND", "EQ", "NEQ", "LESS", "MORE", "LESS_EQ",
  "MORE_EQ", "PLUSPLUS", "'+'", "'-'", "'*'", "'/'", "INT_DIV", "'%'",
  "'^'", "DOTDOT", "LOGIC_NOT", "UMINUS", "'#'", "'.'", "'['", "'{'",
  "'('", "'@'", "';'", "')'", "','", "']'", "'}'", "'\\\\'", "':'", "'|'",
  "'~'", "$accept", "program", "module_decl", "imports", "import_list",
  "import", "declarations", "declaration_list", "declaration",
  "external_decl", "binding", "binding_list", "optional_type",
  "param_list", "type", "function_type", "data_type_list", "data_type",
  "array_type", "primitive_type", "expr", "let_expr", "where_expr",
  "lambda", "array_apply", "array_self_apply", "array_func",
  "array_ranges", "array_pattern_list", "array_pattern",
  "array_domain_list", "array_domain", "array_enum", "array_elem_list",
  "array_size", "func_apply", "func_composition", "expr_list", "if_expr",
  "number", "int", "real", "complex", "boolean", "id", "qualified_id",
  "inf", "optional_semicolon", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned short int
  parser::yyrline_[] =
  {
       0,    68,    68,    77,    79,    85,    87,    91,    96,   105,
     110,   118,   120,   124,   129,   138,   138,   142,   145,   151,
     156,   163,   168,   178,   180,   188,   190,   193,   202,   202,
     206,   211,   214,   223,   223,   227,   232,   237,   239,   241,
     243,   245,   247,   249,   251,   253,   255,   257,   259,   261,
     263,   265,   268,   271,   274,   277,   280,   283,   286,   289,
     292,   295,   298,   301,   304,   307,   310,   313,   316,   319,
     322,   324,   326,   334,   340,   347,   353,   360,   367,   372,
     377,   380,   385,   390,   393,   402,   405,   411,   414,   422,
     427,   436,   439,   448,   451,   456,   463,   470,   473,   481,
     486,   488,   490,   493,   496,   499,   503,   505,   508,   511,
     514,   518,   518
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
       2,     2,     2,     2,     2,    47,     2,    42,     2,     2,
      51,    54,    39,    37,    55,    38,    48,    40,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    59,    53,
       2,    22,     2,     2,    52,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    49,    58,    56,    43,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    50,    60,    57,    61,     2,     2,     2,
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
      15,    16,    17,    18,    19,    20,    21,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    41,    44,    45,    46
    };
    const unsigned int user_token_number_max_ = 294;
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
#line 1759 "parser.cpp" // lalr1.cc:1167
#line 521 "parser.y" // lalr1.cc:1168


void
stream::parsing::parser::error (const location_type& l, const std::string& m)
{
  driver.error (l, m);
}
