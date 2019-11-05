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
#line 57 "parser.y" // lalr1.cc:413

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
#line 73 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(program, yylhs.location, { (yystack_[2].value), (yystack_[1].value), (yystack_[0].value) });
    driver.m_ast = (yylhs.value);
  }
#line 633 "parser.cpp" // lalr1.cc:859
    break;

  case 3:
#line 81 "parser.y" // lalr1.cc:859
    { (yylhs.value) = nullptr; }
#line 639 "parser.cpp" // lalr1.cc:859
    break;

  case 4:
#line 84 "parser.y" // lalr1.cc:859
    { (yylhs.value) = (yystack_[1].value); }
#line 645 "parser.cpp" // lalr1.cc:859
    break;

  case 5:
#line 89 "parser.y" // lalr1.cc:859
    { (yylhs.value) = nullptr; }
#line 651 "parser.cpp" // lalr1.cc:859
    break;

  case 7:
#line 96 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value) } );
  }
#line 659 "parser.cpp" // lalr1.cc:859
    break;

  case 8:
#line 101 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 669 "parser.cpp" // lalr1.cc:859
    break;

  case 9:
#line 110 "parser.y" // lalr1.cc:859
    {
  (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value), nullptr } );
  }
#line 677 "parser.cpp" // lalr1.cc:859
    break;

  case 10:
#line 115 "parser.y" // lalr1.cc:859
    {
  (yylhs.value) = make_list( yylhs.location, { (yystack_[2].value), (yystack_[0].value) } );
  }
#line 685 "parser.cpp" // lalr1.cc:859
    break;

  case 11:
#line 122 "parser.y" // lalr1.cc:859
    { (yylhs.value) = nullptr; }
#line 691 "parser.cpp" // lalr1.cc:859
    break;

  case 13:
#line 129 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value) } );
  }
#line 699 "parser.cpp" // lalr1.cc:859
    break;

  case 14:
#line 134 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 709 "parser.cpp" // lalr1.cc:859
    break;

  case 19:
#line 151 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value) } );
  }
#line 717 "parser.cpp" // lalr1.cc:859
    break;

  case 20:
#line 156 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 727 "parser.cpp" // lalr1.cc:859
    break;

  case 21:
#line 165 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list(ast::input, yylhs.location, {(yystack_[2].value), (yystack_[0].value)}); }
#line 733 "parser.cpp" // lalr1.cc:859
    break;

  case 22:
#line 168 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list(ast::external, yylhs.location, {(yystack_[2].value), (yystack_[0].value)}); }
#line 739 "parser.cpp" // lalr1.cc:859
    break;

  case 23:
#line 171 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list(ast::output, yylhs.location, {(yystack_[0].value), nullptr}); }
#line 745 "parser.cpp" // lalr1.cc:859
    break;

  case 24:
#line 175 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list(ast::output_value, yylhs.location, {(yystack_[2].value), nullptr, (yystack_[0].value)}); }
#line 751 "parser.cpp" // lalr1.cc:859
    break;

  case 25:
#line 178 "parser.y" // lalr1.cc:859
    { (yylhs.value) = (yystack_[0].value); (yylhs.value)->type = ast::output_type; }
#line 757 "parser.cpp" // lalr1.cc:859
    break;

  case 26:
#line 184 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::binding, yylhs.location, {(yystack_[2].value), nullptr, (yystack_[0].value)} );
  }
#line 765 "parser.cpp" // lalr1.cc:859
    break;

  case 27:
#line 189 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::binding, yylhs.location, {(yystack_[5].value), (yystack_[3].value), (yystack_[0].value)} );
  }
#line 773 "parser.cpp" // lalr1.cc:859
    break;

  case 28:
#line 195 "parser.y" // lalr1.cc:859
    {
    auto pattern = make_list(yylhs.location, { (yystack_[3].value), (yystack_[0].value) });
    (yylhs.value) = make_list( ast::array_element_def, yylhs.location, { (yystack_[5].value), pattern });
  }
#line 782 "parser.cpp" // lalr1.cc:859
    break;

  case 29:
#line 203 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {} ); }
#line 788 "parser.cpp" // lalr1.cc:859
    break;

  case 30:
#line 206 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 794 "parser.cpp" // lalr1.cc:859
    break;

  case 31:
#line 209 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 804 "parser.cpp" // lalr1.cc:859
    break;

  case 32:
#line 218 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list(ast::id_type_decl, yylhs.location, {(yystack_[2].value), (yystack_[0].value)}); }
#line 810 "parser.cpp" // lalr1.cc:859
    break;

  case 35:
#line 227 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list(ast::function_type, yylhs.location, {(yystack_[2].value), (yystack_[0].value)}); }
#line 816 "parser.cpp" // lalr1.cc:859
    break;

  case 36:
