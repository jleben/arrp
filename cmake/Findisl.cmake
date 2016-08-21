include(FindPackageHandleStandardArgs)

find_path(ISL_INCLUDE_DIR isl/ctx.h HINTS extra/isl/build/include)
find_library(ISL_LIBRARY isl HINTS extra/isl/build/lib)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(isl DEFAULT_MSG ISL_INCLUDE_DIR ISL_LIBRARY)
