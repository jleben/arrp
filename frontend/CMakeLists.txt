option(PARSER_REGENERATE "Parser: enable regeneration using flex and bison" OFF)
option(PARSER_GENERATOR_OUTPUT_DESCRIPTION "Parser: output parser description when generating." OFF)

# Lexer and parser generation

if(PARSER_REGENERATE)

set(bison_opts "")
if(PARSER_GENERATOR_OUTPUT_DESCRIPTION)
  list(APPEND bison_opts -r state -r solved)
endif()

file(COPY "${FLEX_INCLUDE_DIR}/FlexLexer.h" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/scanner.cpp"
  COMMAND ${FLEX_PROGRAM} ARGS -o scanner.cpp scanner.l
  DEPENDS scanner.l
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMENT "Generating scanner..."
)

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp"
  COMMAND bison ARGS -o parser.cpp ${bison_opts} parser.y
  DEPENDS parser.y
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMENT "Generating parser..."
)

endif(PARSER_REGENERATE)

add_custom_target(parser_core DEPENDS scanner.cpp parser.cpp)

set(src
  scanner.cpp
  parser.cpp
  driver.cpp
)

add_library(parser STATIC ${src})
