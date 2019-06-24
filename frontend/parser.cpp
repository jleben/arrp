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
#line 174 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::binding, yylhs.location, {(yystack_[2].value), nullptr, (yystack_[0].value)} );
  }
#line 747 "parser.cpp" // lalr1.cc:859
    break;

  case 24:
#line 179 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::binding, yylhs.location, {(yystack_[5].value), (yystack_[3].value), (yystack_[0].value)} );
  }
#line 755 "parser.cpp" // lalr1.cc:859
    break;

  case 25:
#line 185 "parser.y" // lalr1.cc:859
    {
    auto pattern = make_list(yylhs.location, { (yystack_[3].value), nullptr, (yystack_[0].value) });
    (yylhs.value) = make_list( ast::array_element_def, yylhs.location, { (yystack_[5].value), pattern });
  }
#line 764 "parser.cpp" // lalr1.cc:859
    break;

  case 26:
#line 192 "parser.y" // lalr1.cc:859
    {
    auto domain = make_list(yylhs.location, { (yystack_[0].value), (yystack_[2].value) });
    auto domains = make_list(yylhs.location, { domain });
    auto pattern = make_list(yylhs.location, { (yystack_[5].value), domains, nullptr });
    (yylhs.value) = make_list( ast::array_element_def, yylhs.location, { (yystack_[7].value), pattern });
  }
#line 775 "parser.cpp" // lalr1.cc:859
    break;

  case 27:
#line 202 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {} ); }
#line 781 "parser.cpp" // lalr1.cc:859
    break;

  case 28:
#line 205 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 787 "parser.cpp" // lalr1.cc:859
    break;

  case 29:
#line 208 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 797 "parser.cpp" // lalr1.cc:859
    break;

  case 30:
#line 217 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list(ast::id_type_decl, yylhs.location, {(yystack_[2].value), (yystack_[0].value)}); }
#line 803 "parser.cpp" // lalr1.cc:859
    break;

  case 33:
#line 226 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list(ast::function_type, yylhs.location, {(yystack_[2].value), (yystack_[0].value)}); }
#line 809 "parser.cpp" // lalr1.cc:859
    break;

  case 34:
#line 231 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 815 "parser.cpp" // lalr1.cc:859
    break;

  case 35:
#line 234 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 825 "parser.cpp" // lalr1.cc:859
    break;

  case 38:
#line 247 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list(ast::array_type, yylhs.location, {(yystack_[2].value), (yystack_[0].value)}); }
#line 831 "parser.cpp" // lalr1.cc:859
    break;

  case 54:
#line 285 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( array_concat, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} ); }
#line 837 "parser.cpp" // lalr1.cc:859
    break;

  case 55:
#line 288 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::negate), (yystack_[0].value)} ); }
#line 843 "parser.cpp" // lalr1.cc:859
    break;

  case 56:
#line 291 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_not), (yystack_[0].value)} ); }
#line 849 "parser.cpp" // lalr1.cc:859
    break;

  case 57:
#line 294 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::logic_or), (yystack_[2].value), (yystack_[0].value)} ); }
#line 855 "parser.cpp" // lalr1.cc:859
    break;

  case 58:
#line 297 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::logic_and), (yystack_[2].value), (yystack_[0].value)} ); }
#line 861 "parser.cpp" // lalr1.cc:859
    break;

  case 59:
#line 300 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_eq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 867 "parser.cpp" // lalr1.cc:859
    break;

  case 60:
#line 303 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_neq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 873 "parser.cpp" // lalr1.cc:859
    break;

  case 61:
#line 306 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_l), (yystack_[2].value), (yystack_[0].value)} ); }
#line 879 "parser.cpp" // lalr1.cc:859
    break;

  case 62:
#line 309 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_leq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 885 "parser.cpp" // lalr1.cc:859
    break;

  case 63:
#line 312 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_g), (yystack_[2].value), (yystack_[0].value)} ); }
#line 891 "parser.cpp" // lalr1.cc:859
    break;

  case 64:
