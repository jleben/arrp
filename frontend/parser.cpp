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
    auto pattern = make_list(yylhs.location, { (yystack_[3].value), nullptr, (yystack_[0].value) });
    (yylhs.value) = make_list( ast::array_element_def, yylhs.location, { (yystack_[5].value), pattern });
  }
#line 782 "parser.cpp" // lalr1.cc:859
    break;

  case 29:
#line 202 "parser.y" // lalr1.cc:859
    {
    auto domain = make_list(yylhs.location, { (yystack_[0].value), (yystack_[2].value) });
    auto domains = make_list(yylhs.location, { domain });
    auto pattern = make_list(yylhs.location, { (yystack_[5].value), domains, nullptr });
    (yylhs.value) = make_list( ast::array_element_def, yylhs.location, { (yystack_[7].value), pattern });
  }
#line 793 "parser.cpp" // lalr1.cc:859
    break;

  case 30:
#line 212 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {} ); }
#line 799 "parser.cpp" // lalr1.cc:859
    break;

  case 31:
#line 215 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 805 "parser.cpp" // lalr1.cc:859
    break;

  case 32:
#line 218 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 815 "parser.cpp" // lalr1.cc:859
    break;

  case 33:
#line 227 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list(ast::id_type_decl, yylhs.location, {(yystack_[2].value), (yystack_[0].value)}); }
#line 821 "parser.cpp" // lalr1.cc:859
    break;

  case 36:
#line 236 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list(ast::function_type, yylhs.location, {(yystack_[2].value), (yystack_[0].value)}); }
#line 827 "parser.cpp" // lalr1.cc:859
    break;

  case 37:
#line 241 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 833 "parser.cpp" // lalr1.cc:859
    break;

  case 38:
#line 244 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 843 "parser.cpp" // lalr1.cc:859
    break;

  case 41:
#line 257 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list(ast::array_type, yylhs.location, {(yystack_[2].value), (yystack_[0].value)}); }
#line 849 "parser.cpp" // lalr1.cc:859
    break;

  case 57:
#line 295 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( array_concat, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} ); }
#line 855 "parser.cpp" // lalr1.cc:859
    break;

  case 58:
#line 298 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::negate), (yystack_[0].value)} ); }
#line 861 "parser.cpp" // lalr1.cc:859
    break;

  case 59:
#line 301 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_not), (yystack_[0].value)} ); }
#line 867 "parser.cpp" // lalr1.cc:859
    break;

  case 60:
#line 304 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::logic_or), (yystack_[2].value), (yystack_[0].value)} ); }
#line 873 "parser.cpp" // lalr1.cc:859
    break;

  case 61:
#line 307 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::logic_and), (yystack_[2].value), (yystack_[0].value)} ); }
#line 879 "parser.cpp" // lalr1.cc:859
    break;

  case 62:
#line 310 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_eq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 885 "parser.cpp" // lalr1.cc:859
    break;

  case 63:
#line 313 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_neq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 891 "parser.cpp" // lalr1.cc:859
    break;

  case 64:
#line 316 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_l), (yystack_[2].value), (yystack_[0].value)} ); }
#line 897 "parser.cpp" // lalr1.cc:859
    break;

  case 65:
#line 319 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_leq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 903 "parser.cpp" // lalr1.cc:859
    break;

  case 66:
#line 322 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_g), (yystack_[2].value), (yystack_[0].value)} ); }
#line 909 "parser.cpp" // lalr1.cc:859
    break;

  case 67:
#line 325 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::compare_geq), (yystack_[2].value), (yystack_[0].value)} ); }
#line 915 "parser.cpp" // lalr1.cc:859
    break;

  case 68:
#line 328 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::add), (yystack_[2].value), (yystack_[0].value)} ); }
#line 921 "parser.cpp" // lalr1.cc:859
    break;

  case 69:
#line 331 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::subtract), (yystack_[2].value), (yystack_[0].value)} ); }
#line 927 "parser.cpp" // lalr1.cc:859
    break;

  case 70:
#line 334 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::negate), (yystack_[0].value)} ); }
#line 933 "parser.cpp" // lalr1.cc:859
    break;

  case 71:
#line 337 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::multiply), (yystack_[2].value), (yystack_[0].value)} ); }
#line 939 "parser.cpp" // lalr1.cc:859
    break;

  case 72:
