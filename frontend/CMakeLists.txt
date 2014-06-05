option(FRONTEND_PRINT_TOKENS "Parser: enable printing tokens." OFF)
option(BISON_VERBOSE "Bison: Output detailed information about parser generation." OFF)

set(bison_opts "")

if(FRONTEND_PRINT_TOKENS)
  list(APPEND bison_opts "--print")
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

add_executable(test
  lex.cc
  parse.cc
  test.cpp
)
