Language Syntax
###############

The following syntax is in EBNF, except the definitions on the right
side of "~=" are regular expressions.

::

  program = stmt-list [ ";" ]

  stmt-list = stmt { ";" stmt }

  stmt = id [ "(" [ param-list ] ")" ] "=" block

  param-list = id { ";" id }

  block =
      expr
      |
      "{" [ let-list ";" ] expr [ ";" ] "}"

  let-list = let { ";" let }

  let =
      "let" stmt
      |
      "let" "{" stmt-list [ ";" ] "}"

  expr =
      id
      |
      int
      |
      real
      |
      boolean
      |
      unary-op expr
      |
      expr binary-op expr
      |
      conditional
      |
      func-application
      |
      array
      |
      array-application
      |
      array-recursion
      |
      array-size
      |
      "(" expr ")"

  id ~= [a-zA-Z][a-zA-Z_0-9]*

  int ~= [0-9]+

  real ~= [0-9]+\.[0-9]+

  boolean = "true" | "false"

  unary-op = "!" | "-"

  binary-op =
      "||" | "&&" | "==" | "!=" | "<" | "<=" | ">" | ">=" |
      "+" | "-" | "*" | "/" | "//" | "%" | "^"

  conditional = "if" expr "then" expr "else" expr

  func-application = expr "(" expr-list ")"

  array = "[" array-param-list "->" ( expr | case ) "]"

  array-param-list = array-param { ";" array-param }

  array-param = id [ ":" array-param-bound ]

  array-param-bound = expr

  array-application = expr "[" expr-list "]"

  array-recursion = "this" "[" expr-list "]"

  array-size = "#" expr [ "@" expr ]

  case = "case" subdomain-list ";" "else" ":" expr

  subdomain-list = subdomain { ";" subdomain }

  subdomain = domain ":" expr

  domain = expr
