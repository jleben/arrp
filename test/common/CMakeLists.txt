
set(test_lib_src
  ../../compiler/arg_parser.cpp
  test_parser.cpp
  testing.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/cpp)
add_library(test_lib STATIC ${test_lib_src})