#line 232 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 822 "parser.cpp" // lalr1.cc:859
    break;

  case 37:
#line 235 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 832 "parser.cpp" // lalr1.cc:859
    break;

  case 40:
#line 248 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list(ast::array_type, yylhs.location, {(yystack_[2].value), (yystack_[0].value)}); }
#line 838 "parser.cpp" // lalr1.cc:859
    break;

  case 56:
#line 286 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( array_concat, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} ); }
#line 844 "parser.cpp" // lalr1.cc:859
    break;

  case 57:
#line 289 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::negate), (yystack_[0].value)} ); }
#line 850 "parser.cpp" // lalr1.cc:859
    break;

  case 58:
#line 292 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_not), (yystack_[0].value)} ); }
#line 856 "parser.cpp" // lalr1.cc:859
    break;

  case 59:
#line 295 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::logic_or), (yystack_[2].value), (yystack_[0].value)} ); }
#line 862 "parser.cpp" // lalr1.cc:859
    break;

  case 60:
#line 298 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::logic_and), (yystack_[2].value), (yystack_[0].value)} ); }
#line 868 "parser.cpp" // lalr1.cc:859
    break;

  case 61:
#line 301 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_eq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 874 "parser.cpp" // lalr1.cc:859
    break;

  case 62:
#line 304 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_neq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 880 "parser.cpp" // lalr1.cc:859
    break;

  case 63:
#line 307 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_l), (yystack_[2].value), (yystack_[0].value)} ); }
#line 886 "parser.cpp" // lalr1.cc:859
    break;

  case 64:
#line 310 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_leq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 892 "parser.cpp" // lalr1.cc:859
    break;

  case 65:
#line 313 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_g), (yystack_[2].value), (yystack_[0].value)} ); }
#line 898 "parser.cpp" // lalr1.cc:859
    break;

  case 66:
#line 316 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_geq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 904 "parser.cpp" // lalr1.cc:859
    break;

  case 67:
#line 319 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::add), (yystack_[2].value), (yystack_[0].value)} ); }
#line 910 "parser.cpp" // lalr1.cc:859
    break;

  case 68:
#line 322 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::subtract), (yystack_[2].value), (yystack_[0].value)} ); }
#line 916 "parser.cpp" // lalr1.cc:859
    break;

  case 69:
#line 325 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::negate), (yystack_[0].value)} ); }
#line 922 "parser.cpp" // lalr1.cc:859
    break;

  case 70:
#line 328 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::multiply), (yystack_[2].value), (yystack_[0].value)} ); }
#line 928 "parser.cpp" // lalr1.cc:859
    break;

  case 71:
#line 331 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::divide), (yystack_[2].value), (yystack_[0].value)} ); }
#line 934 "parser.cpp" // lalr1.cc:859
    break;

  case 72:
#line 334 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::divide_integer), (yystack_[2].value), (yystack_[0].value)} ); }
#line 940 "parser.cpp" // lalr1.cc:859
    break;

  case 73:
#line 337 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::modulo), (yystack_[2].value), (yystack_[0].value)} ); }
#line 946 "parser.cpp" // lalr1.cc:859
    break;

  case 74:
#line 340 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::raise), (yystack_[2].value), (yystack_[0].value)} ); }
#line 952 "parser.cpp" // lalr1.cc:859
    break;

  case 75:
#line 343 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_and), (yystack_[2].value), (yystack_[0].value)} ); }
#line 958 "parser.cpp" // lalr1.cc:859
    break;

  case 76:
#line 346 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_or), (yystack_[2].value), (yystack_[0].value)} ); }
#line 964 "parser.cpp" // lalr1.cc:859
    break;

  case 77:
#line 349 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_xor), (yystack_[2].value), (yystack_[0].value)} ); }
#line 970 "parser.cpp" // lalr1.cc:859
    break;

  case 78:
#line 352 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_lshift), (yystack_[2].value), (yystack_[0].value)} ); }
#line 976 "parser.cpp" // lalr1.cc:859
    break;

  case 79:
#line 355 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_rshift), (yystack_[2].value), (yystack_[0].value)} ); }
#line 982 "parser.cpp" // lalr1.cc:859
    break;

  case 80:
#line 358 "parser.y" // lalr1.cc:859
    { (yylhs.value) = (yystack_[1].value); }
#line 988 "parser.cpp" // lalr1.cc:859
    break;

  case 83:
#line 365 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::binding, yylhs.location, {(yystack_[2].value), nullptr, (yystack_[0].value)} );
  }
#line 996 "parser.cpp" // lalr1.cc:859
    break;

  case 84:
#line 373 "parser.y" // lalr1.cc:859
    {
    auto bnd_list = make_list(yystack_[2].location, {(yystack_[2].value)});
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { bnd_list, (yystack_[0].value) } );
  }
