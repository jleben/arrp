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
    auto params = make_list(yylhs.location, { (yystack_[3].value) });
    (yylhs.value) = make_list(ast::lambda, yylhs.location, { params, (yystack_[0].value) } );
  }
#line 1039 "parser.cpp" // lalr1.cc:859
    break;

  case 89:
#line 405 "parser.y" // lalr1.cc:859
    {
    auto params = make_list(yylhs.location, {(yystack_[5].value)});
    params->as_list()->append((yystack_[3].value)->as_list()->elements);
    (yylhs.value) = make_list(ast::lambda, yylhs.location, {params, (yystack_[0].value)} );
  }
#line 1049 "parser.cpp" // lalr1.cc:859
    break;

  case 90:
#line 414 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} ); }
#line 1055 "parser.cpp" // lalr1.cc:859
    break;

  case 91:
#line 419 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} ); }
#line 1061 "parser.cpp" // lalr1.cc:859
    break;

  case 92:
#line 424 "parser.y" // lalr1.cc:859
    {
    auto ranges = make_list(yystack_[3].location, {});
    auto indexes = make_list(yystack_[3].location, {});

    for (auto & param : (yystack_[3].value)->as_list()->elements)
    {
      indexes->as_list()->append(param->as_list()->elements[0]);
      ranges->as_list()->append(param->as_list()->elements[1]);
    }

    auto piece = make_list(yystack_[0].location, { nullptr, (yystack_[0].value) });
    auto pieces = make_list(yystack_[0].location, { piece });
    auto pattern = make_list(yylhs.location, { indexes, pieces });
    auto patterns = make_list(yylhs.location, { pattern });

    (yylhs.value) = make_list( ast::array_def, yylhs.location, {ranges, patterns} );
  }
#line 1083 "parser.cpp" // lalr1.cc:859
    break;

  case 93:
#line 445 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1089 "parser.cpp" // lalr1.cc:859
    break;

  case 94:
#line 448 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1099 "parser.cpp" // lalr1.cc:859
    break;

  case 95:
#line 457 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value), make_node(infinity, yylhs.location)} ); }
#line 1105 "parser.cpp" // lalr1.cc:859
    break;

  case 96:
#line 460 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[2].value), (yystack_[0].value)} ); }
#line 1111 "parser.cpp" // lalr1.cc:859
    break;

  case 97:
#line 465 "parser.y" // lalr1.cc:859
    {
    auto constrained_expr = make_list( yylhs.location, { nullptr, (yystack_[0].value) });
    (yylhs.value) = make_list( yylhs.location, {constrained_expr} );
  }
#line 1120 "parser.cpp" // lalr1.cc:859
    break;

  case 98:
#line 471 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} );
  }
#line 1128 "parser.cpp" // lalr1.cc:859
    break;

  case 99:
#line 476 "parser.y" // lalr1.cc:859
    { (yylhs.value) = (yystack_[2].value); }
#line 1134 "parser.cpp" // lalr1.cc:859
    break;

  case 100:
#line 479 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[4].value);
    (yylhs.value)->as_list()->append( (yystack_[2].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1144 "parser.cpp" // lalr1.cc:859
    break;

  case 101:
#line 488 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1150 "parser.cpp" // lalr1.cc:859
    break;

  case 102:
#line 491 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1160 "parser.cpp" // lalr1.cc:859
    break;

  case 103:
#line 500 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value), (yystack_[3].value) } ); }
#line 1166 "parser.cpp" // lalr1.cc:859
    break;

  case 104:
#line 505 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { nullptr, (yystack_[2].value) } ); }
#line 1172 "parser.cpp" // lalr1.cc:859
    break;

  case 105:
#line 510 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(ast::array_enum, yylhs.location, { (yystack_[3].value) });
    (yylhs.value)->as_list()->append((yystack_[1].value)->as_list()->elements);
  }
#line 1181 "parser.cpp" // lalr1.cc:859
    break;

  case 106:
#line 518 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( array_size, yylhs.location, { (yystack_[0].value), nullptr } ); }
#line 1187 "parser.cpp" // lalr1.cc:859
    break;

  case 107:
#line 521 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( array_size, yylhs.location, { (yystack_[2].value), (yystack_[0].value) } ); }
#line 1193 "parser.cpp" // lalr1.cc:859
    break;

  case 108:
#line 526 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::func_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} );
  }
