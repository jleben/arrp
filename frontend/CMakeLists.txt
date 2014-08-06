option(FRONTEND_PRINT_TOKENS "Parser: enable printing tokens." OFF)
option(BISON_VERBOSE "Bison: Output detailed information about parser generation." OFF)

# Discover LLVM dependencies

#if (NOT DEFINED llvm-compile-flags)
  execute_process(
    COMMAND llvm-config --cxxflags
    OUTPUT_VARIABLE llvm-compile-flags
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
#endif()

#if (NOT DEFINED llvm-link-flags)
  execute_process(
    COMMAND llvm-config --ldflags
    OUTPUT_VARIABLE llvm-link-flags
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  execute_process(
    COMMAND llvm-config --libs core
    OUTPUT_VARIABLE llvm-link-libs
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
#endif()

# Lexer and parser generation

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

# Frontend executable

add_executable(frontend
  lex.cc
  parse.cc
  #semantic.cpp
  environment.cpp
  types.cpp
  type_checker.cpp
  ir-generator.cpp
  frontend.cpp
)

#set_property( TARGET frontend APPEND PROPERTY COMPILE_FLAGS ${llvm-compile-flags} )

target_link_libraries( frontend ${llvm-link-libs} ${llvm-link-flags}  )
