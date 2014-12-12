# Allow test source files generated from template to find test headers:

set(frontend_dir ${CMAKE_SOURCE_DIR}/frontend)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(unit_testing_src
  unit_test.cpp
  unit_test_semantic.cpp
  unit_test_polyhedral_model.cpp
)

add_library(unit_testing ${unit_testing_src})
target_link_libraries(unit_testing streamc-lib)

function(add_unit_tests name test_header)

  set(test_function_call_code "")
  foreach(function IN LISTS ARGN)
    set(code "    if(test_name == \"${function}\") return ${function}();\n" )
    set(test_function_call_code "${test_function_call_code}${code}")
  endforeach()

  set(test_driver_src ${name}.driver.cpp)
  configure_file(unit_test_driver.cpp.template ${test_driver_src})

  set(test_driver "unit_test_${name}")
  add_executable(${test_driver} ${test_driver_src})

  target_link_libraries(${test_driver} unit_testing)

  foreach(function IN LISTS ARGN)
    set(test_name ${function})
    add_test(NAME ${test_name} COMMAND ${test_driver} "${test_name}")
  endforeach()

endfunction()


set(intrinsics_tests
"intrinsics::add_int_int_const"
"intrinsics::add_int_real_const"
"intrinsics::add_real_int_const"
"intrinsics::add_real_real_const"
"intrinsics::add_int_int"
"intrinsics::add_int_real"
"intrinsics::add_real_int"
"intrinsics::add_real_real"
"intrinsics::add_int_range"
"intrinsics::add_real_range"
"intrinsics::add_range_int"
"intrinsics::add_range_real"
"intrinsics::add_range_range"
"intrinsics::add_int_stream"
"intrinsics::add_stream_int"
"intrinsics::add_real_stream"
"intrinsics::add_stream_real"
"intrinsics::power_int_int_const"
"intrinsics::power_real_real_const"
"intrinsics::power_int_real_const"
"intrinsics::power_real_int_const"
"intrinsics::power_int_int"
"intrinsics::power_real_real"
"intrinsics::power_int_real"
"intrinsics::power_real_int"
"intrinsics::power_stream_int"
"intrinsics::div_int_int_const"
"intrinsics::div_real_real_const"
"intrinsics::div_int_real_const"
"intrinsics::div_real_int_const"
"intrinsics::div_int_int"
"intrinsics::div_real_real"
"intrinsics::div_int_real"
"intrinsics::div_real_int"
"intrinsics::i_div_int_int_const"
"intrinsics::i_div_real_real_const"
"intrinsics::i_div_int_real_const"
"intrinsics::i_div_real_int_const"
"intrinsics::i_div_int_int"
"intrinsics::i_div_real_real"
"intrinsics::i_div_int_real"
"intrinsics::i_div_real_int"
)

add_unit_tests(intrinsics test_intrinsics.hpp ${intrinsics_tests})

set(transform_tests
"slice::stream1_by_int"
"slice::stream1_by_range"
"slice::stream3_by_int"
"slice::stream3_by_range"
"slice::stream3_by_int2"
"slice::stream3_by_range2"
"slice::stream3_by_int3"
"slice::stream3_by_range3"
"slice::stream3_by_range_int_range"
"transpose::stream3_to_dim2"
"transpose::stream4_to_dim2_dim3"
"transpose::stream4_to_dim4_dim2"
"transform::slice_by_scalar_and_transpose"
"transform::transpose_and_slice_by_scalar"
"transform::stream2_map_and_slice"
)

add_unit_tests(transform test_transform.hpp ${transform_tests})

set(map_tests
"map::stream1_identity"
"map::stream1_add_scalar"
"map::stream1_mul_range"
"map::stream1_add_stream1"
"map::stream1_take_n_identity"
"map::stream1_take_n_raise_int"
"map::stream1_take_n_sub_range"
"map::stream1_take_n_every_n_identity"
"map::stream3_identity"
"map::stream3_max_double"
"map::stream3_take_n_identity"
"map::stream3_take_n_every_n_identity"
"map::range_identity"
"map::range_add_int"
"map::range_add_range"
"map::range_every_n_identity"
"map::range_to_stream1"
)

add_unit_tests(map test_map.hpp ${map_tests})

set(reduce_tests
"reduce::stream1_add"
"reduce::map_range_by_stream1_reduction"
"reduce::map_stream2_by_substream_reduction"
)

add_unit_tests(reduce test_reduce.hpp ${reduce_tests})
