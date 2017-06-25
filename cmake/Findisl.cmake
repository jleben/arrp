include(FindPackageHandleStandardArgs)

if (NOT ISL_INCLUDE_DIR OR NOT ISL_LIBRARY)
    find_path(ISL_INCLUDE_DIR isl/ctx.h PATHS extra/isl/build/include NO_DEFAULT_PATH)
    find_library(ISL_LIBRARY libisl.a PATHS extra/isl/build/lib NO_DEFAULT_PATH)
endif()

if (NOT ISL_INCLUDE_DIR AND NOT ISL_LIBRARY)
    find_path(ISL_INCLUDE_DIR isl/ctx.h)
    find_library(ISL_LIBRARY isl)
endif()

string(FIND "${ISL_LIBRARY}" "${CMAKE_SOURCE_DIR}" SOURCE_DIR_POS)
if (SOURCE_DIR_POS EQUAL 0)
    set(ISL_LIBRARY_IS_INTERNAL TRUE CACHE BOOL "")
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(isl DEFAULT_MSG ISL_INCLUDE_DIR ISL_LIBRARY)
