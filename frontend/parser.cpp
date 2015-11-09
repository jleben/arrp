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

#line 61 "parser.cpp" // lalr1.cc:408


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
#line 147 "parser.cpp" // lalr1.cc:474

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
#line 58 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[1].value);
    (yylhs.value)->type = ast::program;
    (yylhs.value)->location = yylhs.location;
    driver.m_ast = (yylhs.value);
  }
#line 601 "parser.cpp" // lalr1.cc:847
    break;

  case 3:
#line 66 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = new ast::list_node(ast::program, location_type());
    driver.m_ast = (yylhs.value);
  }
#line 610 "parser.cpp" // lalr1.cc:847
    break;

  case 4:
#line 74 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = new ast::list_node( ast::statement_list, yylhs.location, { (yystack_[0].value) } );
  }
#line 618 "parser.cpp" // lalr1.cc:847
    break;

  case 5:
#line 79 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value).as<ast::list_node>()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 628 "parser.cpp" // lalr1.cc:847
    break;

  case 6:
#line 88 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = new ast::list_node( ast::statement, yylhs.location, {(yystack_[5].value), (yystack_[3].value), (yystack_[0].value)} );
  }
#line 636 "parser.cpp" // lalr1.cc:847
    break;

  case 7:
#line 93 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = new ast::list_node( ast::statement, yylhs.location, {(yystack_[2].value), nullptr, (yystack_[0].value)} );
  }
#line 644 "parser.cpp" // lalr1.cc:847
    break;

  case 8:
#line 100 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::list_node( ast::id_list, yylhs.location ); }
#line 650 "parser.cpp" // lalr1.cc:847
    break;

  case 9:
#line 103 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::list_node( ast::id_list, yylhs.location, {(yystack_[0].value)} ); }
#line 656 "parser.cpp" // lalr1.cc:847
    break;

  case 10:
#line 106 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value).as<ast::list_node>()->append( (yystack_[0].value) );
    (yylhs.value)->location = yylhs.location;
  }
#line 666 "parser.cpp" // lalr1.cc:847
    break;

  case 11:
#line 115 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = new ast::list_node( ast::expression_block, yylhs.location, {nullptr, (yystack_[0].value)} );
  }
#line 674 "parser.cpp" // lalr1.cc:847
    break;

  case 12:
#line 120 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = new ast::list_node( ast::expression_block, yylhs.location, {nullptr, (yystack_[2].value)} );
  }
#line 682 "parser.cpp" // lalr1.cc:847
    break;

  case 13:
#line 125 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = new ast::list_node( ast::expression_block, yylhs.location, {(yystack_[4].value), (yystack_[2].value)} );
  }
#line 690 "parser.cpp" // lalr1.cc:847
    break;

  case 15:
#line 134 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value).as<ast::list_node>()->append( (yystack_[0].value).as<ast::list_node>()->elements );
    (yylhs.value)->location = yylhs.location;
  }
#line 700 "parser.cpp" // lalr1.cc:847
    break;

  case 16:
#line 143 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = new ast::list_node( ast::statement_list, yylhs.location, {(yystack_[0].value)} );
  }
#line 708 "parser.cpp" // lalr1.cc:847
    break;

  case 17:
#line 148 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value)->location = yylhs.location;
  }
#line 717 "parser.cpp" // lalr1.cc:847
    break;

  case 18:
#line 154 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::node( ast::kwd_let, yylhs.location ); }
#line 723 "parser.cpp" // lalr1.cc:847
    break;

  case 21:
#line 163 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::list_node( ast::oppose, yylhs.location, {(yystack_[0].value)} ); }
#line 729 "parser.cpp" // lalr1.cc:847
    break;

  case 22:
#line 166 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::binary_op_expression( (yystack_[2].value), ast::logic_or, (yystack_[0].value), yylhs.location ); }
#line 735 "parser.cpp" // lalr1.cc:847
    break;

  case 23:
#line 169 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::binary_op_expression( (yystack_[2].value), ast::logic_and, (yystack_[0].value), yylhs.location ); }
#line 741 "parser.cpp" // lalr1.cc:847
    break;

  case 24:
#line 172 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::binary_op_expression( (yystack_[2].value), ast::equal, (yystack_[0].value), yylhs.location ); }
#line 747 "parser.cpp" // lalr1.cc:847
    break;

  case 25:
#line 175 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::binary_op_expression( (yystack_[2].value), ast::not_equal, (yystack_[0].value), yylhs.location ); }
#line 753 "parser.cpp" // lalr1.cc:847
    break;

  case 26:
