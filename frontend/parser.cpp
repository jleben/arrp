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

  case 55:
#line 284 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( array_concat, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} ); }
#line 844 "parser.cpp" // lalr1.cc:859
    break;

  case 56:
#line 287 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::negate), (yystack_[0].value)} ); }
#line 850 "parser.cpp" // lalr1.cc:859
    break;

  case 57:
#line 290 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_not), (yystack_[0].value)} ); }
#line 856 "parser.cpp" // lalr1.cc:859
    break;

  case 58:
#line 293 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::logic_or), (yystack_[2].value), (yystack_[0].value)} ); }
#line 862 "parser.cpp" // lalr1.cc:859
    break;

  case 59:
#line 296 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::logic_and), (yystack_[2].value), (yystack_[0].value)} ); }
#line 868 "parser.cpp" // lalr1.cc:859
    break;

  case 60:
#line 299 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_eq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 874 "parser.cpp" // lalr1.cc:859
    break;

  case 61:
#line 302 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_neq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 880 "parser.cpp" // lalr1.cc:859
    break;

  case 62:
#line 305 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_l), (yystack_[2].value), (yystack_[0].value)} ); }
#line 886 "parser.cpp" // lalr1.cc:859
    break;

  case 63:
#line 308 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_leq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 892 "parser.cpp" // lalr1.cc:859
    break;

  case 64:
#line 311 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_g), (yystack_[2].value), (yystack_[0].value)} ); }
#line 898 "parser.cpp" // lalr1.cc:859
    break;

  case 65:
#line 314 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_geq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 904 "parser.cpp" // lalr1.cc:859
    break;

  case 66:
#line 317 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::add), (yystack_[2].value), (yystack_[0].value)} ); }
#line 910 "parser.cpp" // lalr1.cc:859
    break;

  case 67:
#line 320 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::subtract), (yystack_[2].value), (yystack_[0].value)} ); }
#line 916 "parser.cpp" // lalr1.cc:859
    break;

  case 68:
#line 323 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::negate), (yystack_[0].value)} ); }
#line 922 "parser.cpp" // lalr1.cc:859
    break;

  case 69:
#line 326 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::multiply), (yystack_[2].value), (yystack_[0].value)} ); }
#line 928 "parser.cpp" // lalr1.cc:859
    break;

  case 70:
#line 329 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::divide), (yystack_[2].value), (yystack_[0].value)} ); }
#line 934 "parser.cpp" // lalr1.cc:859
    break;

  case 71:
#line 332 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::divide_integer), (yystack_[2].value), (yystack_[0].value)} ); }
#line 940 "parser.cpp" // lalr1.cc:859
    break;

  case 72:
#line 335 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::modulo), (yystack_[2].value), (yystack_[0].value)} ); }
#line 946 "parser.cpp" // lalr1.cc:859
    break;

  case 73:
#line 338 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::raise), (yystack_[2].value), (yystack_[0].value)} ); }
#line 952 "parser.cpp" // lalr1.cc:859
    break;

  case 74:
#line 341 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_and), (yystack_[2].value), (yystack_[0].value)} ); }
#line 958 "parser.cpp" // lalr1.cc:859
    break;

  case 75:
#line 344 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_or), (yystack_[2].value), (yystack_[0].value)} ); }
#line 964 "parser.cpp" // lalr1.cc:859
    break;

  case 76:
#line 347 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_xor), (yystack_[2].value), (yystack_[0].value)} ); }
#line 970 "parser.cpp" // lalr1.cc:859
    break;

  case 77:
#line 350 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_lshift), (yystack_[2].value), (yystack_[0].value)} ); }
#line 976 "parser.cpp" // lalr1.cc:859
    break;

  case 78:
#line 353 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_rshift), (yystack_[2].value), (yystack_[0].value)} ); }
#line 982 "parser.cpp" // lalr1.cc:859
    break;

  case 79:
#line 356 "parser.y" // lalr1.cc:859
    { (yylhs.value) = (yystack_[1].value); }
#line 988 "parser.cpp" // lalr1.cc:859
    break;

  case 82:
