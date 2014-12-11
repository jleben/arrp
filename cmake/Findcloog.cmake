include(FindPackageHandleStandardArgs)

find_path(CLOOG_INCLUDE_DIR cloog/cloog.h HINTS extra/cloog/include)
find_library(CLOOG_LIBRARY cloog-isl HINTS extra/cloog/lib)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(cloog DEFAULT_MSG CLOOG_INCLUDE_DIR CLOOG_LIBRARY)