#line 178 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::binary_op_expression( (yystack_[2].value), ast::lesser, (yystack_[0].value), yylhs.location ); }
#line 759 "parser.cpp" // lalr1.cc:847
    break;

  case 27:
#line 181 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::binary_op_expression( (yystack_[2].value), ast::lesser_or_equal, (yystack_[0].value), yylhs.location ); }
#line 765 "parser.cpp" // lalr1.cc:847
    break;

  case 28:
#line 184 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::binary_op_expression( (yystack_[2].value), ast::greater, (yystack_[0].value), yylhs.location ); }
#line 771 "parser.cpp" // lalr1.cc:847
    break;

  case 29:
#line 187 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::binary_op_expression( (yystack_[2].value), ast::greater_or_equal, (yystack_[0].value), yylhs.location ); }
#line 777 "parser.cpp" // lalr1.cc:847
    break;

  case 30:
#line 190 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::binary_op_expression( (yystack_[2].value), ast::add, (yystack_[0].value), yylhs.location ); }
#line 783 "parser.cpp" // lalr1.cc:847
    break;

  case 31:
#line 193 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::binary_op_expression( (yystack_[2].value), ast::subtract, (yystack_[0].value), yylhs.location ); }
#line 789 "parser.cpp" // lalr1.cc:847
    break;

  case 32:
#line 196 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::list_node( ast::negate, yylhs.location, {(yystack_[0].value)} ); }
#line 795 "parser.cpp" // lalr1.cc:847
    break;

  case 33:
#line 199 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::binary_op_expression( (yystack_[2].value), ast::multiply, (yystack_[0].value), yylhs.location ); }
#line 801 "parser.cpp" // lalr1.cc:847
    break;

  case 34:
#line 202 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::binary_op_expression( (yystack_[2].value), ast::divide, (yystack_[0].value), yylhs.location ); }
#line 807 "parser.cpp" // lalr1.cc:847
    break;

  case 35:
#line 205 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::binary_op_expression( (yystack_[2].value), ast::divide_integer, (yystack_[0].value), yylhs.location ); }
#line 813 "parser.cpp" // lalr1.cc:847
    break;

  case 36:
#line 208 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::binary_op_expression( (yystack_[2].value), ast::modulo, (yystack_[0].value), yylhs.location ); }
#line 819 "parser.cpp" // lalr1.cc:847
    break;

  case 37:
#line 211 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::binary_op_expression( (yystack_[2].value), ast::raise, (yystack_[0].value), yylhs.location ); }
#line 825 "parser.cpp" // lalr1.cc:847
    break;

  case 38:
#line 214 "parser.y" // lalr1.cc:847
    { (yylhs.value) = (yystack_[1].value); (yylhs.value)->location = yylhs.location; }
#line 831 "parser.cpp" // lalr1.cc:847
    break;

  case 45:
#line 231 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::list_node( ast::hash_expression, yylhs.location, {(yystack_[0].value), nullptr} ); }
#line 837 "parser.cpp" // lalr1.cc:847
    break;

  case 46:
#line 236 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::list_node( ast::array_application, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} ); }
#line 843 "parser.cpp" // lalr1.cc:847
    break;

  case 47:
#line 241 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::list_node( ast::array_function, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} ); }
#line 849 "parser.cpp" // lalr1.cc:847
    break;

  case 48:
#line 246 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::list_node( ast::anonymous, yylhs.location, {(yystack_[0].value)} ); }
#line 855 "parser.cpp" // lalr1.cc:847
    break;

  case 49:
#line 249 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value).as<ast::list_node>()->append( (yystack_[0].value) );
  }
#line 864 "parser.cpp" // lalr1.cc:847
    break;

  case 50:
#line 257 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::list_node( ast::anonymous, yylhs.location, {(yystack_[0].value), nullptr} ); }
#line 870 "parser.cpp" // lalr1.cc:847
    break;

  case 51:
#line 260 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::list_node( ast::anonymous, yylhs.location, {(yystack_[2].value), (yystack_[0].value)} ); }
#line 876 "parser.cpp" // lalr1.cc:847
    break;

  case 54:
#line 272 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = new ast::list_node( ast::call_expression, yylhs.location, {(yystack_[3].value), (yystack_[1].value)} );
  }
#line 884 "parser.cpp" // lalr1.cc:847
    break;

  case 55:
#line 279 "parser.y" // lalr1.cc:847
    { (yylhs.value) = new ast::list_node( ast::expression_list, yylhs.location, {(yystack_[0].value)} ); }
#line 890 "parser.cpp" // lalr1.cc:847
    break;

  case 56:
#line 282 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = (yystack_[2].value);
    (yylhs.value).as<ast::list_node>()->append( (yystack_[0].value) );
  }
