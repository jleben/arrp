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

set(input_source_code
  slice1.in
)

foreach(input IN LISTS input_source_code)
  configure_file(${input} ${input} COPYONLY)
endforeach()

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


set(binop_tests
"binop::ii"
"binop::ir"
"binop::ri"
"binop::rr"
"binop::iR"
"binop::rR"
"binop::Ri"
"binop::Rr"
"binop::RR"
"binop::iS"
"binop::Si"
"binop::rS"
"binop::Sr"
"binop::raise_int_int"
"binop::raise_real_real"
"binop::raise_int_real"
"binop::raise_real_int"
"binop::raise_stream_int"
)

add_unit_tests(binop test_binop.hpp ${binop_tests})

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