#line 340 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::divide), (yystack_[2].value), (yystack_[0].value)} ); }
#line 945 "parser.cpp" // lalr1.cc:859
    break;

  case 73:
#line 343 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::divide_integer), (yystack_[2].value), (yystack_[0].value)} ); }
#line 951 "parser.cpp" // lalr1.cc:859
    break;

  case 74:
#line 346 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::modulo), (yystack_[2].value), (yystack_[0].value)} ); }
#line 957 "parser.cpp" // lalr1.cc:859
    break;

  case 75:
#line 349 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::raise), (yystack_[2].value), (yystack_[0].value)} ); }
#line 963 "parser.cpp" // lalr1.cc:859
    break;

  case 76:
#line 352 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_and), (yystack_[2].value), (yystack_[0].value)} ); }
#line 969 "parser.cpp" // lalr1.cc:859
    break;

  case 77:
#line 355 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_or), (yystack_[2].value), (yystack_[0].value)} ); }
#line 975 "parser.cpp" // lalr1.cc:859
    break;

  case 78:
#line 358 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_xor), (yystack_[2].value), (yystack_[0].value)} ); }
#line 981 "parser.cpp" // lalr1.cc:859
    break;

  case 79:
#line 361 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_lshift), (yystack_[2].value), (yystack_[0].value)} ); }
#line 987 "parser.cpp" // lalr1.cc:859
    break;

  case 80:
#line 364 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[1].location,op_type::bitwise_rshift), (yystack_[2].value), (yystack_[0].value)} ); }
#line 993 "parser.cpp" // lalr1.cc:859
    break;

  case 81:
#line 367 "parser.y" // lalr1.cc:859
    { (yylhs.value) = (yystack_[1].value); }
#line 999 "parser.cpp" // lalr1.cc:859
    break;

  case 84:
#line 374 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::binding, yylhs.location, {(yystack_[2].value), nullptr, (yystack_[0].value)} );
  }
#line 1007 "parser.cpp" // lalr1.cc:859
    break;

  case 85:
#line 382 "parser.y" // lalr1.cc:859
    {
    auto bnd_list = make_list(yystack_[2].location, {(yystack_[2].value)});
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { bnd_list, (yystack_[0].value) } );
  }
#line 1016 "parser.cpp" // lalr1.cc:859
    break;

  case 86:
#line 388 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { (yystack_[4].value), (yystack_[0].value) } );
  }
#line 1024 "parser.cpp" // lalr1.cc:859
    break;

  case 87:
#line 395 "parser.y" // lalr1.cc:859
    {
    auto bnd_list = make_list(yystack_[0].location, {(yystack_[0].value)});
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { bnd_list, (yystack_[2].value) } );
  }
#line 1033 "parser.cpp" // lalr1.cc:859
    break;

  case 88:
#line 401 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(ast::local_scope, yylhs.location, { (yystack_[2].value), (yystack_[5].value) } );
  }
#line 1041 "parser.cpp" // lalr1.cc:859
    break;

  case 89:
#line 408 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list(ast::lambda, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} );
  }
#line 1049 "parser.cpp" // lalr1.cc:859
    break;

  case 90:
#line 415 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} ); }
#line 1055 "parser.cpp" // lalr1.cc:859
    break;

  case 91:
#line 420 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} ); }
#line 1061 "parser.cpp" // lalr1.cc:859
    break;

  case 92:
#line 425 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_def, yylhs.location, {(yystack_[4].value), (yystack_[2].value)} ); }
#line 1067 "parser.cpp" // lalr1.cc:859
    break;

  case 93:
#line 428 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( ast::array_def, yylhs.location, {nullptr, (yystack_[2].value)} ); }
#line 1073 "parser.cpp" // lalr1.cc:859
    break;

  case 95:
#line 438 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1079 "parser.cpp" // lalr1.cc:859
    break;

  case 96:
#line 441 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1089 "parser.cpp" // lalr1.cc:859
    break;

  case 97:
#line 450 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[2].value), nullptr, (yystack_[0].value) } ); }
#line 1095 "parser.cpp" // lalr1.cc:859
    break;

  case 98:
#line 453 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[1].value), (yystack_[0].value), nullptr } ); }
#line 1101 "parser.cpp" // lalr1.cc:859
    break;

  case 99:
#line 456 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[3].value), (yystack_[2].value), (yystack_[0].value) } ); }
#line 1107 "parser.cpp" // lalr1.cc:859
    break;

  case 100:
