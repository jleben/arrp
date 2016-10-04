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

add_custom_target(tests)

function(icc_compile_test name sources deps)
  set(source_paths "")
  foreach(source ${sources})
    list(APPEND source_paths ${CMAKE_CURRENT_SOURCE_DIR}/${source})
  endforeach()

  add_custom_command(OUTPUT ${name}
    COMMAND /home/jakob/intel/bin/icc
    #COMMAND g++
      -o ${name}
      -std=c++11 -O3 -g -Winline -inline-forceinline
      -fp-model strict
      #-std=c++11 -O3 -g -Winline
      -I ${CMAKE_CURRENT_BINARY_DIR}
      ${source_paths}
      -lm -lpapi
    DEPENDS ${sources} ${deps}
      ${CMAKE_SOURCE_DIR}/test/common/generic_tester.hpp
      ${CMAKE_SOURCE_DIR}/test/common/generic_printer.hpp
  )
  add_custom_target(${name} DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${name})
endfunction()

function(add_stream_test name source options cpp_extra)

  string(REPLACE " " ";" options "${options}")
  stream_to_cpp(${name} ${source} ${options})
  #stream_to_cpp(${name}_program ${source} "none")
  icc_compile_test(test_${name} "${cpp_extra}" "${name}.cpp")
  add_dependencies(test_${name} ${name})
  add_dependencies(tests test_${name})

endfunction()

function(add_output_test name source)

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
  set(main_file "${CMAKE_SOURCE_DIR}/test/common/test_main.cpp")
  set(headers
    "${CMAKE_SOURCE_DIR}/test/common/io.hpp"
    "${CMAKE_SOURCE_DIR}/test/common/test_parser.hpp"
    ${test_header}
  )

  add_custom_command(OUTPUT ${tester_name}
    COMMAND c++ -std=c++11 -O0 -g ${main_file}
      "-DPROGRAM_SOURCE=<${program_file}>"
      ${define_test_header}
      "-I${CMAKE_SOURCE_DIR}/test/common"
      "-I${CMAKE_SOURCE_DIR}/cpp"
      "-I${CMAKE_CURRENT_BINARY_DIR}"
      "-I${CMAKE_CURRENT_SOURCE_DIR}"
      ${test_libs}
      -o ${tester_name}
      $<TARGET_FILE:test_lib>
    DEPENDS ${program_file} ${main_file} ${headers} test_lib
    VERBATIM
  )

  add_custom_target(${tester_name} DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${tester_name}")

  add_dependencies(tests ${tester_name})

  add_test(NAME ${name} COMMAND
    "${CMAKE_CURRENT_BINARY_DIR}/${tester_name}"
    "--compare=${CMAKE_CURRENT_SOURCE_DIR}/${source}"
  )
endfunction()

add_subdirectory(common)
add_subdirectory(unit)
add_subdirectory(apps)