#line 315 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_geq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 897 "parser.cpp" // lalr1.cc:859
    break;

  case 65:
#line 318 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::add), (yystack_[2].value), (yystack_[0].value)} ); }
#line 903 "parser.cpp" // lalr1.cc:859
    break;

  case 66:
#line 321 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::subtract), (yystack_[2].value), (yystack_[0].value)} ); }
#line 909 "parser.cpp" // lalr1.cc:859
    break;

  case 67:
#line 324 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::negate), (yystack_[0].value)} ); }
#line 915 "parser.cpp" // lalr1.cc:859
    break;

  case 68:
#line 327 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::multiply), (yystack_[2].value), (yystack_[0].value)} ); }
#line 921 "parser.cpp" // lalr1.cc:859
    break;

  case 69:
#line 330 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::divide), (yystack_[2].value), (yystack_[0].value)} ); }
#line 927 "parser.cpp" // lalr1.cc:859
    break;

  case 70:
#line 333 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::divide_integer), (yystack_[2].value), (yystack_[0].value)} ); }
#line 933 "parser.cpp" // lalr1.cc:859
    break;

  case 71:
#line 336 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::modulo), (yystack_[2].value), (yystack_[0].value)} ); }
#line 939 "parser.cpp" // lalr1.cc:859
    break;

  case 72:
#line 339 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::raise), (yystack_[2].value), (yystack_[0].value)} ); }
#line 945 "parser.cpp" // lalr1.cc:859
    break;

  case 73:
#line 342 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_and), (yystack_[2].value), (yystack_[0].value)} ); }
#line 951 "parser.cpp" // lalr1.cc:859
    break;

  case 74:
#line 345 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_or), (yystack_[2].value), (yystack_[0].value)} ); }
#line 957 "parser.cpp" // lalr1.cc:859
    break;

  case 75:
#line 348 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_xor), (yystack_[2].value), (yystack_[0].value)} ); }
#line 963 "parser.cpp" // lalr1.cc:859
    break;

  case 76:
#line 351 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_lshift), (yystack_[2].value), (yystack_[0].value)} ); }
#line 969 "parser.cpp" // lalr1.cc:859
    break;

  case 77:
#line 354 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_rshift), (yystack_[2].value), (yystack_[0].value)} ); }
#line 975 "parser.cpp" // lalr1.cc:859
    break;

  case 78:
#line 357 "parser.y" // lalr1.cc:859
    { (yylhs.value) = (yystack_[1].value); }
#line 981 "parser.cpp" // lalr1.cc:859
    break;

  case 81:
#line 364 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::binding, yylhs.location, {(yystack_[2].value), nullptr, (yystack_[0].value)} );
  }
#line 989 "parser.cpp" // lalr1.cc:859
    break;

  case 82:
#line 372 "parser.y" // lalr1.cc:859
    {
    auto bnd_list = make_list(yystack_[2].location, {(yystack_[2].value)});
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { bnd_list, (yystack_[0].value) } );
  }
#line 998 "parser.cpp" // lalr1.cc:859
    break;

  case 83:
#line 378 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { (yystack_[4].value), (yystack_[0].value) } );
  }
#line 1006 "parser.cpp" // lalr1.cc:859
    break;

  case 84:
#line 385 "parser.y" // lalr1.cc:859
    {
    auto bnd_list = make_list(yystack_[0].location, {(yystack_[0].value)});
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { bnd_list, (yystack_[2].value) } );
  }
#line 1015 "parser.cpp" // lalr1.cc:859
    break;

  case 85:
#line 391 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { (yystack_[2].value), (yystack_[5].value) } );
  }
#line 1023 "parser.cpp" // lalr1.cc:859
    break;

  case 86:
#line 398 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(ast::lambda, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} );
  }
#line 1031 "parser.cpp" // lalr1.cc:859
    break;

  case 87:
