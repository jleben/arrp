include(streams)

compile_stream_func( matrix_mul_kernel
  ${CMAKE_SOURCE_DIR}/examples/matrix-mult.in
  "-g;matrix_multiply;[5,2,3];[5,3,2]" )

add_executable( test-matrix-mult EXCLUDE_FROM_ALL
  test-matrix-mult.cpp ${matrix_mul_kernel} )

add_subdirectory(unit_tests)
