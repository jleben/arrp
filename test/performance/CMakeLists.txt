
find_program(icc_program icc REQUIRED)
if(NOT icc_program)
  message(FATAL_ERROR "Could not find ICC compiler required for performance tests.")
endif()

function(add_perf_test name source)

  if(NOT ARGC LESS 3)
    list(GET ARGN 0 test_header)
    set(define_test_header "-DTEST_HEADER=<${test_header}>")
  endif()

  if (NOT ARGC LESS 4)
    list(GET ARGN 1 test_libs)
  endif()

  stream_to_cpp(${name} ${source} "--cpp-namespace;test")

  set(tester_name test_${name})
  set(program_file "${name}.cpp")
  set(main_file "${CMAKE_SOURCE_DIR}/test/common/perf_test_main.cpp")
  set(headers
    "${CMAKE_SOURCE_DIR}/test/common/perf_io.hpp"
    "${CMAKE_SOURCE_DIR}/test/common/perf_test_driver.hpp"
    ${test_header}
  )

  add_custom_command(OUTPUT ${tester_name}
    COMMAND ${icc_program} -std=c++11
      -O3
      -Winline -inline-forceinline
      ${main_file}
      "-DPROGRAM_SOURCE=<${program_file}>"
      ${define_test_header}
      "-I${CMAKE_SOURCE_DIR}/test/common"
      "-I${CMAKE_SOURCE_DIR}/cpp"
      "-I${CMAKE_CURRENT_BINARY_DIR}"
      "-I${CMAKE_CURRENT_SOURCE_DIR}"
      ${test_libs}
      -lpapi
      -o ${tester_name}
      $<TARGET_FILE:test_lib>
    DEPENDS ${program_file} ${main_file} ${headers} test_lib
    VERBATIM
  )

  add_custom_target(${tester_name} DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${tester_name}")

  add_dependencies(perf_tests ${tester_name})

endfunction()

add_custom_target(perf_tests)
add_dependencies(tests perf_tests)

add_subdirectory(dummy)
