option(PARSER_REGENERATE "Parser: enable regeneration using flexc++ bisonc++" OFF)
option(PARSER_PRINT_TOKENS "Parser: enable printing tokens." OFF)
option(BISON_VERBOSE "Bison: Output detailed information about parser generation." OFF)

# Lexer and parser generation

if(PARSER_REGENERATE)

set(bison_opts "")

if(PARSER_PRINT_TOKENS)
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

endif(PARSER_REGENERATE)

# Frontend library
if(WIN32)
  set(lib_mode STATIC)
else()
  set(lib_mode SHARED)
endif()

add_library(frontend_lib ${lib_mode}
  lex.cc
  parse.cc
  environment.cpp
  types.cpp
  type_checker.cpp
  ir-generator.cpp
  ../polyhedral/translator.cpp
  ../polyhedral/printer.cpp
)

#set_property( TARGET frontend_lib APPEND PROPERTY COMPILE_FLAGS ${llvm-compile-flags} )

if(WIN32)
  llvm_config(frontend_lib core)
else()
  target_link_libraries( frontend_lib ${llvm-link-libs} ${llvm-link-flags}  )
endif()

# Frontend executable

add_executable(frontend frontend.cpp)
target_link_libraries(frontend frontend_lib)