#line 363 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::binding, yylhs.location, {(yystack_[2].value), nullptr, (yystack_[0].value)} );
  }
#line 996 "parser.cpp" // lalr1.cc:859
    break;

  case 83:
#line 371 "parser.y" // lalr1.cc:859
    {
    auto bnd_list = make_list(yystack_[2].location, {(yystack_[2].value)});
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { bnd_list, (yystack_[0].value) } );
  }
#line 1005 "parser.cpp" // lalr1.cc:859
    break;

  case 84:
#line 377 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { (yystack_[4].value), (yystack_[0].value) } );
  }
#line 1013 "parser.cpp" // lalr1.cc:859
    break;

  case 85:
#line 384 "parser.y" // lalr1.cc:859
    {
    auto bnd_list = make_list(yystack_[0].location, {(yystack_[0].value)});
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { bnd_list, (yystack_[2].value) } );
  }
#line 1022 "parser.cpp" // lalr1.cc:859
    break;

  case 86:
#line 390 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { (yystack_[2].value), (yystack_[5].value) } );
  }
#line 1030 "parser.cpp" // lalr1.cc:859
    break;

  case 87:
#line 397 "parser.y" // lalr1.cc:859
    {
    auto params = make_list(yylhs.location, { (yystack_[3].value) });
    (yylhs.value) = make_list(ast::lambda, yylhs.location, { params, (yystack_[0].value) } );
  }
#line 1039 "parser.cpp" // lalr1.cc:859
    break;

  case 88:
#line 403 "parser.y" // lalr1.cc:859
    {
    auto params = make_list(yylhs.location, {(yystack_[5].value)});
    params->as_list()->append((yystack_[3].value)->as_list()->elements);
    (yylhs.value) = make_list(ast::lambda, yylhs.location, {params, (yystack_[0].value)} );
  }
#line 1049 "parser.cpp" // lalr1.cc:859
    break;

  case 89:
#line 412 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} ); }
#line 1055 "parser.cpp" // lalr1.cc:859
    break;

  case 90:
#line 417 "parser.y" // lalr1.cc:859
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
#line 1077 "parser.cpp" // lalr1.cc:859
    break;

  case 91:
#line 438 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1083 "parser.cpp" // lalr1.cc:859
    break;

  case 92:
#line 441 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1093 "parser.cpp" // lalr1.cc:859
    break;

  case 93:
#line 450 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value), make_node(infinity, yylhs.location)} ); }
#line 1099 "parser.cpp" // lalr1.cc:859
    break;

  case 94:
#line 453 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[2].value), (yystack_[0].value)} ); }
#line 1105 "parser.cpp" // lalr1.cc:859
    break;

  case 95:
#line 458 "parser.y" // lalr1.cc:859
    {
    auto constrained_expr = make_list( yylhs.location, { nullptr, (yystack_[0].value) });
    (yylhs.value) = make_list( yylhs.location, {constrained_expr} );
  }
#line 1114 "parser.cpp" // lalr1.cc:859
    break;

  case 96:
#line 464 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} );
  }
#line 1122 "parser.cpp" // lalr1.cc:859
    break;

  case 97:
#line 469 "parser.y" // lalr1.cc:859
    { (yylhs.value) = (yystack_[2].value); }
#line 1128 "parser.cpp" // lalr1.cc:859
    break;

  case 98:
#line 472 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[4].value);
    (yylhs.value)->as_list()->append( (yystack_[2].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1138 "parser.cpp" // lalr1.cc:859
    break;

  case 99:
#line 481 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1144 "parser.cpp" // lalr1.cc:859
    break;

  case 100:
#line 484 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1154 "parser.cpp" // lalr1.cc:859
    break;

  case 101:
#line 493 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[0].value), (yystack_[3].value) } ); }
#line 1160 "parser.cpp" // lalr1.cc:859
    break;

  case 102:
#line 498 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { nullptr, (yystack_[2].value) } ); }
#line 1166 "parser.cpp" // lalr1.cc:859
    break;

  case 103:
#line 503 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(ast::array_enum, yylhs.location, { (yystack_[3].value) });
    (yylhs.value)->as_list()->append((yystack_[1].value)->as_list()->elements);
  }
#line 1175 "parser.cpp" // lalr1.cc:859
    break;

  case 104:
#line 511 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( array_size, yylhs.location, { (yystack_[0].value), nullptr } ); }
#line 1181 "parser.cpp" // lalr1.cc:859
    break;

  case 105:
#line 514 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( array_size, yylhs.location, { (yystack_[2].value), (yystack_[0].value) } ); }
#line 1187 "parser.cpp" // lalr1.cc:859
    break;

  case 106:
#line 519 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::func_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} );
  }
