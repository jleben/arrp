
find_program(icc_program icc REQUIRED)
if(NOT icc_program)
  message(FATAL_ERROR "Could not find ICC compiler required for performance tests.")
endif()

function(compile_perf_test name main_source test_source test_header test_libs)
  if(test_header)
    set(define_test_header "-DTEST_HEADER=<${test_header}>")
  endif()

  set(headers
    "${CMAKE_SOURCE_DIR}/test/common/perf_io.hpp"
    "${CMAKE_SOURCE_DIR}/test/common/perf_test_driver.hpp"
    ${test_header}
  )

  add_custom_command(OUTPUT ${name}
    COMMAND ${icc_program} -std=c++11
      -O3
      -g
      -Winline -inline-forceinline
      -qopt-report-phase=vec
      -qopt-report=5
      -qopt-report-file=${name}.report
      -ansi-alias
      #-ivdep-parallel
      #-parallel
      -restrict
      #-guide
      ${main_source}
      "-DPROGRAM_SOURCE=<${test_source}>"
      ${define_test_header}
      "-I${CMAKE_SOURCE_DIR}/test/common"
      "-I${CMAKE_SOURCE_DIR}/cpp"
      "-I${CMAKE_CURRENT_BINARY_DIR}"
      "-I${CMAKE_CURRENT_SOURCE_DIR}"
      ${test_libs}
      -lpapi
      -o ${name}
      $<TARGET_FILE:test_lib>
    DEPENDS ${test_source} ${main_source} ${headers} test_lib
    VERBATIM
  )
endfunction()

function(add_perf_test name source)

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
    stream_to_cpp(${name}_perf ${source} "--cpp-namespace;test;--ast-avoid-branch-in-loop")
    set(cpp_source "${name}.cpp")
  endif()

  ## Raw

  set(driver_name run_${name})

  compile_perf_test(${driver_name}
    "${CMAKE_SOURCE_DIR}/test/common/perf_pure_main.cpp"
    "${cpp_source}" "${test_header}" "${test_libs}")

  add_custom_target(${driver_name} DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${driver_name}")

  add_dependencies(perf_tests ${driver_name})


  ## Profile

  set(profiler_name profile_${name})

  compile_perf_test(${profiler_name}
    "${CMAKE_SOURCE_DIR}/test/common/perf_test_main.cpp"
    "${cpp_source}" "${test_header}" "${test_libs}")

  add_custom_target(${profiler_name} DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${profiler_name}")

  add_dependencies(perf_tests ${profiler_name})

  ## Debug

  set(debug_name debug_${name})

  compile_perf_test(${debug_name}
    "${CMAKE_SOURCE_DIR}/test/common/test_main.cpp"
    "${cpp_source}" "${test_header}" "${test_libs}")

  add_custom_target(${debug_name} DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${debug_name}")

  add_dependencies(perf_tests ${debug_name})

endfunction()


add_custom_target(perf_tests)
add_dependencies(tests perf_tests)

#add_subdirectory(dummy)
add_subdirectory(mfcc)
