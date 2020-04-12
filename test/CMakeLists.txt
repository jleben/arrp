
function(stream_to_cpp name source)

  set(source_path ${CMAKE_CURRENT_SOURCE_DIR}/${source})
  set(options ${ARGN})

  add_custom_command(OUTPUT ${name}.cpp
    COMMAND $<TARGET_FILE:arrp>
      ${source_path} --cpp ${name} -i "${CMAKE_SOURCE_DIR}/test/libs" ${options}
    DEPENDS ${source_path} arrp
    VERBATIM
  )

  add_custom_target(${name} DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${name}.cpp")

endfunction()

function(compile_output_test name test_source test_header test_libs)
  if(test_header)
    set(define_test_header "-DTEST_HEADER=<${test_header}>")
  endif()

  set(main_source "${CMAKE_SOURCE_DIR}/test/common/test_main.cpp")
  set(headers
    "${CMAKE_SOURCE_DIR}/test/common/io.hpp"
    "${CMAKE_SOURCE_DIR}/test/common/test_parser.hpp"
    ${test_header}
  )

  add_custom_command(OUTPUT ${name}
    COMMAND c++ -std=c++17 -O0 -g ${main_source}
      "-DPROGRAM_SOURCE=<${test_source}>"
      ${define_test_header}
      "-I${CMAKE_SOURCE_DIR}/test/common"
      "-I${CMAKE_SOURCE_DIR}/cpp"
      "-I${CMAKE_CURRENT_BINARY_DIR}"
      "-I${CMAKE_CURRENT_SOURCE_DIR}"
      ${test_libs}
      -o ${name}
      $<TARGET_FILE:test_lib>
    DEPENDS ${test_source} ${main_source} ${headers} test_lib
    VERBATIM
  )
endfunction()

function(add_output_test_old name source)

  if(NOT ARGC LESS 3)
    list(GET ARGN 0 test_header)
  endif()

  if (NOT ARGC LESS 4)
    list(GET ARGN 1 test_libs)
  endif()

  string(REGEX MATCH "\\.cpp$" cpp_ending ${source})

  if (cpp_ending)
    set(cpp_source ${source})
  else()
    stream_to_cpp(${name} ${source} "--cpp-namespace;test")
    set(cpp_source "${name}.cpp")
  endif()

  set(tester_name test_${name})

  compile_output_test(${tester_name} "${cpp_source}" "${test_header}" "${test_libs}")

  add_custom_target(${tester_name} DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${tester_name}")

  add_dependencies(tests ${tester_name})

  add_test(NAME ${name} COMMAND
    "${CMAKE_CURRENT_BINARY_DIR}/${tester_name}"
    "--compare=${CMAKE_CURRENT_SOURCE_DIR}/${source}"
  )
endfunction()

function(add_output_test name source compile_options run_options)
  add_test(NAME ${name}
    COMMAND sh "${CMAKE_SOURCE_DIR}/test/common/compile_and_evaluate.sh"
        "${name}" "${source}" "${compile_options}" "${run_options}"
  )
  set_property(TEST ${name}
    PROPERTY ENVIRONMENT
      CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}
      CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}
      CXX=${CMAKE_CXX_COMPILER}
      ARRP_INSTALL_DIR=${CMAKE_INSTALL_PREFIX}
  )
  set_property(TEST ${name} PROPERTY TIMEOUT 10)
endfunction()

add_custom_target(tests)

add_subdirectory(exe)
add_subdirectory(common)
add_subdirectory(unit)
add_subdirectory(apps)

set(ARRP_COMPILER ${CMAKE_BINARY_DIR}/compiler/arrp)
set(TEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common)
set(CPP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/cpp)

configure_file(make_test.sh.in make_test.sh)
