option(PARSER_REGENERATE "Parser: enable regeneration using flexc++ bisonc++" OFF)
option(PARSER_PRINT_TOKENS "Parser: enable printing tokens." OFF)
option(PARSER_ERROR_VERBOSE "Parser: enable verbose error output." OFF)
option(BISON_VERBOSE "Bison: Output detailed information about parser generation." OFF)

# Lexer and parser generation

if(PARSER_REGENERATE)

set(bison_opts "")

if(PARSER_PRINT_TOKENS)
  list(APPEND bison_opts "--print")
endif()

if(PARSER_ERROR_VERBOSE)
  list(APPEND bison_opts "--error-verbose")
endif()

if(BISON_VERBOSE)
  list(APPEND bison_opts "--verbose")
endif()

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/lex.cc"
  COMMAND flexc++ ARGS scanner.l
  DEPENDS scanner.l
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMENT "Generating scanner..."
)

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/parse.cc"
  COMMAND bisonc++ ARGS ${bison_opts} parser.y
  DEPENDS parser.y
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMENT "Generating parser..."
)

endif(PARSER_REGENERATE)

add_custom_target(parser DEPENDS lex.cc parse.cc)