#line 462 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1113 "parser.cpp" // lalr1.cc:859
    break;

  case 101:
#line 465 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[1].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1123 "parser.cpp" // lalr1.cc:859
    break;

  case 102:
#line 473 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, { (yystack_[2].value), (yystack_[0].value) } ); }
#line 1129 "parser.cpp" // lalr1.cc:859
    break;

  case 103:
#line 478 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[1].value);
    (yylhs.value)->type = ast::array_enum;
    (yylhs.value)->location = yylhs.location;
  }
#line 1139 "parser.cpp" // lalr1.cc:859
    break;

  case 104:
#line 487 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1145 "parser.cpp" // lalr1.cc:859
    break;

  case 105:
#line 490 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 1155 "parser.cpp" // lalr1.cc:859
    break;

  case 106:
#line 499 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( array_size, yylhs.location, { (yystack_[0].value), nullptr } ); }
#line 1161 "parser.cpp" // lalr1.cc:859
    break;

  case 107:
#line 502 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( array_size, yylhs.location, { (yystack_[2].value), (yystack_[0].value) } ); }
#line 1167 "parser.cpp" // lalr1.cc:859
    break;

  case 108:
#line 507 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::func_apply, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} );
  }
#line 1175 "parser.cpp" // lalr1.cc:859
    break;

  case 109:
#line 514 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = make_list( ast::func_compose, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} );
  }
#line 1183 "parser.cpp" // lalr1.cc:859
    break;

  case 110:
#line 521 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( yylhs.location, {(yystack_[0].value)} ); }
#line 1189 "parser.cpp" // lalr1.cc:859
    break;

  case 111:
#line 524 "parser.y" // lalr1.cc:859
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->as_list()->append( (yystack_[0].value) );
  }
#line 1198 "parser.cpp" // lalr1.cc:859
    break;

  case 112:
#line 532 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_list( primitive, yylhs.location, {make_const(yystack_[5].location,op_type::conditional), (yystack_[4].value), (yystack_[2].value), (yystack_[0].value)} ); }
#line 1204 "parser.cpp" // lalr1.cc:859
    break;

  case 123:
#line 565 "parser.y" // lalr1.cc:859
    { (yylhs.value) = make_node(infinity, yylhs.location); }
#line 1210 "parser.cpp" // lalr1.cc:859
    break;