#line 1201 "parser.cpp" // lalr1.cc:859
    break;

  case 109:
#line 533 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::func_compose, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} );
  }
#line 1209 "parser.cpp" // lalr1.cc:859
    break;

  case 110:
#line 540 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1215 "parser.cpp" // lalr1.cc:859
    break;

  case 111:
#line 543 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
  }
#line 1224 "parser.cpp" // lalr1.cc:859
    break;

  case 112:
#line 551 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[5].location,op_type::conditional), (yystack_[4].value), (yystack_[2].value), (yystack_[0].value)} ); }
#line 1230 "parser.cpp" // lalr1.cc:859
    break;

  case 123:
#line 584 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_node(infinity, yylhs.location); }
#line 1236 "parser.cpp" // lalr1.cc:859
    break;


#line 1240 "parser.cpp" // lalr1.cc:859
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


  const short int parser::yypact_ninf_ = -175;

  const signed char parser::yytable_ninf_ = -37;

  const short int
  parser::yypact_[] =
  {
      30,    20,    55,    54,  -175,    78,  -175,    20,   188,    85,
    -175,  -175,   137,    20,    20,    20,  -175,   103,  -175,  -175,
    -175,  -175,  -175,    77,    54,    20,   139,  -175,    -6,   142,
     188,  -175,   217,     8,   351,    20,  -175,  -175,     8,   217,
       8,  -175,  -175,  -175,  -175,  -175,  -175,  -175,   217,   112,
       0,   217,   217,   217,   217,    20,   351,  -175,   771,  -175,
    -175,  -175,  -175,  -175,  -175,  -175,  -175,  -175,  -175,  -175,
    -175,  -175,  -175,  -175,  -175,   149,  -175,  -175,   351,  -175,
    -175,    73,   105,  -175,  -175,  -175,   801,   -21,   -15,  -175,
    -175,   771,  -175,   508,   351,    20,   150,    81,    33,    33,
      33,   102,   -20,  -175,   161,   577,     5,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   351,
     351,   217,    -4,     8,     8,   351,   165,    20,   166,   217,
      -2,  -175,   129,   217,   217,    20,   168,   351,   351,   170,
      20,  -175,   860,   917,   972,  1025,  1076,   242,   242,   242,
     242,   242,   242,    38,    38,  1134,   342,   342,    12,    12,
      12,    12,    33,    33,     3,    11,   771,    20,  -175,  -175,
     801,   146,  -175,   217,   709,  -175,    20,   132,   771,  -175,
    -175,   217,   801,    16,   217,   129,  -175,  -175,  -175,   351,
     647,  -175,  -175,   771,   217,  -175,   173,   771,   184,   771,
     148,   647,   152,  -175,   199,   450,   217,   217,  -175,   351,
     153,   217,   771,   771,   677,  -175,   155,  -175,   771,    14,
    -175,   154,  -175,  -175,   217,     6,   351,   351,   351,   351,
      20,   351,   203,     5,   351,   351,   351,   351,   351,   351,
     351,   351,   351,   351,   351,   351,   351,   351,   351,   351,
     351,   351,   351,   351,   351,   351,   547,    20,   202,    88,
      88,    88,   123,     4,   612,   351,   111,   889,   945,   999,
    1051,  1101,   326,   326,   326,   326,   326,   326,  1118,  1118,
    1150,   379,   379,   117,   117,   117,   117,    88,    88,   217,
     129,   351,   351,   211,   351,   212,   801,   351,   351,    20,
     741,   175,   801,  -175,   351,    17,   351,   801,     9,    18,
     351,   213,   801,   219,   801,   223,   224,   831,   351,   351,
     146,   351,   801,   801,   801,   801
  };

  const unsigned char
  parser::yydefact_[] =
  {
       3,     0,     0,     5,   121,     0,     1,     0,    11,     0,
       7,     4,     9,     0,     0,     0,     2,   125,    13,    16,
      15,    17,    18,     0,     6,     0,     0,    25,    23,     0,
     124,    12,     0,     0,     0,    29,     8,    10,     0,     0,
       0,    14,   116,   117,   118,   119,   120,   122,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   123,    26,    81,
      82,    48,    53,    54,    51,    52,    55,    49,    50,    47,
      44,   113,   114,   115,    46,    42,    43,    45,     0,    32,
      34,     0,    33,    38,    39,    41,   110,     0,     0,    30,
      21,    24,    22,     0,     0,     0,     0,     0,    69,    57,
      58,   106,     0,    93,    95,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    19,   125,     0,     0,     0,     0,     0,     0,    80,
       0,    86,    59,    60,    76,    77,    75,    61,    62,    63,
      65,    64,    66,    78,    79,    56,    67,    68,    70,    71,
      72,    73,    74,   109,     0,     0,    83,     0,    37,    35,
     111,     0,    31,     0,     0,    91,   124,     0,    84,   107,
      94,     0,    96,     0,     0,   125,    90,   108,    40,     0,
      97,    28,    98,    27,     0,    20,     0,    92,   105,    88,
       0,     0,   125,   101,     0,   112,     0,     0,    87,   124,
       0,     0,    85,    89,     0,   102,   125,    99,   103,     0,
     124,     0,   104,   100,     0,     0,     0,     0,     0,     0,
       0,     0,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    69,
      57,    58,   106,     0,     0,     0,     0,    59,    60,    76,
      77,    75,    61,    62,    63,    65,    64,    66,    78,    79,
      56,    67,    68,    70,    71,    72,    73,    74,   109,     0,
     125,     0,     0,     0,     0,    80,    83,     0,     0,    29,
       0,     0,    84,   107,     0,     0,     0,    26,     0,     0,
       0,     0,    92,   105,    88,     0,     0,   112,     0,     0,
       0,     0,    85,    89,    97,    27
  };

  const short int
  parser::yypgoto_[] =
  {
    -175,  -175,  -175,  -175,  -175,   225,  -175,  -175,   218,    -7,
    -139,  -175,   -48,   -59,   237,    63,  -175,  -175,   -57,  -175,
      76,   204,  -175,  -175,  -175,  -175,  -175,  -175,    19,   109,
    -175,  -175,  -174,  -175,  -175,  -175,  -175,  -175,   -73,  -175,
    -175,  -175,  -175,  -175,  -175,    -1,  -175,  -175,  -134
  };

  const short int
  parser::yydefgoto_[] =
  {
      -1,     2,     3,     8,     9,    10,    16,    17,    18,   141,
     142,    20,    21,    88,    22,    79,    80,    81,    82,    83,
      84,    86,    59,    60,    61,    62,    63,    64,   102,   103,
     201,   212,   202,   226,    65,    66,    67,    68,    87,    69,
      70,    71,    72,    73,    74,   242,    76,    77,    31
  };

  const short int
  parser::yytable_[] =
  {
       5,    19,    96,   135,   145,   132,    12,    23,   187,   137,
       4,   195,    26,    28,    29,     4,     4,    39,     4,    33,
     135,   140,   135,    19,    37,   213,   221,   135,   145,    23,
       4,    75,    85,   135,    89,   135,   232,    85,    75,    85,
     135,   135,   137,   136,   146,   225,     1,    75,   138,    97,
      75,    75,    75,    75,   104,     6,   174,   175,   151,    95,
     177,   210,   185,   127,   150,   267,    78,   196,   303,   128,
     129,     7,   130,   325,   197,   193,   178,   179,   220,   208,
     323,   326,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   231,   130,    23,   128,   129,   133,   130,   134,
      32,    90,    33,    92,    32,    97,    75,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,   300,   -36,
      75,   -36,    85,    85,   307,    34,   182,    35,    75,    34,
      11,    35,    75,    75,   104,   265,   129,    24,   130,    23,
      42,    43,    44,    45,    46,    25,     4,    47,   234,   128,
     129,    49,   130,   144,    38,    30,   311,    40,   264,   308,
      94,   309,   131,   235,   265,   129,    85,   130,   143,   205,
     265,   129,    75,   130,   302,    23,   147,   268,   181,   183,
      75,   186,   236,    75,   191,   151,   194,   206,     4,   237,
     238,   216,   239,    75,   240,   199,   241,    13,    14,    15,
     217,   221,    57,   218,   219,    75,    75,   230,   227,   233,
      75,    42,    43,    44,    45,    46,   275,     4,    47,    48,
     301,   315,    49,    75,    97,   318,    58,   314,   316,   104,
     321,   328,   276,    91,    50,   329,   330,   331,    41,    36,
     319,    27,    93,   198,   190,    98,    99,   100,   101,   273,
     105,     0,     0,    51,     0,     0,    23,     0,     0,     0,
      52,    53,     0,    54,     0,    55,     0,    56,     0,     0,
       0,     0,     0,    57,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,     0,     0,     0,     0,    75,   128,
     129,     0,   130,     0,     0,     0,     0,     0,    89,     0,
       0,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,     0,     0,   176,     0,     0,     0,   180,
       0,     0,     0,   184,     0,     0,     0,   188,   189,     0,
       0,   192,     0,     0,     0,    42,    43,    44,    45,    46,
       0,     4,    47,   234,     0,     0,    49,     0,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   235,     0,
       0,     0,     0,   265,   129,   200,   130,   203,     0,   123,
     124,   125,   126,   127,     0,   207,     0,   236,   209,   128,
     129,     0,   130,   211,   237,   238,     0,   239,   215,   240,
       0,   241,     0,     0,     0,     0,     0,    57,     0,     0,
     222,   223,     0,   224,     0,   228,   260,   261,   262,   263,
     264,     0,     0,     0,     0,     0,   265,   129,   266,   130,
     269,   270,   271,   272,     0,   274,     0,     0,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   306,
       0,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,     0,   310,     0,   312,   313,   128,   129,     0,
     130,   317,     0,     0,     0,     0,     0,     0,   322,     0,
     324,   139,     0,     0,   327,     0,     0,     0,     0,     0,
       0,     0,   332,   333,   334,   335,     0,   106,     0,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     299,     0,     0,     0,     0,   128,   129,     0,   130,     0,
       0,     0,     0,     0,     0,     0,   106,     0,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,     0,
       0,   148,     0,     0,   128,   129,   243,   130,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,     0,
       0,     0,     0,     0,   265,   129,   304,   130,     0,     0,
     149,   243,     0,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,     0,     0,     0,     0,     0,   265,
     129,   214,   130,     0,     0,   305,   243,     0,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,     0,
       0,   229,     0,     0,   265,   129,   243,   130,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,     0,
       0,     0,     0,     0,   265,   129,     0,   130,   106,   204,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,     0,     0,     0,     0,     0,   128,   129,     0,   130,
     106,   320,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,     0,     0,     0,     0,     0,   128,   129,
     106,   130,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,     0,     0,     0,     0,     0,   128,   129,
     243,   130,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,     0,     0,     0,     0,     0,   265,   129,
       0,   130,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,     0,     0,     0,     0,     0,   265,   129,
       0,   130,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,     0,     0,     0,     0,     0,   128,   129,     0,
     130,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,     0,     0,     0,     0,     0,   265,   129,     0,   130,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,     0,
       0,     0,     0,     0,   128,   129,     0,   130,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,     0,     0,     0,
       0,     0,   265,   129,     0,   130,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,     0,     0,     0,     0,     0,   128,
     129,     0,   130,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,     0,     0,     0,     0,     0,   265,   129,     0,   130,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,     0,     0,     0,
       0,     0,   128,   129,     0,   130,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,     0,     0,     0,     0,     0,   265,   129,
       0,   130,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,     0,     0,
       0,     0,     0,   128,   129,     0,   130,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,     0,     0,     0,     0,     0,   265,   129,
       0,   130,   257,   258,   259,   260,   261,   262,   263,   264,
       0,     0,     0,     0,     0,   265,   129,     0,   130,   121,
     122,   123,   124,   125,   126,   127,     0,     0,     0,     0,
       0,   128,   129,     0,   130,   258,   259,   260,   261,   262,
     263,   264,     0,     0,     0,     0,     0,   265,   129,     0,
     130
  };

  const short int
  parser::yycheck_[] =
  {
       1,     8,    50,    24,    24,    78,     7,     8,   142,    24,
      10,   150,    13,    14,    15,    10,    10,    23,    10,    25,
      24,    94,    24,    30,    25,   199,    12,    24,    24,    30,
      10,    32,    33,    24,    35,    24,    22,    38,    39,    40,
      24,    24,    24,    64,    64,   219,    16,    48,    63,    50,
      51,    52,    53,    54,    55,     0,   129,   130,   106,    59,
      64,   195,    64,    51,    59,    59,    58,    64,    64,    57,
      58,    17,    60,    64,    63,   148,   133,   134,   212,    63,
      63,    63,    44,    45,    46,    47,    48,    49,    50,    51,
      57,    58,   226,    60,    95,    57,    58,    24,    60,    26,
      23,    38,    25,    40,    23,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   267,    24,
     131,    26,   133,   134,    23,    58,   137,    60,   139,    58,
      62,    60,   143,   144,   145,    57,    58,    62,    60,   150,
       4,     5,     6,     7,     8,    18,    10,    11,    12,    57,
      58,    15,    60,    61,    25,    62,   300,    25,    51,    58,
      58,    60,    23,    27,    57,    58,   177,    60,    28,   186,
      57,    58,   183,    60,    61,   186,    25,   235,    23,    23,
     191,    62,    46,   194,    26,   243,    26,    65,    10,    53,
      54,    28,    56,   204,    58,    59,    60,    19,    20,    21,
      26,    12,    66,    65,    62,   216,   217,    62,    65,    65,
     221,     4,     5,     6,     7,     8,    23,    10,    11,    12,
      28,   304,    15,   234,   235,   308,    32,    26,    26,   240,
      65,    28,   243,    39,    27,    26,    23,    23,    30,    24,
     309,    14,    48,   177,   145,    51,    52,    53,    54,   240,
      56,    -1,    -1,    46,    -1,    -1,   267,    -1,    -1,    -1,
      53,    54,    -1,    56,    -1,    58,    -1,    60,    -1,    -1,
      -1,    -1,    -1,    66,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    -1,    -1,    -1,   299,    57,
      58,    -1,    60,    -1,    -1,    -1,    -1,    -1,   309,    -1,
      -1,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    -1,    -1,   131,    -1,    -1,    -1,   135,
      -1,    -1,    -1,   139,    -1,    -1,    -1,   143,   144,    -1,
      -1,   147,    -1,    -1,    -1,     4,     5,     6,     7,     8,
      -1,    10,    11,    12,    -1,    -1,    15,    -1,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    27,    -1,
      -1,    -1,    -1,    57,    58,   181,    60,   183,    -1,    47,
      48,    49,    50,    51,    -1,   191,    -1,    46,   194,    57,
      58,    -1,    60,   199,    53,    54,    -1,    56,   204,    58,
      -1,    60,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
     216,   217,    -1,   219,    -1,   221,    47,    48,    49,    50,
      51,    -1,    -1,    -1,    -1,    -1,    57,    58,   234,    60,
     236,   237,   238,   239,    -1,   241,    -1,    -1,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   275,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,   299,    -1,   301,   302,    57,    58,    -1,
      60,   307,    -1,    -1,    -1,    -1,    -1,    -1,   314,    -1,
     316,    13,    -1,    -1,   320,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   328,   329,   330,   331,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      13,    -1,    -1,    -1,    -1,    57,    58,    -1,    60,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    24,    -1,    -1,    57,    58,    29,    60,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    57,    58,    24,    60,    -1,    -1,
      63,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,    57,
      58,    24,    60,    -1,    -1,    63,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    24,    -1,    -1,    57,    58,    29,    60,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    57,    58,    -1,    60,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    -1,    -1,    -1,    57,    58,    -1,    60,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    57,    58,
      29,    60,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    57,    58,
      29,    60,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    57,    58,
      -1,    60,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    57,    58,
      -1,    60,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    -1,    -1,    -1,    -1,    57,    58,    -1,
      60,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    -1,    -1,    -1,    57,    58,    -1,    60,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    57,    58,    -1,    60,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
      -1,    -1,    57,    58,    -1,    60,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,    57,
      58,    -1,    60,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    -1,    -1,    -1,    57,    58,    -1,    60,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
      -1,    -1,    57,    58,    -1,    60,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    57,    58,
      -1,    60,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    -1,
      -1,    -1,    -1,    57,    58,    -1,    60,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    57,    58,
      -1,    60,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    -1,    -1,    -1,    57,    58,    -1,    60,    45,
      46,    47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,
      -1,    57,    58,    -1,    60,    45,    46,    47,    48,    49,
      50,    51,    -1,    -1,    -1,    -1,    -1,    57,    58,    -1,
      60
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,    16,    68,    69,    10,   112,     0,    17,    70,    71,
      72,    62,   112,    19,    20,    21,    73,    74,    75,    76,
      78,    79,    81,   112,    62,    18,   112,    81,   112,   112,
      62,   115,    23,    25,    58,    60,    72,   112,    25,    23,
      25,    75,     4,     5,     6,     7,     8,    11,    12,    15,
      27,    46,    53,    54,    56,    58,    60,    66,    88,    89,
      90,    91,    92,    93,    94,   101,   102,   103,   104,   106,
     107,   108,   109,   110,   111,   112,   113,   114,    58,    82,
      83,    84,    85,    86,    87,   112,    88,   105,    80,   112,
      82,    88,    82,    88,    58,    59,    79,   112,    88,    88,
      88,    88,    95,    96,   112,    88,    29,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    57,    58,
      60,    23,   105,    24,    26,    24,    64,    24,    63,    13,
     105,    76,    77,    28,    61,    24,    64,    25,    24,    63,
      59,    79,    88,    88,    88,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    88,
      88,    88,    88,    88,   105,   105,    88,    64,    85,    85,
      88,    23,   112,    23,    88,    64,    62,   115,    88,    88,
      96,    26,    88,   105,    26,    77,    64,    63,    87,    59,
      88,    97,    99,    88,    30,    76,    65,    88,    63,    88,
     115,    88,    98,    99,    24,    88,    28,    26,    65,    62,
     115,    12,    88,    88,    88,    99,   100,    65,    88,    24,
      62,   115,    22,    65,    12,    27,    46,    53,    54,    56,
      58,    60,   112,    29,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    57,    88,    59,    79,    88,
      88,    88,    88,    95,    88,    23,   112,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    13,
      77,    28,    61,    64,    24,    63,    88,    23,    58,    60,
      88,   115,    88,    88,    26,   105,    26,    88,   105,    80,
      30,    65,    88,    63,    88,    64,    63,    88,    28,    26,
      23,    23,    88,    88,    88,    88
  };

  const unsigned char
  parser::yyr1_[] =
  {
       0,    67,    68,    69,    69,    70,    70,    71,    71,    72,
      72,    73,    73,    74,    74,    75,    75,    76,    76,    77,
      77,    78,    78,    78,    78,    78,    79,    79,    79,    80,
      80,    80,    81,    82,    82,    83,    84,    84,    85,    85,
      86,    87,    88,    88,    88,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    89,    89,    90,    90,    91,    91,
      92,    93,    94,    95,    95,    96,    96,    97,    97,    97,
      97,    98,    98,    99,   100,   101,   102,   102,   103,   104,
     105,   105,   106,   107,   107,   107,   108,   109,   110,   111,
     111,   112,   113,   114,   115,   115
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
       3,     1,     1,     3,     4,     7,     3,     6,     5,     7,
       4,     4,     5,     1,     3,     1,     3,     1,     1,     4,
       6,     1,     3,     4,     3,     5,     2,     4,     4,     3,
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
  "THEN", "CASE", "THIS", "MODULE", "IMPORT", "AS", "INPUT", "OUTPUT",
  "EXTERNAL", "OTHERWISE", "'='", "','", "':'", "RIGHT_ARROW", "LET", "IN",
  "WHERE", "ELSE", "LOGIC_OR", "LOGIC_AND", "BIT_OR", "BIT_XOR", "BIT_AND",
  "EQ", "NEQ", "LESS", "MORE", "LESS_EQ", "MORE_EQ", "BIT_SHIFT_LEFT",
  "BIT_SHIFT_RIGHT", "PLUSPLUS", "'+'", "'-'", "'*'", "'/'", "INT_DIV",
  "'%'", "'^'", "DOTDOT", "LOGIC_NOT", "BIT_NOT", "UMINUS", "'#'", "'.'",
  "'['", "'{'", "'('", "'@'", "';'", "')'", "']'", "'}'", "'~'", "$accept",
  "program", "module_decl", "imports", "import_list", "import",
  "declarations", "declaration_list", "declaration", "nested_decl",
  "nested_decl_list", "external_decl", "binding", "param_list",
  "id_type_decl", "type", "function_type", "data_type_list", "data_type",
  "array_type", "primitive_type", "expr", "let_expr", "where_expr",
  "func_lambda", "array_apply", "array_self_apply", "array_lambda",
  "array_lambda_params", "array_lambda_param", "array_exprs",
  "constrained_array_expr_list", "constrained_array_expr",
  "final_constrained_array_expr", "array_enum", "array_size", "func_apply",
  "func_composition", "expr_list", "if_expr", "number", "int", "real",
  "complex", "boolean", "id", "qualified_id", "inf", "optional_semicolon", YY_NULLPTR
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
     357,   360,   362,   364,   372,   378,   385,   391,   398,   404,
     413,   418,   423,   444,   447,   456,   459,   464,   470,   475,
     478,   487,   490,   499,   504,   509,   517,   520,   525,   532,
     539,   542,   550,   555,   557,   559,   562,   565,   568,   572,
     574,   577,   580,   583,   587,   587
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
       2,     2,     2,     2,     2,    56,     2,    50,     2,     2,
      60,    63,    47,    45,    24,    46,    57,    48,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    25,    62,
       2,    23,     2,     2,    61,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    58,     2,    64,    51,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    59,     2,    65,    66,     2,     2,     2,
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
      15,    16,    17,    18,    19,    20,    21,    22,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    49,    52,    53,
      54,    55
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
#line 2057 "parser.cpp" // lalr1.cc:1167
#line 590 "parser.y" // lalr1.cc:1168


void
stream::parsing::parser::error (const location_type& l, const std::string& m)
{
  driver.error (l, m);
}