#line 1005 "parser.cpp" // lalr1.cc:859
    break;

  case 85:
#line 379 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { (yystack_[4].value), (yystack_[0].value) } );
  }
#line 1013 "parser.cpp" // lalr1.cc:859
    break;

  case 86:
#line 386 "parser.y" // lalr1.cc:859
    {
    auto bnd_list = make_list(yystack_[0].location, {(yystack_[0].value)});
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { bnd_list, (yystack_[2].value) } );
  }
#line 1022 "parser.cpp" // lalr1.cc:859
    break;

  case 87:
#line 392 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { (yystack_[2].value), (yystack_[5].value) } );
  }
#line 1030 "parser.cpp" // lalr1.cc:859
    break;

  case 88:
#line 399 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(ast::lambda, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} );
  }
#line 1038 "parser.cpp" // lalr1.cc:859
    break;

  case 89:
#line 406 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} ); }
#line 1044 "parser.cpp" // lalr1.cc:859
    break;

  case 90:
#line 411 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} ); }
#line 1050 "parser.cpp" // lalr1.cc:859
    break;

  case 91:
#line 416 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_def, yylhs.location, {(yystack_[4].value), (yystack_[2].value)} ); }
#line 1056 "parser.cpp" // lalr1.cc:859
    break;

  case 92:
#line 419 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_def, yylhs.location, {nullptr, (yystack_[2].value)} ); }
#line 1062 "parser.cpp" // lalr1.cc:859
    break;

  case 94:
#line 428 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1068 "parser.cpp" // lalr1.cc:859
    break;

  case 95:
#line 431 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1078 "parser.cpp" // lalr1.cc:859
    break;

  case 96:
#line 440 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[2].value), (yystack_[0].value) } ); }
#line 1084 "parser.cpp" // lalr1.cc:859
    break;

  case 97:
#line 445 "parser.y" // lalr1.cc:859
    {
    auto constrained_expr = make_list( yylhs.location, { nullptr, (yystack_[0].value) });
    (yylhs.value) = make_list( yylhs.location, {constrained_expr} );
  }
#line 1093 "parser.cpp" // lalr1.cc:859
    break;

  case 98:
#line 451 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} );
  }
#line 1101 "parser.cpp" // lalr1.cc:859
    break;

  case 99:
#line 456 "parser.y" // lalr1.cc:859
    { (yylhs.value) = (yystack_[2].value); }
#line 1107 "parser.cpp" // lalr1.cc:859
    break;

  case 100:
#line 459 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[4].value);
    (yylhs.value)->as_list()->append( (yystack_[2].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1117 "parser.cpp" // lalr1.cc:859
    break;

  case 101:
#line 468 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1123 "parser.cpp" // lalr1.cc:859
    break;

  case 102:
#line 471 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1133 "parser.cpp" // lalr1.cc:859
    break;

  case 103:
#line 480 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value), (yystack_[3].value) } ); }
#line 1139 "parser.cpp" // lalr1.cc:859
    break;

  case 104:
#line 485 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { nullptr, (yystack_[2].value) } ); }
#line 1145 "parser.cpp" // lalr1.cc:859
    break;

  case 105:
#line 491 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[1].value);
    (yylhs.value)->type = ast::array_enum;
    (yylhs.value)->location = yylhs.location;
  }
#line 1155 "parser.cpp" // lalr1.cc:859
    break;

  case 106:
#line 500 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1161 "parser.cpp" // lalr1.cc:859
    break;

  case 107:
#line 503 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1171 "parser.cpp" // lalr1.cc:859
    break;

  case 108:
#line 512 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( array_size, yylhs.location, { (yystack_[0].value), nullptr } ); }
#line 1177 "parser.cpp" // lalr1.cc:859
    break;

  case 109:
#line 515 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( array_size, yylhs.location, { (yystack_[2].value), (yystack_[0].value) } ); }
#line 1183 "parser.cpp" // lalr1.cc:859
    break;

  case 110:
#line 520 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::func_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} );
  }
#line 1191 "parser.cpp" // lalr1.cc:859
    break;

  case 111:
#line 527 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::func_compose, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} );
  }
#line 1199 "parser.cpp" // lalr1.cc:859
    break;

  case 112:
#line 534 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1205 "parser.cpp" // lalr1.cc:859
    break;

  case 113:
#line 537 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
  }
#line 1214 "parser.cpp" // lalr1.cc:859
    break;

  case 114:
#line 545 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[5].location,op_type::conditional), (yystack_[4].value), (yystack_[2].value), (yystack_[0].value)} ); }
#line 1220 "parser.cpp" // lalr1.cc:859
    break;

  case 125:
#line 578 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_node(infinity, yylhs.location); }
#line 1226 "parser.cpp" // lalr1.cc:859
    break;


#line 1230 "parser.cpp" // lalr1.cc:859
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


  const short int parser::yypact_ninf_ = -195;

  const signed char parser::yytable_ninf_ = -107;

  const short int
  parser::yypact_[] =
  {
       7,    26,    46,    24,  -195,     4,  -195,    26,   159,    35,
    -195,  -195,    42,    26,    26,    26,  -195,    94,  -195,  -195,
    -195,  -195,  -195,    45,    24,    26,   114,  -195,    47,   121,
     159,  -195,   441,   505,    26,    10,  -195,  -195,    10,   441,
      10,  -195,  -195,  -195,  -195,  -195,  -195,  -195,   441,    51,
       0,   441,   441,   441,   441,   505,   441,    26,  -195,   769,
    -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,
    -195,  -195,  -195,  -195,  -195,  -195,   161,  -195,  -195,   799,
     -22,    77,  -195,   505,  -195,  -195,    68,   129,  -195,  -195,
    -195,  -195,   769,  -195,    41,   505,    26,   162,    80,   -40,
     -40,   -40,   165,   576,   124,   131,  -195,   -21,   134,   611,
     157,     1,   441,   441,   441,   441,   441,   441,   441,   441,
     441,   441,   441,   441,   441,   441,   441,   441,   441,   441,
     441,   441,   441,   441,   505,   505,   441,   505,   171,    26,
     183,   -19,    10,    10,   441,   -16,  -195,   148,   441,   441,
     505,   505,   147,   441,  -195,   327,  -195,   441,    26,  -195,
     888,   945,  1000,  1053,   497,   188,   188,   188,   188,   188,
     188,   118,   118,   205,   151,   151,   209,   209,   209,   209,
     -40,   -40,    -3,    81,   769,   799,   327,  -195,   441,    26,
    -195,  -195,   707,  -195,    26,   149,   769,  -195,   131,   134,
    -195,  -195,   769,   505,   645,  -195,  -195,   769,   148,  -195,
    -195,  -195,   769,  -195,   441,  -195,   199,   175,   645,   167,
    -195,   246,   195,   829,   441,  -195,   505,   198,   441,  -195,
     769,   675,  -195,   206,  -195,   769,    13,  -195,   204,  -195,
    -195,   441,     5,   505,   505,   505,   505,    26,   252,     1,
     505,   505,   505,   505,   505,   505,   505,   505,   505,   505,
     505,   505,   505,   505,   505,   505,   505,   505,   505,   505,
     505,   505,   546,    26,   272,   -29,   -29,   -29,   214,   180,
     505,   113,   917,   973,  1027,  1079,  1104,   314,   314,   314,
     314,   314,   314,  1121,  1121,  1137,  1142,  1142,   248,   248,
     248,   248,   -29,   -29,   441,   148,   505,   505,   505,   799,
     505,   505,    26,   739,   237,   799,  -195,   799,   799,    -2,
     110,   505,   279,   286,   287,   859,   505,   327,   505,   799,
     799,   799
  };

  const unsigned char
  parser::yydefact_[] =
  {
       3,     0,     0,     5,   123,     0,     1,     0,    11,     0,
       7,     4,     9,     0,     0,     0,     2,   127,    13,    16,
      15,    17,    18,     0,     6,     0,     0,    25,    23,     0,
     126,    12,     0,     0,    29,     0,     8,    10,     0,     0,
       0,    14,   118,   119,   120,   121,   122,   124,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    29,   125,    26,
      81,    82,    48,    53,    54,    51,    52,    55,    49,    50,
      47,    44,   115,   116,   117,    46,    42,    43,    45,   112,
       0,     0,    30,     0,    32,    34,     0,    33,    38,    39,
      41,    21,    24,    22,     0,     0,     0,     0,     0,    69,
      57,    58,   108,   112,     0,   127,    94,     0,    93,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    19,   127,     0,     0,
       0,   126,     0,     0,   105,     0,    80,     0,     0,    86,
      59,    60,    76,    77,    75,    61,    62,    63,    65,    64,
      66,    78,    79,    56,    67,    68,    70,    71,    72,    73,
      74,   111,     0,     0,    83,   113,     0,    31,     0,     0,
      37,    35,     0,    90,   126,     0,    84,   109,   127,     0,
      95,    92,   107,     0,    97,    96,    98,    88,   127,    89,
     110,    28,    27,    40,     0,    20,     0,     0,     0,   127,
     101,     0,     0,   114,     0,    91,   126,     0,     0,    87,
      85,     0,   102,   127,    99,   103,     0,   126,     0,   104,
     100,     0,     0,     0,     0,     0,     0,    29,    42,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    69,    57,    58,   108,     0,
       0,     0,    59,    60,    76,    77,    75,    61,    62,    63,
      65,    64,    66,    78,    79,    56,    67,    68,    70,    71,
      72,    73,    74,   111,     0,   127,     0,     0,     0,    83,
       0,     0,    29,     0,     0,    84,   109,    88,    26,     0,
       0,     0,     0,     0,     0,   114,     0,     0,     0,    85,
      97,    27
  };

  const short int
  parser::yypgoto_[] =
  {
    -195,  -195,  -195,  -195,  -195,   290,  -195,  -195,   285,    -4,
    -140,  -195,   -47,   -56,   302,   111,  -195,  -195,   100,  -195,
     135,   164,  -195,  -195,  -195,  -195,  -195,  -195,  -195,   168,
     169,   170,  -195,  -194,  -195,  -195,  -195,  -195,  -195,  -195,
      11,  -195,  -195,  -195,  -195,  -195,  -195,    -1,  -195,  -195,
     -48
  };

  const short int
  parser::yydefgoto_[] =
  {
      -1,     2,     3,     8,     9,    10,    16,    17,    18,   146,
     147,    20,    21,    81,    22,    84,    85,    86,    87,    88,
      89,    79,    60,    61,    62,    63,    64,    65,   104,   105,
     106,   211,   219,   206,   233,    66,   107,    67,    68,    69,
     199,    70,    71,    72,    73,    74,    75,   248,    77,    78,
      31
  };

  const short int
  parser::yytable_[] =
  {
       5,   110,   137,    97,    19,   137,    12,    23,   137,   220,
       4,     4,    26,    28,    29,     4,   133,   134,   208,   135,
       4,   137,   137,     1,    37,   228,    19,   271,   134,    23,
     135,    76,   232,    82,    90,   239,     4,    90,    76,    90,
     153,     7,   138,   154,    80,   189,     6,    76,   193,    98,
      76,    76,    76,    76,   144,    76,    82,   152,    96,   158,
      25,   209,   323,   273,   159,    11,   108,    83,    32,   111,
      39,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   142,   143,   141,    23,    24,   133,   134,   195,
     135,   139,    33,    32,    34,   137,   145,    35,    95,    35,
      98,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    76,   305,   139,    76,   310,    33,   187,    34,
     140,    90,    90,    76,   210,   182,   183,    76,    76,    91,
     217,    93,    76,   -36,   -36,    30,    76,    23,   137,   155,
     222,   125,   126,   127,   128,   129,   130,   131,   132,     4,
     311,   227,   312,   324,   133,   134,    38,   135,    13,    14,
      15,   139,   157,    40,   136,   238,   150,    76,    90,   148,
     215,   279,   151,    23,   186,   274,    59,   128,   129,   130,
     131,   132,   159,    92,   139,   308,   188,   133,   134,   194,
     135,   201,    94,    76,   216,    99,   100,   101,   102,   103,
     109,   133,   134,    76,   135,   149,   224,    76,   226,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   225,
      76,    98,   190,   191,   133,   134,    82,   135,   281,   126,
     127,   128,   129,   130,   131,   132,   320,   314,   228,   132,
     229,   133,   134,   234,   135,   133,   134,   237,   135,   240,
     271,   134,    23,   135,   307,   280,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   270,   306,
     184,   185,   322,    76,   271,   134,   326,   135,   192,   327,
     328,    82,   196,   197,    36,    41,    27,   202,   198,   204,
     200,   207,   319,     0,   213,   205,     0,     0,     0,     0,
       0,    42,    43,    44,    45,    46,     0,     4,    47,   241,
       0,     0,    49,     0,     0,     0,     0,     0,     0,     0,
     204,     0,   212,   242,     0,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,     0,     0,   218,     0,     0,
     271,   134,   243,   135,     0,     0,     0,     0,   223,   244,
     245,     0,   246,     0,    55,   203,    56,     0,   230,     0,
     231,     0,   235,   247,    58,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   272,     0,   275,   276,   277,
     278,     0,     0,     0,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,     0,     0,     0,     0,
       0,     0,     0,     0,   309,    42,    43,    44,    45,    46,
       0,     4,    47,    48,     0,     0,    49,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,   313,     0,
     315,   316,   317,     0,   318,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   325,    51,     0,     0,     0,
     329,   330,   331,    52,    53,     0,    54,     0,    55,     0,
      56,     0,     0,     0,     0,     0,     0,    57,    58,    42,
      43,    44,    45,    46,     0,     4,    47,   241,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   242,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,     0,     0,
     243,     0,     0,   133,   134,     0,   135,   244,   245,   304,
     246,     0,    55,     0,    56,     0,     0,     0,     0,     0,
       0,   247,    58,     0,   111,     0,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,     0,     0,     0,
       0,     0,   133,   134,   249,   135,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,     0,     0,     0,
       0,     0,   271,   134,     0,   135,     0,  -106,     0,   111,
    -106,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,     0,     0,     0,     0,     0,   133,   134,   221,
     135,     0,     0,   249,   156,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,     0,     0,     0,   236,
       0,   271,   134,   249,   135,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,     0,     0,     0,     0,
       0,   271,   134,     0,   135,   111,   214,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,     0,     0,
       0,     0,     0,   133,   134,     0,   135,   111,   321,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
       0,     0,     0,     0,     0,   133,   134,   111,   135,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
       0,     0,     0,     0,     0,   133,   134,   249,   135,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   268,   269,   270,
       0,     0,     0,     0,     0,   271,   134,     0,   135,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
       0,     0,     0,     0,     0,   133,   134,     0,   135,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   268,   269,   270,
       0,     0,     0,     0,     0,   271,   134,     0,   135,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,     0,
       0,     0,     0,     0,   133,   134,     0,   135,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,     0,     0,
       0,     0,     0,   271,   134,     0,   135,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,     0,     0,     0,     0,
       0,   133,   134,     0,   135,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,     0,     0,     0,     0,     0,   271,
     134,     0,   135,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,     0,     0,     0,     0,     0,   133,   134,     0,   135,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,     0,     0,
       0,     0,     0,   271,   134,     0,   135,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,     0,     0,     0,     0,     0,   133,
     134,     0,   135,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   268,   269,   270,
       0,     0,     0,     0,     0,   271,   134,     0,   135,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,     0,     0,     0,     0,     0,
     271,   134,     0,   135,   263,   264,   265,   266,   267,   268,
     269,   270,     0,     0,     0,     0,     0,   271,   134,     0,
     135,   264,   265,   266,   267,   268,   269,   270,   266,   267,
     268,   269,   270,   271,   134,     0,   135,     0,   271,   134,
       0,   135
  };

  const short int
  parser::yycheck_[] =
  {
       1,    57,    24,    50,     8,    24,     7,     8,    24,   203,
      10,    10,    13,    14,    15,    10,    56,    57,   158,    59,
      10,    24,    24,    16,    25,    12,    30,    56,    57,    30,
      59,    32,   226,    34,    35,    22,    10,    38,    39,    40,
      61,    17,    64,    64,    33,    64,     0,    48,    64,    50,
      51,    52,    53,    54,    13,    56,    57,   105,    58,    58,
      18,    64,    64,    58,   111,    61,    55,    57,    23,    28,
      23,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    24,    25,    83,    96,    61,    56,    57,   147,
      59,    24,    57,    23,    59,    24,    95,    62,    57,    62,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   273,    24,   136,    23,    57,   139,    59,
      63,   142,   143,   144,    63,   134,   135,   148,   149,    38,
     198,    40,   153,    24,    25,    61,   157,   158,    24,    25,
     208,    43,    44,    45,    46,    47,    48,    49,    50,    10,
      57,   219,    59,    63,    56,    57,    62,    59,    19,    20,
      21,    24,    25,    62,    23,   233,    62,   188,   189,    27,
     194,   247,    61,   194,    23,   242,    32,    46,    47,    48,
      49,    50,   249,    39,    24,    25,    23,    56,    57,    61,
      59,    64,    48,   214,    65,    51,    52,    53,    54,    55,
      56,    56,    57,   224,    59,    60,    27,   228,    61,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    64,
     241,   242,   142,   143,    56,    57,   247,    59,   249,    44,
      45,    46,    47,    48,    49,    50,   312,   305,    12,    50,
      65,    56,    57,    65,    59,    56,    57,    61,    59,    65,
      56,    57,   273,    59,    60,    23,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,    50,    27,
     136,   137,    65,   304,    56,    57,    27,    59,   144,    23,
      23,   312,   148,   149,    24,    30,    14,   153,   150,   155,
     151,   157,   311,    -1,   189,   155,    -1,    -1,    -1,    -1,
      -1,     4,     5,     6,     7,     8,    -1,    10,    11,    12,
      -1,    -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     186,    -1,   188,    26,    -1,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    -1,    -1,   203,    -1,    -1,
      56,    57,    45,    59,    -1,    -1,    -1,    -1,   214,    52,
      53,    -1,    55,    -1,    57,    58,    59,    -1,   224,    -1,
     226,    -1,   228,    66,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   241,    -1,   243,   244,   245,
     246,    -1,    -1,    -1,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   280,     4,     5,     6,     7,     8,
      -1,    10,    11,    12,    -1,    -1,    15,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,   304,    -1,
     306,   307,   308,    -1,   310,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   321,    45,    -1,    -1,    -1,
     326,   327,   328,    52,    53,    -1,    55,    -1,    57,    -1,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    66,    67,     4,
       5,     6,     7,     8,    -1,    10,    11,    12,    -1,    -1,
      15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    -1,
      45,    -1,    -1,    56,    57,    -1,    59,    52,    53,    13,
      55,    -1,    57,    -1,    59,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    67,    -1,    28,    -1,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    -1,    -1,    -1,
      -1,    -1,    56,    57,    28,    59,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    -1,    -1,    -1,
      -1,    -1,    56,    57,    -1,    59,    -1,    61,    -1,    28,
      64,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    -1,    -1,    -1,    -1,    -1,    56,    57,    24,
      59,    -1,    -1,    28,    63,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    -1,    -1,    -1,    24,
      -1,    56,    57,    28,    59,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    -1,    -1,    -1,    -1,
      -1,    56,    57,    -1,    59,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    -1,
      -1,    -1,    -1,    56,    57,    -1,    59,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      -1,    -1,    -1,    -1,    -1,    56,    57,    28,    59,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      -1,    -1,    -1,    -1,    -1,    56,    57,    28,    59,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      -1,    -1,    -1,    -1,    -1,    56,    57,    -1,    59,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      -1,    -1,    -1,    -1,    -1,    56,    57,    -1,    59,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      -1,    -1,    -1,    -1,    -1,    56,    57,    -1,    59,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    -1,
      -1,    -1,    -1,    -1,    56,    57,    -1,    59,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    -1,
      -1,    -1,    -1,    56,    57,    -1,    59,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    -1,    -1,    -1,    -1,
      -1,    56,    57,    -1,    59,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    -1,    -1,    -1,    -1,    -1,    56,
      57,    -1,    59,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    -1,    -1,    -1,    -1,    -1,    56,    57,    -1,    59,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    -1,
      -1,    -1,    -1,    56,    57,    -1,    59,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    -1,    -1,    -1,    -1,    -1,    56,
      57,    -1,    59,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      -1,    -1,    -1,    -1,    -1,    56,    57,    -1,    59,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    -1,    -1,    -1,    -1,    -1,
      56,    57,    -1,    59,    43,    44,    45,    46,    47,    48,
      49,    50,    -1,    -1,    -1,    -1,    -1,    56,    57,    -1,
      59,    44,    45,    46,    47,    48,    49,    50,    46,    47,
      48,    49,    50,    56,    57,    -1,    59,    -1,    56,    57,
      -1,    59
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,    16,    69,    70,    10,   115,     0,    17,    71,    72,
      73,    61,   115,    19,    20,    21,    74,    75,    76,    77,
      79,    80,    82,   115,    61,    18,   115,    82,   115,   115,
      61,   118,    23,    57,    59,    62,    73,   115,    62,    23,
      62,    76,     4,     5,     6,     7,     8,    11,    12,    15,
      26,    45,    52,    53,    55,    57,    59,    66,    67,    89,
      90,    91,    92,    93,    94,    95,   103,   105,   106,   107,
     109,   110,   111,   112,   113,   114,   115,   116,   117,    89,
     108,    81,   115,    57,    83,    84,    85,    86,    87,    88,
     115,    83,    89,    83,    89,    57,    58,    80,   115,    89,
      89,    89,    89,    89,    96,    97,    98,   104,   108,    89,
      81,    28,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    56,    57,    59,    23,    24,    64,    24,
      63,   108,    24,    25,    13,   108,    77,    78,    27,    60,
      62,    61,   118,    61,    64,    25,    63,    25,    58,    80,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,   108,   108,    89,    89,    23,   115,    23,    64,
      86,    86,    89,    64,    61,   118,    89,    89,    97,   108,
      98,    64,    89,    58,    89,    99,   101,    89,    78,    64,
      63,    99,    89,    88,    29,    77,    65,   118,    89,   100,
     101,    24,   118,    89,    27,    64,    61,   118,    12,    65,
      89,    89,   101,   102,    65,    89,    24,    61,   118,    22,
      65,    12,    26,    45,    52,    53,    55,    66,   115,    28,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    56,    89,    58,    80,    89,    89,    89,    89,    81,
      23,   115,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    13,    78,    27,    60,    25,    89,
      23,    57,    59,    89,   118,    89,    89,    89,    89,   108,
      81,    29,    65,    64,    63,    89,    27,    23,    23,    89,
      89,    89
  };

  const unsigned char
  parser::yyr1_[] =
  {
       0,    68,    69,    70,    70,    71,    71,    72,    72,    73,
      73,    74,    74,    75,    75,    76,    76,    77,    77,    78,
      78,    79,    79,    79,    79,    79,    80,    80,    80,    81,
      81,    81,    82,    83,    83,    84,    85,    85,    86,    86,
      87,    88,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    90,    90,    91,    91,    92,    93,
      94,    95,    95,    96,    97,    97,    98,    99,    99,    99,
      99,   100,   100,   101,   102,   103,   104,   104,   105,   105,
     106,   107,   108,   108,   109,   110,   110,   110,   111,   112,
     113,   114,   114,   115,   116,   117,   118,   118
  };

  const unsigned char
  parser::yyr2_[] =
  {
       0,     2,     3,     0,     3,     0,     2,     1,     3,     2,
       4,     0,     2,     1,     3,     1,     1,     1,     1,     1,
       3,     4,     4,     2,     4,     2,     3,     6,     6,     0,
       1,     3,     3,     1,     1,     3,     1,     3,     1,     1,
       4,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     1,     3,     4,     7,     3,     6,     4,     4,
       4,     6,     4,     1,     1,     3,     3,     1,     1,     4,
       6,     1,     3,     4,     3,     3,     1,     3,     2,     4,
       4,     3,     1,     3,     6,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     0
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const parser::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "\"invalid token\"", "INT",
  "REAL", "COMPLEX", "TRUE", "FALSE", "STRING", "ID", "QUALIFIED_ID", "IF",
  "THEN", "CASE", "THIS", "MODULE", "IMPORT", "AS", "INPUT", "OUTPUT",
  "EXTERNAL", "OTHERWISE", "'='", "','", "RIGHT_ARROW", "LET", "IN",
  "WHERE", "ELSE", "LOGIC_OR", "LOGIC_AND", "BIT_OR", "BIT_XOR", "BIT_AND",
  "EQ", "NEQ", "LESS", "MORE", "LESS_EQ", "MORE_EQ", "BIT_SHIFT_LEFT",
  "BIT_SHIFT_RIGHT", "PLUSPLUS", "'+'", "'-'", "'*'", "'/'", "INT_DIV",
  "'%'", "'^'", "DOTDOT", "LOGIC_NOT", "BIT_NOT", "UMINUS", "'#'", "'.'",
  "'['", "'{'", "'('", "'@'", "';'", "':'", "')'", "']'", "'}'", "'\\\\'",
  "'~'", "$accept", "program", "module_decl", "imports", "import_list",
  "import", "declarations", "declaration_list", "declaration",
  "nested_decl", "nested_decl_list", "external_decl", "binding",
  "param_list", "id_type_decl", "type", "function_type", "data_type_list",
  "data_type", "array_type", "primitive_type", "expr", "let_expr",
  "where_expr", "lambda", "array_apply", "array_self_apply", "array_func",
  "array_ranges", "array_pattern_list", "array_pattern", "array_exprs",
  "constrained_array_expr_list", "constrained_array_expr",
  "final_constrained_array_expr", "array_enum", "array_elem_list",
  "array_size", "func_apply", "func_composition", "expr_list", "if_expr",
  "number", "int", "real", "complex", "boolean", "id", "qualified_id",
  "inf", "optional_semicolon", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned short int
  parser::yyrline_[] =
  {
       0,    72,    72,    81,    83,    89,    91,    95,   100,   109,
     114,   122,   124,   128,   133,   142,   142,   146,   146,   150,
     155,   164,   167,   170,   173,   177,   183,   188,   194,   203,
     205,   208,   217,   222,   222,   226,   231,   234,   243,   243,
     247,   252,   257,   259,   261,   263,   265,   267,   269,   271,
     273,   275,   277,   279,   281,   283,   285,   288,   291,   294,
     297,   300,   303,   306,   309,   312,   315,   318,   321,   324,
     327,   330,   333,   336,   339,   342,   345,   348,   351,   354,
     357,   360,   362,   364,   372,   378,   385,   391,   398,   405,
     410,   415,   418,   423,   427,   430,   439,   444,   450,   455,
     458,   467,   470,   479,   484,   490,   499,   502,   511,   514,
     519,   526,   533,   536,   544,   549,   551,   553,   556,   559,
     562,   566,   568,   571,   574,   577,   581,   581
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
       2,     2,     2,     2,     2,    55,     2,    49,     2,     2,
      59,    63,    46,    44,    24,    45,    56,    47,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    62,    61,
       2,    23,     2,     2,    60,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    57,    66,    64,    50,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    58,     2,    65,    67,     2,     2,     2,
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
      15,    16,    17,    18,    19,    20,    21,    22,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    48,    51,    52,
      53,    54
    };
    const unsigned int user_token_number_max_ = 301;
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
#line 2048 "parser.cpp" // lalr1.cc:1167
#line 584 "parser.y" // lalr1.cc:1168


void
stream::parsing::parser::error (const location_type& l, const std::string& m)
{
  driver.error (l, m);
}