#line 1214 "parser.cpp" // lalr1.cc:859
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


  const signed char parser::yypact_ninf_ = -104;

  const signed char parser::yytable_ninf_ = -105;

  const short int
  parser::yypact_[] =
  {
       9,     6,    38,    -8,  -104,   -13,  -104,     6,    21,    19,
    -104,  -104,    40,     6,     6,     6,  -104,    29,  -104,  -104,
    -104,  -104,  -104,     0,    -8,     6,    43,  -104,    -3,    46,
      21,  -104,    67,    -2,    67,     6,  -104,  -104,    -2,    67,
      -2,  -104,  -104,  -104,  -104,  -104,  -104,  -104,    67,    42,
      -7,    67,    67,    67,    67,    67,    67,     6,  -104,   411,
    -104,  -104,  -104,  -104,  -104,  -104,  -104,  -104,  -104,  -104,
    -104,  -104,  -104,  -104,  -104,  -104,    89,  -104,  -104,    67,
    -104,  -104,   -17,   -11,  -104,  -104,  -104,   411,    50,    27,
    -104,  -104,   411,  -104,   187,    67,     6,    95,     4,    71,
      71,    71,     8,   247,    56,    64,  -104,   -28,   -18,   282,
     -10,    -6,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    67,    54,    -2,    -2,
     107,    67,   119,     6,    67,    68,  -104,    90,    67,    67,
      67,    67,    91,    67,  -104,    67,    67,    84,  -104,  -104,
      67,     6,  -104,   470,   498,   525,   551,   576,   206,   206,
     206,   206,   206,   206,   100,   100,   592,    37,    37,   -29,
     -29,   -29,   -29,    71,    71,    76,    45,   411,     6,  -104,
    -104,    67,   411,    67,  -104,   347,  -104,     6,    88,   411,
    -104,    64,   -18,  -104,  -104,   411,   411,   380,    67,  -104,
     441,    90,  -104,  -104,  -104,   316,   411,    67,  -104,   129,
     120,    67,   380,    93,    67,   441,    67,  -104,   411,  -104,
     411,   411
  };

  const unsigned char
  parser::yydefact_[] =
  {
       3,     0,     0,     5,   121,     0,     1,     0,    11,     0,
       7,     4,     9,     0,     0,     0,     2,   125,    13,    16,
      15,    17,    18,     0,     6,     0,     0,    25,    23,     0,
     124,    12,     0,     0,     0,    30,     8,    10,     0,     0,
       0,    14,   116,   117,   118,   119,   120,   122,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    30,   123,    26,
      82,    83,    49,    54,    55,    52,    53,    56,    50,    51,
      48,    45,   113,   114,   115,    47,    43,    44,    46,     0,
      33,    35,     0,    34,    39,    40,    42,   110,     0,     0,
      31,    21,    24,    22,     0,     0,     0,     0,     0,    70,
      58,    59,   106,   110,     0,   125,    95,     0,    94,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    19,   125,     0,     0,
       0,   124,     0,     0,   103,     0,     0,    98,   100,    81,
       0,     0,    87,    60,    61,    77,    78,    76,    62,    63,
      64,    66,    65,    67,    79,    80,    57,    68,    69,    71,
      72,    73,    74,    75,   109,     0,     0,    84,     0,    36,
      38,     0,   111,     0,    32,     0,    91,   124,     0,    85,
     107,   125,     0,    96,    93,   105,    97,     0,     0,   101,
      89,   125,    90,   108,    41,    28,    27,     0,    20,     0,
       0,     0,    99,     0,     0,   112,     0,    92,   102,    88,
      29,    86
  };

  const short int
  parser::yypgoto_[] =
  {
    -104,  -104,  -104,  -104,  -104,   160,  -104,  -104,   156,   -91,
      28,  -104,   -49,   131,   177,     5,  -104,  -104,    -4,  -104,
       7,    49,  -104,  -104,  -104,  -104,  -104,  -104,  -104,    44,
      41,  -104,    51,  -104,  -104,  -104,  -104,  -104,   -19,  -104,
    -104,  -104,  -104,  -104,  -104,    -1,  -104,  -104,  -103
  };

  const short int
  parser::yydefgoto_[] =
  {
      -1,     2,     3,     8,     9,    10,    16,    17,    18,    19,
     147,    20,    21,    89,    22,    80,    81,    82,    83,    84,
      85,    87,    60,    61,    62,    63,    64,    65,   104,   105,
     106,   157,   158,    66,   107,    67,    68,    69,   202,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    31
  };

  const short int
  parser::yytable_[] =
  {
       5,    97,   152,     4,     4,   146,    12,    23,     4,     7,
     155,   138,    26,    28,    29,    88,     4,   -37,   160,    39,
      33,   132,    32,    33,    37,     1,    32,   133,   134,    23,
     135,     4,    86,   153,    90,   154,   108,    86,     6,    86,
      13,    14,    15,    91,   198,    93,   141,   139,    11,    98,
     156,    96,   161,   -37,   143,    79,    90,    34,    25,    35,
     137,    34,   162,    35,   133,   134,    38,   135,   149,    40,
     146,    42,    43,    44,    45,    46,   145,     4,    47,    48,
      24,    59,    49,   128,   129,   130,   131,   132,    92,   142,
      30,   143,    50,   133,   134,    23,   135,    94,   220,    95,
      99,   100,   101,   102,   103,   109,   218,   213,   223,   141,
      98,   136,    51,   140,   141,   185,   186,   188,   141,    52,
      53,   148,    54,   150,    55,   151,    56,   133,   134,   191,
     135,   196,   141,    57,   189,   190,    58,    86,    86,   212,
     141,   193,   194,   125,   126,   127,   128,   129,   130,   131,
     132,   197,   208,   219,   204,   226,   133,   134,   229,   135,
      23,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   227,    36,   187,    41,    86,   110,   211,
     192,    27,   203,   195,   201,   214,    23,   199,   200,     0,
     144,     0,   205,     0,   206,   207,     0,     0,   209,   210,
       0,     0,     0,     0,   111,     0,     0,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,     0,     0,
     215,     0,   216,   133,   134,     0,   135,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   222,     0,     0,
       0,     0,   133,   134,     0,   135,   225,     0,     0,     0,
     228,     0,     0,   230,   111,   231,     0,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,     0,     0,
       0,     0,     0,   133,   134,     0,   135,     0,  -104,   111,
    -104,     0,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,     0,     0,     0,     0,     0,   133,   134,
     224,   135,     0,   111,   159,     0,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,     0,     0,     0,
       0,     0,   133,   134,   111,   135,   217,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,     0,     0,
       0,     0,     0,   133,   134,     0,   135,   111,   221,     0,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,     0,     0,     0,     0,     0,   133,   134,   111,   135,
       0,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,     0,     0,     0,     0,     0,   133,   134,     0,
     135,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,     0,     0,     0,     0,     0,   133,   134,     0,
     135,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,     0,     0,     0,     0,     0,   133,   134,     0,   135,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,     0,
       0,     0,     0,     0,   133,   134,     0,   135,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,     0,     0,     0,     0,
       0,   133,   134,     0,   135,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,     0,     0,     0,     0,     0,   133,   134,     0,
     135,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,     0,     0,     0,
       0,     0,   133,   134,     0,   135,   126,   127,   128,   129,
     130,   131,   132,     0,     0,     0,     0,     0,   133,   134,
       0,   135
  };

  const short int
  parser::yycheck_[] =
  {
       1,    50,   105,    10,    10,    96,     7,     8,    10,    17,
      28,    28,    13,    14,    15,    34,    10,    28,    28,    22,
      23,    50,    22,    23,    25,    16,    22,    56,    57,    30,
      59,    10,    33,    61,    35,    63,    55,    38,     0,    40,
      19,    20,    21,    38,   147,    40,    64,    64,    61,    50,
      68,    58,    58,    64,    64,    57,    57,    57,    18,    59,
      79,    57,   111,    59,    56,    57,    23,    59,    60,    23,
     161,     4,     5,     6,     7,     8,    95,    10,    11,    12,
      61,    32,    15,    46,    47,    48,    49,    50,    39,    62,
      61,    64,    25,    56,    57,    96,    59,    48,   201,    57,
      51,    52,    53,    54,    55,    56,   197,    62,   211,    64,
     111,    22,    45,    63,    64,   134,   135,    63,    64,    52,
      53,    26,    55,    67,    57,    61,    59,    56,    57,    22,
      59,    63,    64,    66,   138,   139,    69,   138,   139,    63,
      64,    22,   143,    43,    44,    45,    46,    47,    48,    49,
      50,    61,    68,    65,    63,    26,    56,    57,    65,    59,
     161,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,    63,    24,   136,    30,   188,    57,   161,
     141,    14,   151,   144,   150,   188,   197,   148,   149,    -1,
      13,    -1,   153,    -1,   155,   156,    -1,    -1,   157,   160,
      -1,    -1,    -1,    -1,    27,    -1,    -1,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    -1,
     191,    -1,   193,    56,    57,    -1,    59,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,   208,    -1,    -1,
      -1,    -1,    56,    57,    -1,    59,   217,    -1,    -1,    -1,
     221,    -1,    -1,   224,    27,   226,    -1,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    -1,
      -1,    -1,    -1,    56,    57,    -1,    59,    -1,    61,    27,
      63,    -1,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    -1,    -1,    -1,    -1,    -1,    56,    57,
      24,    59,    -1,    27,    62,    -1,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    -1,    -1,    -1,
      -1,    -1,    56,    57,    27,    59,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    -1,
      -1,    -1,    -1,    56,    57,    -1,    59,    27,    28,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    -1,    -1,    -1,    -1,    -1,    56,    57,    27,    59,
      -1,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    -1,    -1,    -1,    -1,    -1,    56,    57,    -1,
      59,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    -1,    -1,    -1,    -1,    -1,    56,    57,    -1,
      59,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    -1,    -1,    -1,    -1,    -1,    56,    57,    -1,    59,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    -1,
      -1,    -1,    -1,    -1,    56,    57,    -1,    59,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    -1,    -1,    -1,    -1,
      -1,    56,    57,    -1,    59,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    -1,    -1,    -1,    -1,    -1,    56,    57,    -1,
      59,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    -1,    -1,    -1,
      -1,    -1,    56,    57,    -1,    59,    44,    45,    46,    47,
      48,    49,    50,    -1,    -1,    -1,    -1,    -1,    56,    57,
      -1,    59
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,    16,    71,    72,    10,   115,     0,    17,    73,    74,
      75,    61,   115,    19,    20,    21,    76,    77,    78,    79,
      81,    82,    84,   115,    61,    18,   115,    84,   115,   115,
      61,   118,    22,    23,    57,    59,    75,   115,    23,    22,
      23,    78,     4,     5,     6,     7,     8,    11,    12,    15,
      25,    45,    52,    53,    55,    57,    59,    66,    69,    91,
      92,    93,    94,    95,    96,    97,   103,   105,   106,   107,
     109,   110,   111,   112,   113,   114,   115,   116,   117,    57,
      85,    86,    87,    88,    89,    90,   115,    91,   108,    83,
     115,    85,    91,    85,    91,    57,    58,    82,   115,    91,
      91,    91,    91,    91,    98,    99,   100,   104,   108,    91,
      83,    27,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    56,    57,    59,    22,   108,    28,    64,
      63,    64,    62,    64,    13,   108,    79,    80,    26,    60,
      67,    61,   118,    61,    63,    28,    68,   101,   102,    62,
      28,    58,    82,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,   108,   108,    91,    63,    88,
      88,    22,    91,    22,   115,    91,    63,    61,   118,    91,
      91,    99,   108,   100,    63,    91,    91,    91,    68,   102,
      91,    80,    63,    62,    90,    91,    91,    29,    79,    65,
     118,    28,    91,   118,    24,    91,    26,    63,    91,    65,
      91,    91
  };

  const unsigned char
  parser::yyr1_[] =
  {
       0,    70,    71,    72,    72,    73,    73,    74,    74,    75,
      75,    76,    76,    77,    77,    78,    78,    79,    79,    80,
      80,    81,    81,    81,    81,    81,    82,    82,    82,    82,
      83,    83,    83,    84,    85,    85,    86,    87,    87,    88,
      88,    89,    90,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    92,    92,    93,    93,    94,
      95,    96,    97,    97,    98,    99,    99,   100,   100,   100,
     101,   101,   102,   103,   104,   104,   105,   105,   106,   107,
     108,   108,   109,   110,   110,   110,   111,   112,   113,   114,
     114,   115,   116,   117,   118,   118
  };

  const unsigned char
  parser::yyr2_[] =
  {
       0,     2,     3,     0,     3,     0,     2,     1,     3,     2,
       4,     0,     2,     1,     3,     1,     1,     1,     1,     1,
       3,     4,     4,     2,     4,     2,     3,     6,     6,     8,
       0,     1,     3,     3,     1,     1,     3,     1,     3,     1,
       1,     4,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     1,     3,     4,     7,     3,     6,     4,
       4,     4,     6,     4,     1,     1,     3,     3,     2,     4,
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
  "THEN", "CASE", "THIS", "MODULE", "IMPORT", "AS", "INPUT", "OUTPUT",
  "EXTERNAL", "'='", "TYPE_EQ", "FOR", "LET", "IN", "WHERE", "RIGHT_ARROW",
  "ELSE", "LOGIC_OR", "LOGIC_AND", "BIT_OR", "BIT_XOR", "BIT_AND", "EQ",
  "NEQ", "LESS", "MORE", "LESS_EQ", "MORE_EQ", "BIT_SHIFT_LEFT",
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
     155,   164,   167,   170,   173,   177,   183,   188,   194,   201,
     212,   214,   217,   226,   231,   231,   235,   240,   243,   252,
     252,   256,   261,   266,   268,   270,   272,   274,   276,   278,
     280,   282,   284,   286,   288,   290,   292,   294,   297,   300,
     303,   306,   309,   312,   315,   318,   321,   324,   327,   330,
     333,   336,   339,   342,   345,   348,   351,   354,   357,   360,
     363,   366,   369,   371,   373,   381,   387,   394,   400,   407,
     414,   419,   424,   427,   432,   437,   440,   449,   452,   455,
     461,   464,   472,   477,   486,   489,   498,   501,   506,   513,
     520,   523,   531,   536,   538,   540,   543,   546,   549,   553,
     555,   558,   561,   564,   568,   568
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
      59,    62,    46,    44,    64,    45,    56,    47,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    67,    61,
       2,    22,     2,     2,    60,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    57,    66,    63,    50,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    58,    68,    65,    69,     2,     2,     2,
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
      36,    37,    38,    39,    40,    41,    42,    43,    48,    51,
      52,    53,    54
    };
    const unsigned int user_token_number_max_ = 302;
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
#line 1889 "parser.cpp" // lalr1.cc:1167
#line 571 "parser.y" // lalr1.cc:1168


void
stream::parsing::parser::error (const location_type& l, const std::string& m)
{
  driver.error (l, m);
}