#line 899 "parser.cpp" // lalr1.cc:847
    break;

  case 57:
#line 290 "parser.y" // lalr1.cc:847
    {
    (yylhs.value) = new ast::list_node( ast::if_expression, yylhs.location, {(yystack_[4].value), (yystack_[2].value),(yystack_[0].value)} );
  }
#line 907 "parser.cpp" // lalr1.cc:847
    break;


#line 911 "parser.cpp" // lalr1.cc:847
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


  const signed char parser::yypact_ninf_ = -38;

  const signed char parser::yytable_ninf_ = -1;

  const short int
  parser::yypact_[] =
  {
      25,   -38,    13,   -13,   -38,    -3,   -38,    25,   -38,    63,
      25,   -38,   -38,   -38,   -38,   -38,   110,   110,   110,   110,
     102,   110,    25,   -38,   230,   -38,   -38,   -38,   -38,   -38,
     -38,   -38,   -38,   -38,   -38,    18,   -38,   138,   -24,   -24,
     -24,   -38,    21,   -38,    20,   184,   160,    -7,   -38,    47,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,    49,    25,   110,
     102,    25,   -38,   -38,    23,   -38,   110,    25,   110,   251,
     271,   285,   285,   285,   285,   285,   285,    -6,    -6,   -18,
     -18,   -18,   -18,   -24,   230,   -14,    35,    63,   -38,   208,
     -38,   184,   -13,   -38,   230,   -38,   230,   110,   -38,   -38,
     -38,   110,    26,    36,   230,   230,   -38,   -38
  };

  const unsigned char
  parser::yydefact_[] =
  {
       3,    62,     0,    64,     4,     0,     1,    63,     2,     0,
       8,     5,    58,    59,    60,    61,     0,     0,     0,     0,
       0,     0,     0,     7,    11,    40,    20,    19,    43,    39,
      41,    52,    53,    44,    42,     0,     9,     0,    32,    21,
      45,    18,     0,    14,     0,    64,     0,     0,    48,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    16,    63,     0,    38,     0,     0,     0,    22,
      23,    24,    25,    26,    28,    27,    29,    30,    31,    33,
      34,    35,    36,    37,    55,     0,     0,     0,    10,     0,
      15,    64,    64,    12,    47,    49,    51,     0,    46,    54,
       6,     0,     0,     0,    56,    57,    13,    17
  };

  const signed char
  parser::yypgoto_[] =
  {
     -38,   -38,     6,     8,   -38,   -17,   -38,     9,   -38,   -16,
     -38,   -38,   -38,   -38,     4,   -38,   -38,    16,   -38,   -38,
     -38,   -38,     7,   -37
  };

  const signed char
  parser::yydefgoto_[] =
  {
      -1,     2,     3,     4,    35,    23,    42,    43,    44,    24,
      25,    26,    27,    47,    48,    28,    29,    95,    30,    31,
      32,    33,    34,     8
  };

  const unsigned char
  parser::yytable_[] =
  {
      37,    38,    39,    40,    45,    46,    76,     5,    74,     9,
      65,    64,    66,     6,     5,    11,    65,    36,    66,    60,
      61,    62,    63,    64,     7,   107,     1,   108,    65,    49,
      66,     1,    77,    10,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      94,     5,    72,    99,   101,    71,    67,    68,    70,    78,
     104,    97,   106,   103,   112,   113,   116,    12,    13,     1,
      14,    15,    16,   109,   107,    98,   117,   102,     5,   100,
     110,   105,    96,     0,    49,     0,     0,    17,     0,     0,
       0,   114,     0,     0,    18,   115,    19,     0,    20,    21,
       0,     0,     0,     0,     0,    22,    12,    13,     1,    14,
      15,    16,     0,    41,    12,    13,     1,    14,    15,    16,
       0,     0,     0,     0,     0,     0,    17,     0,     0,     0,
       0,     0,     0,    18,    17,    19,     0,     0,    21,     0,
       0,    18,     0,    19,    22,     0,    21,     0,    69,     0,
       0,     0,    22,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,     0,     0,
       0,     0,    65,     0,    66,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
       0,     0,     0,     0,    65,     0,    66,     0,    75,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,     0,     0,     0,     0,    65,     0,
      66,    73,   111,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,     0,     0,
       0,     0,    65,     0,    66,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
       0,     0,     0,     0,    65,     0,    66,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,     0,     0,     0,     0,    65,     0,    66,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,     0,     0,     0,     0,    65,     0,    66,    58,    59,
      60,    61,    62,    63,    64,     0,     0,     0,     0,    65,
       0,    66
  };

  const signed char
  parser::yycheck_[] =
  {
      16,    17,    18,    19,    20,    21,    13,     0,    45,    12,
      34,    29,    36,     0,     7,     7,    34,    10,    36,    25,
      26,    27,    28,    29,    37,    39,     6,    41,    34,    22,
      36,     6,    39,    36,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    44,    44,    69,    70,    35,    38,    39,    37,    12,
      76,    12,    78,    40,   101,   102,    40,     4,     5,     6,
       7,     8,     9,    38,    39,    68,    40,    71,    71,    70,
      97,    77,    66,    -1,    77,    -1,    -1,    24,    -1,    -1,
      -1,   107,    -1,    -1,    31,   111,    33,    -1,    35,    36,
      -1,    -1,    -1,    -1,    -1,    42,     4,     5,     6,     7,
       8,     9,    -1,    11,     4,     5,     6,     7,     8,     9,
      -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    24,    33,    -1,    -1,    36,    -1,
      -1,    31,    -1,    33,    42,    -1,    36,    -1,    10,    -1,
      -1,    -1,    42,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    -1,    -1,
      -1,    -1,    34,    -1,    36,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      -1,    -1,    -1,    -1,    34,    -1,    36,    -1,    38,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    -1,    -1,    -1,    -1,    34,    -1,
      36,    37,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    -1,    -1,
      -1,    -1,    34,    -1,    36,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      -1,    -1,    -1,    -1,    34,    -1,    36,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    -1,    -1,    -1,    -1,    34,    -1,    36,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    -1,    -1,    -1,    -1,    34,    -1,    36,    23,    24,
      25,    26,    27,    28,    29,    -1,    -1,    -1,    -1,    34,
      -1,    36
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,     6,    44,    45,    46,    65,     0,    37,    66,    12,
      36,    46,     4,     5,     7,     8,     9,    24,    31,    33,
      35,    36,    42,    48,    52,    53,    54,    55,    58,    59,
      61,    62,    63,    64,    65,    47,    65,    52,    52,    52,
      52,    11,    49,    50,    51,    52,    52,    56,    57,    65,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    34,    36,    38,    39,    10,
      37,    35,    46,    37,    66,    38,    13,    39,    12,    52,
      52,    52,    52,    52,    52,    52,    52,    52,    52,    52,
      52,    52,    52,    52,    52,    60,    60,    12,    65,    52,
      50,    52,    45,    40,    52,    57,    52,    39,    41,    38,
      48,    14,    66,    66,    52,    52,    40,    40
  };

  const unsigned char
  parser::yyr1_[] =
  {
       0,    43,    44,    44,    45,    45,    46,    46,    47,    47,
      47,    48,    48,    48,    49,    49,    50,    50,    51,    52,
      52,    52,    52,    52,    52,    52,    52,    52,    52,    52,
      52,    52,    52,    52,    52,    52,    52,    52,    52,    52,
      52,    52,    52,    52,    52,    53,    54,    55,    56,    56,
      57,    57,    58,    58,    59,    60,    60,    61,    62,    63,
      64,    64,    65,    66,    66
  };

  const unsigned char
  parser::yyr2_[] =
  {
       0,     2,     2,     0,     1,     3,     6,     3,     0,     1,
       3,     1,     4,     6,     1,     3,     2,     5,     1,     1,
       1,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     3,     3,     3,     3,     3,     3,     1,
       1,     1,     1,     1,     1,     2,     4,     4,     1,     3,
       1,     3,     1,     1,     4,     1,     3,     6,     1,     1,
       1,     1,     1,     1,     0
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
  "param_list", "expr_block", "let_block_list", "let_block", "let", "expr",
  "hash", "array_apply", "array_func", "array_arg_list", "array_arg",
  "number", "call", "expr_list", "if_expr", "int", "real", "boolean", "id",
  "optional_semicolon", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned short int
  parser::yyrline_[] =
  {
       0,    57,    57,    66,    73,    78,    87,    92,   100,   102,
     105,   114,   119,   124,   131,   133,   142,   147,   154,   158,
     160,   162,   165,   168,   171,   174,   177,   180,   183,   186,
     189,   192,   195,   198,   201,   204,   207,   210,   213,   216,
     218,   220,   222,   224,   226,   230,   235,   240,   245,   248,
     256,   259,   264,   266,   271,   278,   281,   289,   296,   299,
     303,   305,   308,   311,   311
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
#line 1449 "parser.cpp" // lalr1.cc:1155
#line 314 "parser.y" // lalr1.cc:1156


void
stream::parsing::parser::error (const location_type& l, const std::string& m)
{
  driver.error (l, m);
}
