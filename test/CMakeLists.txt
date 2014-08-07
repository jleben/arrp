include(streams)

compile_stream_func(reduction_kernel ${CMAKE_CURRENT_SOURCE_DIR}/test-reduction.in "-g;f;[10]")
add_executable(test-reduction EXCLUDE_FROM_ALL test-reduction.cpp ${reduction_kernel})

compile_stream_func( matrix_mul_kernel
  ${CMAKE_SOURCE_DIR}/examples/matrix-mult.in
  "-g;matrix_multiply;[5,2,2];[5,2,2]" )

add_executable( test-matrix-mult EXCLUDE_FROM_ALL
  test-matrix-mult.cpp ${matrix_mul_kernel} )
