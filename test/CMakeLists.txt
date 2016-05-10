function(stream_to_cpp name source)

  set(source_path ${CMAKE_CURRENT_SOURCE_DIR}/${source})
  set(options ${ARGN})

  add_custom_command(OUTPUT ${name}.cpp
    COMMAND $<TARGET_FILE:streamc>
      ${source_path} --cpp ${name} ${options}
    DEPENDS ${source_path} streamc
    VERBATIM
  )

  add_custom_target(${name} DEPENDS ${name}.cpp)

endfunction()

add_custom_target(tests)

function(icc_compile_test name sources deps)
  set(source_paths "")
  foreach(source ${sources})
    list(APPEND source_paths ${CMAKE_CURRENT_SOURCE_DIR}/${source})
  endforeach()

  add_custom_command(OUTPUT ${name}
    #COMMAND /home/jakob/intel/bin/icc
    COMMAND g++
      -o ${name}
      #-std=c++11 -O3 -Winline -inline-forceinline -fp-model strict
      -std=c++11 -O3 -Winline -g
      -I ${CMAKE_CURRENT_BINARY_DIR}
      ${source_paths}
      -lm -lpapi
    DEPENDS ${sources} ${deps}
  )
  add_custom_target(${name} DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${name})
endfunction()

function(add_stream_test name source options cpp_extra)

  string(REPLACE " " ";" options "${options}")
  stream_to_cpp(${name}_kernel ${source} ${options})
  #stream_to_cpp(${name}_kernel ${source} "none")
  icc_compile_test(${name} "${cpp_extra}" "${name}_kernel.cpp")
  add_dependencies(${name} ${name}_kernel)
  add_dependencies(tests ${name})

endfunction()

function(add_test name)
  icc_compile_test(${name} "${ARGN}" "")
  add_dependencies(tests ${name})
endfunction()

add_subdirectory(performance)