#line 405 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} ); }
#line 1037 "parser.cpp" // lalr1.cc:859
    break;

  case 88:
#line 410 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} ); }
#line 1043 "parser.cpp" // lalr1.cc:859
    break;

  case 89:
#line 415 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_def, yylhs.location, {(yystack_[4].value), (yystack_[2].value)} ); }
#line 1049 "parser.cpp" // lalr1.cc:859
    break;

  case 90:
#line 418 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_def, yylhs.location, {nullptr, (yystack_[2].value)} ); }
#line 1055 "parser.cpp" // lalr1.cc:859
    break;

  case 92:
#line 428 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1061 "parser.cpp" // lalr1.cc:859
    break;

  case 93:
#line 431 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1071 "parser.cpp" // lalr1.cc:859
    break;

  case 94:
#line 440 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[2].value), nullptr, (yystack_[0].value) } ); }
#line 1077 "parser.cpp" // lalr1.cc:859
    break;

  case 95:
#line 443 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[1].value), (yystack_[0].value), nullptr } ); }
#line 1083 "parser.cpp" // lalr1.cc:859
    break;

  case 96:
#line 446 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[3].value), (yystack_[2].value), (yystack_[0].value) } ); }
#line 1089 "parser.cpp" // lalr1.cc:859
    break;

  case 97:
#line 452 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1095 "parser.cpp" // lalr1.cc:859
    break;

  case 98:
#line 455 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[1].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1105 "parser.cpp" // lalr1.cc:859
    break;

  case 99:
#line 463 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[2].value), (yystack_[0].value) } ); }
#line 1111 "parser.cpp" // lalr1.cc:859
    break;

  case 100:
#line 468 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[1].value);
    (yylhs.value)->type = ast::array_enum;
    (yylhs.value)->location = yylhs.location;
  }
#line 1121 "parser.cpp" // lalr1.cc:859
    break;

  case 101:
#line 477 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1127 "parser.cpp" // lalr1.cc:859
    break;

  case 102:
#line 480 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1137 "parser.cpp" // lalr1.cc:859
    break;

  case 103:
#line 489 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( array_size, yylhs.location, { (yystack_[0].value), nullptr } ); }
#line 1143 "parser.cpp" // lalr1.cc:859
    break;

  case 104:
#line 492 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( array_size, yylhs.location, { (yystack_[2].value), (yystack_[0].value) } ); }
#line 1149 "parser.cpp" // lalr1.cc:859
    break;

  case 105:
#line 497 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::func_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} );
  }
#line 1157 "parser.cpp" // lalr1.cc:859
    break;

  case 106:
#line 504 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::func_compose, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} );
  }
#line 1165 "parser.cpp" // lalr1.cc:859
    break;

  case 107:
#line 511 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1171 "parser.cpp" // lalr1.cc:859
    break;

  case 108:
#line 514 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
  }
#line 1180 "parser.cpp" // lalr1.cc:859
    break;

  case 109:
#line 522 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[5].location,op_type::conditional), (yystack_[4].value), (yystack_[2].value), (yystack_[0].value)} ); }
#line 1186 "parser.cpp" // lalr1.cc:859
    break;

  case 120:
#line 555 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_node(infinity, yylhs.location); }
#line 1192 "parser.cpp" // lalr1.cc:859
    break;


