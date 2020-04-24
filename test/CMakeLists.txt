
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
add_subdirectory(library)
add_subdirectory(apps)

set(ARRP_COMPILER ${CMAKE_BINARY_DIR}/compiler/arrp)
set(TEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common)
set(CPP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/cpp)

configure_file(make_test.sh.in make_test.sh)