#line 1195 "parser.cpp" // lalr1.cc:859
    break;

  case 107:
#line 526 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::func_compose, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} );
  }
#line 1203 "parser.cpp" // lalr1.cc:859
    break;

  case 108:
#line 533 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1209 "parser.cpp" // lalr1.cc:859
    break;

  case 109:
#line 536 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
  }
#line 1218 "parser.cpp" // lalr1.cc:859
    break;

  case 110:
#line 544 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[5].location,op_type::conditional), (yystack_[4].value), (yystack_[2].value), (yystack_[0].value)} ); }
#line 1224 "parser.cpp" // lalr1.cc:859
    break;

  case 121:
#line 577 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_node(infinity, yylhs.location); }
#line 1230 "parser.cpp" // lalr1.cc:859
    break;


#line 1234 "parser.cpp" // lalr1.cc:859
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


  const short int parser::yypact_ninf_ = -187;

  const signed char parser::yytable_ninf_ = -37;

  const short int
  parser::yypact_[] =
  {
      15,     9,    22,    61,  -187,    34,  -187,     9,    66,    67,
    -187,  -187,    65,     9,     9,     9,  -187,    76,  -187,  -187,
    -187,  -187,  -187,    11,    61,     9,    77,  -187,    -6,   112,
      66,  -187,   212,     7,   343,     9,  -187,  -187,     7,   212,
       7,  -187,  -187,  -187,  -187,  -187,  -187,  -187,   212,     0,
     212,   212,   212,   212,     9,   343,  -187,   761,  -187,  -187,
    -187,  -187,  -187,  -187,  -187,  -187,  -187,  -187,  -187,  -187,
    -187,  -187,  -187,   120,  -187,  -187,   343,  -187,  -187,    48,
     118,  -187,  -187,  -187,   791,   -20,    13,  -187,  -187,   761,
    -187,   499,     9,   117,    40,    33,    33,    33,   133,    -3,
    -187,   127,   567,     1,   212,   212,   212,   212,   212,   212,
     212,   212,   212,   212,   212,   212,   212,   212,   212,   212,
     212,   212,   212,   212,   212,   212,   343,   343,   212,    -2,
       7,     7,   343,   143,     9,   153,   212,  -187,    94,   212,
     212,     9,   151,   343,   343,   152,     9,  -187,   850,   907,
     962,  1015,  1066,   238,   238,   238,   238,   238,   238,  1108,
    1108,  1141,   110,   110,    37,    37,    37,    37,    33,    33,
       2,    17,   761,     9,  -187,  -187,   791,   142,  -187,   212,
     699,     9,   115,   761,  -187,  -187,   212,   791,    18,   212,
      94,  -187,  -187,  -187,   343,   637,  -187,  -187,   761,   212,
    -187,   154,   761,   157,   761,   122,   637,   123,  -187,   184,
     442,   212,   212,  -187,   343,   140,   212,   761,   761,   667,
    -187,   147,  -187,   761,    57,  -187,   145,  -187,  -187,   212,
       5,   343,   343,   343,   343,     9,   343,   190,     1,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   537,     9,   186,    75,    75,    75,   146,     3,   602,
     343,   104,   879,   935,   989,  1041,  1091,   315,   315,   315,
     315,   315,   315,  1125,  1125,  1157,   334,   334,   114,   114,
     114,   114,    75,    75,   212,    94,   343,   343,   189,   343,
     196,   791,   343,   343,     9,   731,   161,   791,  -187,   343,
      19,   343,   791,     4,    21,   343,   199,   791,   205,   791,
     211,   213,   821,   343,   343,   142,   343,   791,   791,   791,
     791
  };

  const unsigned char
  parser::yydefact_[] =
  {
       3,     0,     0,     5,   119,     0,     1,     0,    11,     0,
       7,     4,     9,     0,     0,     0,     2,   123,    13,    16,
      15,    17,    18,     0,     6,     0,     0,    25,    23,     0,
     122,    12,     0,     0,     0,    29,     8,    10,     0,     0,
       0,    14,   114,   115,   116,   117,   118,   120,     0,     0,
       0,     0,     0,     0,     0,     0,   121,    26,    80,    81,
      48,    53,    51,    52,    54,    49,    50,    47,    44,   111,
     112,   113,    46,    42,    43,    45,     0,    32,    34,     0,
      33,    38,    39,    41,   108,     0,     0,    30,    21,    24,
      22,     0,     0,     0,     0,    68,    56,    57,   104,     0,
      91,    93,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    19,   123,     0,
       0,     0,     0,     0,     0,    79,     0,    85,    58,    59,
      75,    76,    74,    60,    61,    62,    64,    63,    65,    77,
      78,    55,    66,    67,    69,    70,    71,    72,    73,   107,
       0,     0,    82,     0,    37,    35,   109,     0,    31,     0,
       0,   122,     0,    83,   105,    92,     0,    94,     0,     0,
     123,    89,   106,    40,     0,    95,    28,    96,    27,     0,
      20,     0,    90,   103,    87,     0,     0,   123,    99,     0,
     110,     0,     0,    86,   122,     0,     0,    84,    88,     0,
     100,   123,    97,   101,     0,   122,     0,   102,    98,     0,
       0,     0,     0,     0,     0,     0,     0,    42,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    68,    56,    57,   104,     0,     0,
       0,     0,    58,    59,    75,    76,    74,    60,    61,    62,
      64,    63,    65,    77,    78,    55,    66,    67,    69,    70,
      71,    72,    73,   107,     0,   123,     0,     0,     0,     0,
      79,    82,     0,     0,    29,     0,     0,    83,   105,     0,
       0,     0,    26,     0,     0,     0,     0,    90,   103,    87,
       0,     0,   110,     0,     0,     0,     0,    84,    88,    95,
      27
  };

  const short int
  parser::yypgoto_[] =
  {
    -187,  -187,  -187,  -187,  -187,   216,  -187,  -187,   206,    -7,
    -137,  -187,   -47,   -63,   228,    60,  -187,  -187,   -85,  -187,
      70,   200,  -187,  -187,  -187,  -187,  -187,    10,   103,  -187,
    -187,  -186,  -187,  -187,  -187,  -187,  -187,   -72,  -187,  -187,
    -187,  -187,  -187,  -187,    -1,  -187,  -187,  -133
  };

  const short int
  parser::yydefgoto_[] =
  {
      -1,     2,     3,     8,     9,    10,    16,    17,    18,   137,
     138,    20,    21,    86,    22,    77,    78,    79,    80,    81,
      82,    84,    58,    59,    60,    61,    62,    99,   100,   196,
     207,   197,   221,    63,    64,    65,    66,    85,    67,    68,
      69,    70,    71,    72,   237,    74,    75,    31
  };

  const short int
  parser::yytable_[] =
  {
       5,    19,    93,   132,   129,   182,    12,    23,   208,   190,
       4,     4,    26,    28,    29,     4,    39,     4,    33,     4,
     141,   132,     6,    19,    37,   132,   141,   132,   220,    23,
       1,    73,    83,    32,    87,    33,   134,    83,    73,    83,
     132,   132,   132,   133,   134,   174,   175,    73,    94,    73,
      73,    73,    73,   101,   170,   171,   147,   205,    92,   146,
     142,   173,    32,   262,    76,   191,   298,   320,    34,   216,
      35,   130,   188,   131,   215,   135,     4,     7,   227,   192,
     203,   318,    25,   321,    13,    14,    15,   124,   226,   125,
     126,    23,   127,   125,   126,    11,   127,    34,    88,    35,
      90,    38,    94,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,   295,   302,    73,    24,    83,
      83,   260,   126,   178,   127,    73,    40,    30,    73,    73,
     101,   -36,   128,   -36,   139,    23,    42,    43,    44,    45,
      46,   143,     4,    47,   229,   181,   120,   121,   122,   123,
     124,   303,   306,   304,   259,   177,   125,   126,   230,   127,
     260,   126,    83,   127,   200,   179,   186,   189,    73,   201,
      23,   211,   212,   263,   214,    73,   213,   231,    73,   125,
     126,   147,   127,   140,   232,   233,   216,   234,    73,   235,
     194,   236,   260,   126,   222,   127,   297,    56,   225,   228,
      73,    73,   270,   296,   309,    73,    42,    43,    44,    45,
      46,   311,     4,    47,    48,   316,   323,   310,    73,    94,
     324,   313,    57,   325,   101,   326,    41,   271,    49,    89,
      36,   314,    27,   193,   185,   268,     0,     0,    91,     0,
      95,    96,    97,    98,     0,   102,     0,    50,     0,     0,
       0,    23,     0,     0,    51,    52,     0,    53,     0,    54,
       0,    55,     0,     0,     0,     0,     0,    56,     0,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,     0,
       0,     0,     0,    73,   125,   126,     0,   127,     0,     0,
       0,     0,     0,    87,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,     0,     0,   172,     0,
       0,     0,   176,     0,     0,     0,   180,     0,     0,   183,
     184,     0,     0,   187,     0,     0,     0,    42,    43,    44,
      45,    46,     0,     4,    47,   229,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,     0,     0,     0,   230,
       0,   260,   126,     0,   127,     0,     0,   195,     0,   198,
     255,   256,   257,   258,   259,     0,   202,     0,   231,   204,
     260,   126,     0,   127,   206,   232,   233,     0,   234,   210,
     235,     0,   236,     0,     0,     0,     0,     0,    56,     0,
       0,   217,   218,     0,   219,     0,   223,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   261,
       0,   264,   265,   266,   267,     0,   269,     0,     0,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     301,     0,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,     0,   305,     0,   307,   308,   125,   126,
       0,   127,   312,     0,     0,     0,     0,     0,     0,   317,
       0,   319,   136,     0,     0,   322,     0,     0,     0,     0,
       0,     0,     0,   327,   328,   329,   330,   103,     0,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     294,     0,     0,     0,     0,   125,   126,     0,   127,     0,
       0,     0,     0,     0,     0,   103,     0,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,     0,     0,
     144,     0,     0,   125,   126,   238,   127,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,     0,     0,
       0,     0,     0,   260,   126,   299,   127,     0,     0,   145,
     238,     0,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,     0,     0,     0,     0,     0,   260,   126,
     209,   127,     0,     0,   300,   238,     0,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,     0,     0,
     224,     0,     0,   260,   126,   238,   127,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,     0,     0,
       0,     0,     0,   260,   126,     0,   127,   103,   199,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
       0,     0,     0,     0,     0,   125,   126,     0,   127,   103,
     315,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,     0,     0,     0,     0,     0,   125,   126,   103,
     127,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,     0,     0,     0,     0,     0,   125,   126,   238,
     127,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,     0,     0,     0,     0,     0,   260,   126,     0,
     127,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,     0,     0,     0,     0,     0,   260,   126,     0,
     127,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,     0,     0,     0,     0,     0,   125,   126,     0,   127,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
       0,     0,     0,     0,     0,   260,   126,     0,   127,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,     0,     0,
       0,     0,     0,   125,   126,     0,   127,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,     0,     0,     0,     0,
       0,   260,   126,     0,   127,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,     0,     0,     0,     0,     0,   125,   126,
       0,   127,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
       0,     0,     0,     0,     0,   260,   126,     0,   127,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,     0,     0,     0,     0,
       0,   125,   126,     0,   127,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,     0,     0,     0,     0,     0,   260,   126,     0,
     127,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,     0,     0,     0,
       0,     0,   125,   126,     0,   127,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,     0,     0,     0,     0,     0,   260,   126,     0,
     127,   117,   118,   119,   120,   121,   122,   123,   124,     0,
       0,     0,     0,     0,   125,   126,     0,   127,   252,   253,
     254,   255,   256,   257,   258,   259,     0,     0,     0,     0,
       0,   260,   126,     0,   127,   118,   119,   120,   121,   122,
     123,   124,     0,     0,     0,     0,     0,   125,   126,     0,
     127,   253,   254,   255,   256,   257,   258,   259,     0,     0,
       0,     0,     0,   260,   126,     0,   127
  };

  const short int
  parser::yycheck_[] =
  {
       1,     8,    49,    23,    76,   138,     7,     8,   194,   146,
      10,    10,    13,    14,    15,    10,    22,    10,    24,    10,
      23,    23,     0,    30,    25,    23,    23,    23,   214,    30,
      15,    32,    33,    22,    35,    24,    23,    38,    39,    40,
      23,    23,    23,    63,    23,   130,   131,    48,    49,    50,
      51,    52,    53,    54,   126,   127,   103,   190,    58,    58,
      63,    63,    22,    58,    57,    63,    63,    63,    57,    12,
      59,    23,   144,    25,   207,    62,    10,    16,    21,    62,
      62,    62,    17,    62,    18,    19,    20,    50,   221,    56,
      57,    92,    59,    56,    57,    61,    59,    57,    38,    59,
      40,    24,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   262,    22,   128,    61,   130,
     131,    56,    57,   134,    59,   136,    24,    61,   139,   140,
     141,    23,    22,    25,    27,   146,     4,     5,     6,     7,
       8,    24,    10,    11,    12,    61,    46,    47,    48,    49,
      50,    57,   295,    59,    50,    22,    56,    57,    26,    59,
      56,    57,   173,    59,   181,    22,    25,    25,   179,    64,
     181,    27,    25,   230,    61,   186,    64,    45,   189,    56,
      57,   238,    59,    60,    52,    53,    12,    55,   199,    57,
      58,    59,    56,    57,    64,    59,    60,    65,    61,    64,
     211,   212,    22,    27,    25,   216,     4,     5,     6,     7,
       8,    25,    10,    11,    12,    64,    27,   299,   229,   230,
      25,   303,    32,    22,   235,    22,    30,   238,    26,    39,
      24,   304,    14,   173,   141,   235,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    -1,    55,    -1,    45,    -1,    -1,
      -1,   262,    -1,    -1,    52,    53,    -1,    55,    -1,    57,
      -1,    59,    -1,    -1,    -1,    -1,    -1,    65,    -1,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    -1,
      -1,    -1,    -1,   294,    56,    57,    -1,    59,    -1,    -1,
      -1,    -1,    -1,   304,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,    -1,    -1,   128,    -1,
      -1,    -1,   132,    -1,    -1,    -1,   136,    -1,    -1,   139,
     140,    -1,    -1,   143,    -1,    -1,    -1,     4,     5,     6,
       7,     8,    -1,    10,    11,    12,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    -1,    -1,    -1,    26,
      -1,    56,    57,    -1,    59,    -1,    -1,   177,    -1,   179,
      46,    47,    48,    49,    50,    -1,   186,    -1,    45,   189,
      56,    57,    -1,    59,   194,    52,    53,    -1,    55,   199,
      57,    -1,    59,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,   211,   212,    -1,   214,    -1,   216,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   229,
      -1,   231,   232,   233,   234,    -1,   236,    -1,    -1,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     270,    -1,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    -1,   294,    -1,   296,   297,    56,    57,
      -1,    59,   302,    -1,    -1,    -1,    -1,    -1,    -1,   309,
      -1,   311,    13,    -1,    -1,   315,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   323,   324,   325,   326,    28,    -1,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      13,    -1,    -1,    -1,    -1,    56,    57,    -1,    59,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    -1,
      23,    -1,    -1,    56,    57,    28,    59,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    -1,
      -1,    -1,    -1,    56,    57,    23,    59,    -1,    -1,    62,
      28,    -1,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    -1,    -1,    -1,    -1,    -1,    56,    57,
      23,    59,    -1,    -1,    62,    28,    -1,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    -1,
      23,    -1,    -1,    56,    57,    28,    59,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    -1,
      -1,    -1,    -1,    56,    57,    -1,    59,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      -1,    -1,    -1,    -1,    -1,    56,    57,    -1,    59,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    -1,    -1,    -1,    -1,    -1,    56,    57,    28,
      59,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    -1,    -1,    -1,    -1,    -1,    56,    57,    28,
      59,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    -1,    -1,    -1,    -1,    -1,    56,    57,    -1,
      59,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    -1,    -1,    -1,    -1,    -1,    56,    57,    -1,
      59,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    -1,    -1,    -1,    -1,    -1,    56,    57,    -1,    59,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      -1,    -1,    -1,    -1,    -1,    56,    57,    -1,    59,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    -1,
      -1,    -1,    -1,    56,    57,    -1,    59,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    -1,    -1,    -1,    -1,
      -1,    56,    57,    -1,    59,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    -1,    -1,    -1,    -1,    -1,    56,    57,
      -1,    59,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      -1,    -1,    -1,    -1,    -1,    56,    57,    -1,    59,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    -1,    -1,    -1,    -1,
      -1,    56,    57,    -1,    59,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    -1,    -1,    -1,    -1,    -1,    56,    57,    -1,
      59,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    -1,    -1,    -1,
      -1,    -1,    56,    57,    -1,    59,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    -1,    -1,    -1,    -1,    -1,    56,    57,    -1,
      59,    43,    44,    45,    46,    47,    48,    49,    50,    -1,
      -1,    -1,    -1,    -1,    56,    57,    -1,    59,    43,    44,
      45,    46,    47,    48,    49,    50,    -1,    -1,    -1,    -1,
      -1,    56,    57,    -1,    59,    44,    45,    46,    47,    48,
      49,    50,    -1,    -1,    -1,    -1,    -1,    56,    57,    -1,
      59,    44,    45,    46,    47,    48,    49,    50,    -1,    -1,
      -1,    -1,    -1,    56,    57,    -1,    59
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,    15,    67,    68,    10,   110,     0,    16,    69,    70,
      71,    61,   110,    18,    19,    20,    72,    73,    74,    75,
      77,    78,    80,   110,    61,    17,   110,    80,   110,   110,
      61,   113,    22,    24,    57,    59,    71,   110,    24,    22,
      24,    74,     4,     5,     6,     7,     8,    11,    12,    26,
      45,    52,    53,    55,    57,    59,    65,    87,    88,    89,
      90,    91,    92,    99,   100,   101,   102,   104,   105,   106,
     107,   108,   109,   110,   111,   112,    57,    81,    82,    83,
      84,    85,    86,   110,    87,   103,    79,   110,    81,    87,
      81,    87,    58,    78,   110,    87,    87,    87,    87,    93,
      94,   110,    87,    28,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    56,    57,    59,    22,   103,
      23,    25,    23,    63,    23,    62,    13,    75,    76,    27,
      60,    23,    63,    24,    23,    62,    58,    78,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
     103,   103,    87,    63,    84,    84,    87,    22,   110,    22,
      87,    61,   113,    87,    87,    94,    25,    87,   103,    25,
      76,    63,    62,    86,    58,    87,    95,    97,    87,    29,
      75,    64,    87,    62,    87,   113,    87,    96,    97,    23,
      87,    27,    25,    64,    61,   113,    12,    87,    87,    87,
      97,    98,    64,    87,    23,    61,   113,    21,    64,    12,
      26,    45,    52,    53,    55,    57,    59,   110,    28,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      56,    87,    58,    78,    87,    87,    87,    87,    93,    87,
      22,   110,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    13,    76,    27,    60,    63,    23,
      62,    87,    22,    57,    59,    87,   113,    87,    87,    25,
     103,    25,    87,   103,    79,    29,    64,    87,    62,    87,
      63,    62,    87,    27,    25,    22,    22,    87,    87,    87,
      87
  };

  const unsigned char
  parser::yyr1_[] =
  {
       0,    66,    67,    68,    68,    69,    69,    70,    70,    71,
      71,    72,    72,    73,    73,    74,    74,    75,    75,    76,
      76,    77,    77,    77,    77,    77,    78,    78,    78,    79,
      79,    79,    80,    81,    81,    82,    83,    83,    84,    84,
      85,    86,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    88,    88,    89,    89,    90,    90,    91,
      92,    93,    93,    94,    94,    95,    95,    95,    95,    96,
      96,    97,    98,    99,   100,   100,   101,   102,   103,   103,
     104,   105,   105,   105,   106,   107,   108,   109,   109,   110,
     111,   112,   113,   113
  };

  const unsigned char
  parser::yyr2_[] =
  {
       0,     2,     3,     0,     3,     0,     2,     1,     3,     2,
       4,     0,     2,     1,     3,     1,     1,     1,     1,     1,
       3,     4,     4,     2,     4,     2,     3,     6,     6,     0,
       1,     3,     3,     1,     1,     3,     1,     3,     1,     1,
       4,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       1,     1,     3,     4,     7,     3,     6,     5,     7,     4,
       5,     1,     3,     1,     3,     1,     1,     4,     6,     1,
       3,     4,     3,     5,     2,     4,     4,     3,     1,     3,
       6,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     0
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const parser::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "\"invalid token\"", "INT",
  "REAL", "COMPLEX", "TRUE", "FALSE", "STRING", "ID", "QUALIFIED_ID", "IF",
  "THEN", "CASE", "MODULE", "IMPORT", "AS", "INPUT", "OUTPUT", "EXTERNAL",
  "OTHERWISE", "'='", "','", "':'", "RIGHT_ARROW", "LET", "IN", "WHERE",
  "ELSE", "LOGIC_OR", "LOGIC_AND", "BIT_OR", "BIT_XOR", "BIT_AND", "EQ",
  "NEQ", "LESS", "MORE", "LESS_EQ", "MORE_EQ", "BIT_SHIFT_LEFT",
  "BIT_SHIFT_RIGHT", "PLUSPLUS", "'+'", "'-'", "'*'", "'/'", "INT_DIV",
  "'%'", "'^'", "DOTDOT", "LOGIC_NOT", "BIT_NOT", "UMINUS", "'#'", "'.'",
  "'['", "'{'", "'('", "'@'", "';'", "')'", "']'", "'}'", "'~'", "$accept",
  "program", "module_decl", "imports", "import_list", "import",
  "declarations", "declaration_list", "declaration", "nested_decl",
  "nested_decl_list", "external_decl", "binding", "param_list",
  "id_type_decl", "type", "function_type", "data_type_list", "data_type",
  "array_type", "primitive_type", "expr", "let_expr", "where_expr",
  "func_lambda", "array_apply", "array_lambda", "array_lambda_params",
  "array_lambda_param", "array_exprs", "constrained_array_expr_list",
  "constrained_array_expr", "final_constrained_array_expr", "array_enum",
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
     273,   275,   277,   279,   281,   283,   286,   289,   292,   295,
     298,   301,   304,   307,   310,   313,   316,   319,   322,   325,
     328,   331,   334,   337,   340,   343,   346,   349,   352,   355,
     358,   360,   362,   370,   376,   383,   389,   396,   402,   411,
     416,   437,   440,   449,   452,   457,   463,   468,   471,   480,
     483,   492,   497,   502,   510,   513,   518,   525,   532,   535,
     543,   548,   550,   552,   555,   558,   561,   565,   567,   570,
     573,   576,   580,   580
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
      59,    62,    46,    44,    23,    45,    56,    47,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    24,    61,
       2,    22,     2,     2,    60,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    57,     2,    63,    50,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    58,     2,    64,    65,     2,     2,     2,
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
      15,    16,    17,    18,    19,    20,    21,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    48,    51,    52,    53,
      54
    };
    const unsigned int user_token_number_max_ = 300;
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
#line 2051 "parser.cpp" // lalr1.cc:1167
#line 583 "parser.y" // lalr1.cc:1168


void
stream::parsing::parser::error (const location_type& l, const std::string& m)
{
  driver.error (l, m);
}