#line 1196 "parser.cpp" // lalr1.cc:859
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


  const signed char parser::yypact_ninf_ = -94;

  const signed char parser::yytable_ninf_ = -102;

  const short int
  parser::yypact_[] =
  {
      24,     2,     6,    29,   -94,    10,   -94,     2,    19,    52,
     -94,   -94,    32,     2,     2,   -94,    56,   -94,   -94,   -94,
     -94,   -94,   -13,    29,     2,    33,    37,    19,   -94,   161,
      -8,   161,     2,   -94,   -94,    -8,    -8,   -94,   -94,   -94,
     -94,   -94,   -94,   -94,   161,    64,    -6,   161,   161,   161,
     161,   161,   161,     2,   -94,   438,   -94,   -94,   -94,   -94,
     -94,   -94,   -94,   -94,   -94,   -94,   -94,   -94,   -94,   -94,
     -94,   -94,   104,   -94,   -94,   161,   -94,   -94,   -16,     0,
     -94,   -94,   -94,   438,   -26,   -37,   -94,   -94,   -94,   239,
     161,     2,   111,     4,   -42,   -42,   -42,    72,   274,    63,
      75,   -94,    -4,   -10,   309,     1,    -5,   161,   161,   161,
     161,   161,   161,   161,   161,   161,   161,   161,   161,   161,
     161,   161,   161,   161,   161,   161,   161,   161,   161,   161,
     161,   161,    14,    -8,    -8,   117,   161,   118,     2,   161,
      38,   -94,    81,   161,   161,   161,   161,    82,   161,   -94,
     161,   161,    76,   -94,   -94,   161,     2,   -94,   497,   525,
     552,   578,   603,   191,   191,   191,   191,   191,   191,   106,
     106,   151,   -14,   -14,    16,    16,    16,    16,   -42,   -42,
      43,    12,   438,     2,   -94,   -94,   161,   438,   161,   -94,
     374,   -94,     2,    83,   438,   -94,    75,   -10,   -94,   -94,
     438,   438,   407,   161,   -94,   468,    81,   -94,   -94,   -94,
     343,   438,   161,   -94,   133,    98,   161,   407,   113,   161,
     468,   161,   -94,   438,   -94,   438,   438
  };

  const unsigned char
  parser::yydefact_[] =
  {
       3,     0,     0,     5,   118,     0,     1,     0,    11,     0,
       7,     4,     9,     0,     0,     2,   122,    13,    16,    15,
      17,    18,     0,     6,     0,     0,     0,   121,    12,     0,
       0,     0,    27,     8,    10,     0,     0,    14,   113,   114,
     115,   116,   117,   119,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    27,   120,    23,    79,    80,    46,    51,
      52,    49,    50,    53,    47,    48,    45,    42,   110,   111,
     112,    44,    40,    41,    43,     0,    30,    32,     0,    31,
      36,    37,    39,   107,     0,     0,    28,    21,    22,     0,
       0,     0,     0,     0,    67,    55,    56,   103,   107,     0,
     122,    92,     0,    91,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    19,   122,     0,     0,     0,   121,     0,     0,   100,
       0,     0,    95,    97,    78,     0,     0,    84,    57,    58,
      74,    75,    73,    59,    60,    61,    63,    62,    64,    76,
      77,    54,    65,    66,    68,    69,    70,    71,    72,   106,
       0,     0,    81,     0,    33,    35,     0,   108,     0,    29,
       0,    88,   121,     0,    82,   104,   122,     0,    93,    90,
     102,    94,     0,     0,    98,    86,   122,    87,   105,    38,
      25,    24,     0,    20,     0,     0,     0,    96,     0,     0,
     109,     0,    89,    99,    85,    26,    83
  };

  const short int
  parser::yypgoto_[] =
  {
     -94,   -94,   -94,   -94,   -94,   147,   -94,   -94,   148,   -88,
      22,   -94,   -45,   126,   -94,    31,   -94,   -94,    -1,   -94,
      -3,   -29,   -94,   -94,   -94,   -94,   -94,   -94,   -94,    36,
      40,   -94,    30,   -94,   -94,   -94,   -94,   -94,   -21,   -94,
     -94,   -94,   -94,   -94,   -94,   110,   -94,   -94,   -93
  };

  const short int
  parser::yydefgoto_[] =
  {
      -1,     2,     3,     8,     9,    10,    15,    16,    17,    18,
     142,    19,    20,    85,    21,    76,    77,    78,    79,    80,
      81,    83,    56,    57,    58,    59,    60,    61,    99,   100,
     101,   152,   153,    62,   102,    63,    64,    65,   197,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    28
  };

  const short int
  parser::yytable_[] =
  {
      55,    92,     4,   141,     4,     4,     6,   147,    29,    30,
      84,   133,     4,   128,   129,    89,   130,   150,    94,    95,
      96,    97,    98,   104,   137,    29,   138,   -34,   155,     4,
     103,   123,   124,   125,   126,   127,   135,   136,    13,    14,
       1,   128,   129,    31,   130,    32,     7,   134,    75,   193,
      24,    91,   156,   136,   132,    35,   148,   151,   149,    36,
      31,   157,    32,   -34,   138,   127,    87,    88,   141,   140,
      11,   128,   129,   208,   130,   136,   183,   136,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     191,   136,   182,   215,   213,   207,   136,   187,   180,   181,
     190,     5,    23,   218,   194,   195,    27,    12,    22,   200,
      90,   201,   202,    25,    26,   131,   205,   128,   129,   145,
     130,   144,   184,   185,    34,   146,   143,    22,   186,   188,
      82,   192,    86,   203,   199,    82,    82,   214,   120,   121,
     122,   123,   124,   125,   126,   127,    93,   210,   221,   211,
     222,   128,   129,    86,   130,    38,    39,    40,    41,    42,
      33,     4,    43,    44,   217,    37,    45,   224,   206,   105,
     209,   196,   204,   220,     0,    46,   198,   223,     0,     0,
     225,     0,   226,     0,   121,   122,   123,   124,   125,   126,
     127,    22,     0,     0,     0,    47,   128,   129,     0,   130,
       0,     0,    48,    49,     0,    50,    93,    51,     0,    52,
       0,     0,     0,     0,     0,     0,    53,     0,     0,    54,
       0,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,     0,     0,    82,    82,     0,   128,   129,   189,   130,
       0,     0,   139,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   106,    22,     0,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,     0,
       0,     0,     0,    82,   128,   129,     0,   130,     0,     0,
     106,     0,    22,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,     0,     0,     0,     0,     0,   128,
     129,     0,   130,     0,  -101,   106,  -101,     0,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,     0,
       0,     0,     0,     0,   128,   129,   219,   130,     0,   106,
     154,     0,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,     0,     0,     0,     0,     0,   128,   129,
     106,   130,   212,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,     0,     0,     0,     0,     0,   128,
     129,     0,   130,   106,   216,     0,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,     0,     0,     0,
       0,     0,   128,   129,   106,   130,     0,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,     0,     0,
       0,     0,     0,   128,   129,     0,   130,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,     0,     0,
       0,     0,     0,   128,   129,     0,   130,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,     0,     0,     0,
       0,     0,   128,   129,     0,   130,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,     0,     0,     0,     0,     0,
     128,   129,     0,   130,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,     0,     0,     0,     0,     0,   128,   129,     0,
     130,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,     0,     0,
       0,     0,     0,   128,   129,     0,   130,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,     0,     0,     0,     0,     0,   128,   129,
       0,   130
  };

  const short int
  parser::yycheck_[] =
  {
      29,    46,    10,    91,    10,    10,     0,   100,    21,    22,
      31,    27,    10,    55,    56,    44,    58,    27,    47,    48,
      49,    50,    51,    52,    61,    21,    63,    27,    27,    10,
      51,    45,    46,    47,    48,    49,    62,    63,    19,    20,
      16,    55,    56,    56,    58,    58,    17,    63,    56,   142,
      18,    57,    57,    63,    75,    22,    60,    67,    62,    22,
      56,   106,    58,    63,    63,    49,    35,    36,   156,    90,
      60,    55,    56,    61,    58,    63,    62,    63,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
      62,    63,   131,   196,   192,    62,    63,   136,   129,   130,
     139,     1,    60,   206,   143,   144,    60,     7,     8,   148,
      56,   150,   151,    13,    14,    21,   155,    55,    56,    66,
      58,    59,   133,   134,    24,    60,    25,    27,    21,    21,
      30,    60,    32,    67,    62,    35,    36,    64,    42,    43,
      44,    45,    46,    47,    48,    49,    46,   186,    25,   188,
      62,    55,    56,    53,    58,     4,     5,     6,     7,     8,
      23,    10,    11,    12,   203,    27,    15,    64,   156,    53,
     183,   145,   152,   212,    -1,    24,   146,   216,    -1,    -1,
     219,    -1,   221,    -1,    43,    44,    45,    46,    47,    48,
      49,    91,    -1,    -1,    -1,    44,    55,    56,    -1,    58,
      -1,    -1,    51,    52,    -1,    54,   106,    56,    -1,    58,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    68,
      -1,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    -1,    -1,   133,   134,    -1,    55,    56,   138,    58,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,   156,    -1,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    -1,
      -1,    -1,    -1,   183,    55,    56,    -1,    58,    -1,    -1,
      26,    -1,   192,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    -1,    -1,    -1,    -1,    -1,    55,
      56,    -1,    58,    -1,    60,    26,    62,    -1,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    -1,
      -1,    -1,    -1,    -1,    55,    56,    23,    58,    -1,    26,
      61,    -1,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    -1,    -1,    -1,    -1,    -1,    55,    56,
      26,    58,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    -1,    -1,    -1,    -1,    -1,    55,
      56,    -1,    58,    26,    27,    -1,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    -1,    -1,    -1,
      -1,    -1,    55,    56,    26,    58,    -1,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    -1,
      -1,    -1,    -1,    55,    56,    -1,    58,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    -1,
      -1,    -1,    -1,    55,    56,    -1,    58,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    -1,    -1,    -1,
      -1,    -1,    55,    56,    -1,    58,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    -1,    -1,    -1,    -1,    -1,
      55,    56,    -1,    58,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    -1,    -1,    -1,    -1,    -1,    55,    56,    -1,
      58,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    -1,
      -1,    -1,    -1,    55,    56,    -1,    58,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    -1,    -1,    -1,    -1,    -1,    55,    56,
      -1,    58
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,    16,    70,    71,    10,   114,     0,    17,    72,    73,
      74,    60,   114,    19,    20,    75,    76,    77,    78,    80,
      81,    83,   114,    60,    18,   114,   114,    60,   117,    21,
      22,    56,    58,    74,   114,    22,    22,    77,     4,     5,
       6,     7,     8,    11,    12,    15,    24,    44,    51,    52,
      54,    56,    58,    65,    68,    90,    91,    92,    93,    94,
      95,    96,   102,   104,   105,   106,   108,   109,   110,   111,
     112,   113,   114,   115,   116,    56,    84,    85,    86,    87,
      88,    89,   114,    90,   107,    82,   114,    84,    84,    90,
      56,    57,    81,   114,    90,    90,    90,    90,    90,    97,
      98,    99,   103,   107,    90,    82,    26,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    55,    56,
      58,    21,   107,    27,    63,    62,    63,    61,    63,    13,
     107,    78,    79,    25,    59,    66,    60,   117,    60,    62,
      27,    67,   100,   101,    61,    27,    57,    81,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
     107,   107,    90,    62,    87,    87,    21,    90,    21,   114,
      90,    62,    60,   117,    90,    90,    98,   107,    99,    62,
      90,    90,    90,    67,   101,    90,    79,    62,    61,    89,
      90,    90,    28,    78,    64,   117,    27,    90,   117,    23,
      90,    25,    62,    90,    64,    90,    90
  };

  const unsigned char
  parser::yyr1_[] =
  {
       0,    69,    70,    71,    71,    72,    72,    73,    73,    74,
      74,    75,    75,    76,    76,    77,    77,    78,    78,    79,
      79,    80,    80,    81,    81,    81,    81,    82,    82,    82,
      83,    84,    84,    85,    86,    86,    87,    87,    88,    89,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    91,    91,    92,    92,    93,    94,    95,    96,
      96,    97,    98,    98,    99,    99,    99,   100,   100,   101,
     102,   103,   103,   104,   104,   105,   106,   107,   107,   108,
     109,   109,   109,   110,   111,   112,   113,   113,   114,   115,
     116,   117,   117
  };

  const unsigned char
  parser::yyr2_[] =
  {
       0,     2,     3,     0,     3,     0,     2,     1,     3,     2,
       4,     0,     2,     1,     3,     1,     1,     1,     1,     1,
       3,     4,     4,     3,     6,     6,     8,     0,     1,     3,
       3,     1,     1,     3,     1,     3,     1,     1,     4,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     1,
       1,     3,     4,     7,     3,     6,     4,     4,     4,     6,
       4,     1,     1,     3,     3,     2,     4,     1,     2,     4,
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
  "THEN", "CASE", "THIS", "MODULE", "IMPORT", "AS", "INPUT", "EXTERNAL",
  "'='", "TYPE_EQ", "FOR", "LET", "IN", "WHERE", "RIGHT_ARROW", "ELSE",
  "LOGIC_OR", "LOGIC_AND", "BIT_OR", "BIT_XOR", "BIT_AND", "EQ", "NEQ",
  "LESS", "MORE", "LESS_EQ", "MORE_EQ", "BIT_SHIFT_LEFT",
  "BIT_SHIFT_RIGHT", "PLUSPLUS", "'+'", "'-'", "'*'", "'/'", "INT_DIV",
  "'%'", "'^'", "DOTDOT", "LOGIC_NOT", "BIT_NOT", "UMINUS", "'#'", "'.'",
  "'['", "'{'", "'('", "'@'", "';'", "')'", "']'", "','", "'}'", "'\\\\'",
  "':'", "'|'", "'~'", "$accept", "program", "module_decl", "imports",
  "import_list", "import", "declarations", "declaration_list",
  "declaration", "nested_decl", "nested_decl_list", "external_decl",
  "binding", "param_list", "id_type_decl", "type", "function_type",
  "data_type_list", "data_type", "array_type", "primitive_type", "expr",
  "let_expr", "where_expr", "lambda", "array_apply", "array_self_apply",
  "array_func", "array_ranges", "array_pattern_list", "array_pattern",
  "array_domain_list", "array_domain", "array_enum", "array_elem_list",
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
     155,   164,   167,   173,   178,   184,   191,   202,   204,   207,
     216,   221,   221,   225,   230,   233,   242,   242,   246,   251,
     256,   258,   260,   262,   264,   266,   268,   270,   272,   274,
     276,   278,   280,   282,   284,   287,   290,   293,   296,   299,
     302,   305,   308,   311,   314,   317,   320,   323,   326,   329,
     332,   335,   338,   341,   344,   347,   350,   353,   356,   359,
     361,   363,   371,   377,   384,   390,   397,   404,   409,   414,
     417,   422,   427,   430,   439,   442,   445,   451,   454,   462,
     467,   476,   479,   488,   491,   496,   503,   510,   513,   521,
     526,   528,   530,   533,   536,   539,   543,   545,   548,   551,
     554,   558,   558
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
       2,     2,     2,     2,     2,    54,     2,    48,     2,     2,
      58,    61,    45,    43,    63,    44,    55,    46,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    66,    60,
       2,    21,     2,     2,    59,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    56,    65,    62,    49,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    57,    67,    64,    68,     2,     2,     2,
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
      15,    16,    17,    18,    19,    20,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    47,    50,    51,
      52,    53
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
#line 1870 "parser.cpp" // lalr1.cc:1167
#line 561 "parser.y" // lalr1.cc:1168


void
stream::parsing::parser::error (const location_type& l, const std::string& m)
{
  driver.error (l, m);
}
