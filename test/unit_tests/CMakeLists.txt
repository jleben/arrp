# Allow test source files generated from template to find test headers:

set(frontend_dir ${CMAKE_SOURCE_DIR}/frontend)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(unit_testing unit_test.cpp)
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

set(semantic_test_funcs
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
"binop::rS"
"binop::Si"
"binop::Sr"
"binop::SS"
"binop::raise_int_int"
"binop::raise_real_real"
"binop::raise_int_real"
"binop::raise_real_int"
"binop::raise_stream_int"
"slicing::stream1_by_int"
"slicing::stream1_by_range"
"slicing::stream3_by_int"
"slicing::stream3_by_range"
"slicing::stream3_by_int2"
"slicing::stream3_by_range2"
"slicing::stream3_by_range_int_range"
"transposition::stream3_to_2"
"transposition::stream4_to_3_2"
"reduction::stream_1d"
#"reduction::stream_1d_to_int"
#"reduction::stream_1d_to_stream_1d"
"mapping::stream1"
"mapping::stream1_to_real"
"mapping::stream1_to_int"
"mapping::stream1_to_stream2"
"mapping::stream1_take_n"
"mapping::stream1_take_n_every_n"
"mapping::stream3"
"mapping::stream3_take_n"
"mapping::range"
"mapping::range_take_n"
"mapping::range_take_n_every_n"
)

add_unit_tests(semantics test_semantics.hpp ${semantic_test_funcs})

set(polyhedral_tests
"poly::slice::dim_0_of_3"
"poly::slice::dummy"
)

add_unit_tests(polyhedral test_polyhedral_model.hpp ${polyhedral_tests})
