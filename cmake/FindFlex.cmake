include(FindPackageHandleStandardArgs)

find_path(FLEX_INCLUDE_DIR FlexLexer.h)
find_program(FLEX_PROGRAM flex)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Flex DEFAULT_MSG FLEX_PROGRAM FLEX_INCLUDE_DIR)
