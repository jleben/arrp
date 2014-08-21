# Allow test source files generated from template to find test headers:

set(frontend_dir ${CMAKE_SOURCE_DIR}/frontend)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

function(add_unit_test test_header test_function)
  set(test_name ${test_function})

  set(clean_test_name ${test_name})
  string(REPLACE "::" "_" clean_test_name ${clean_test_name})

  set(test_driver_src ${clean_test_name}_driver.cpp)
  configure_file(unit_test_driver.cpp.template ${test_driver_src})

  set(test_driver "unit_test_${clean_test_name}")
  add_executable(${test_driver} ${test_driver_src} unit_test.cpp)

  target_link_libraries(${test_driver} frontend_lib)

  add_test(NAME ${test_name} COMMAND ${test_driver})
endfunction()

add_unit_test(test_semantics.hpp "binop::ii")
add_unit_test(test_semantics.hpp "binop::ir")
add_unit_test(test_semantics.hpp "binop::ri")
add_unit_test(test_semantics.hpp "binop::rr")
