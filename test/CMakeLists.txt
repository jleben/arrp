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

function(icc_compile_test name)
  set(sources ${ARGN})
  set(source_paths "")
  foreach(source ${sources})
    list(APPEND source_paths ${CMAKE_CURRENT_SOURCE_DIR}/${source})
  endforeach()
  add_custom_command(OUTPUT ${name}
    COMMAND /home/jakob/intel/bin/icc
    #COMMAND g++
      -o ${name} -std=c++11 -O3 -Winline -I ${CMAKE_CURRENT_BINARY_DIR} ${source_paths}
    DEPENDS ${sources}
  )
  add_custom_target(${name} DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${name})
endfunction()

function(add_stream_test name source options cpp_extra)

  string(REPLACE " " ";" options "${options}")
  stream_to_cpp(${name}_kernel ${source} ${options})
  #stream_to_cpp(${name}_kernel ${source} "none")
  icc_compile_test(${name} ${cpp_extra})
  add_dependencies(${name} ${name}_kernel)
  add_dependencies(tests ${name})

endfunction()

function(add_test name)
  icc_compile_test(${name} ${ARGN})
  add_dependencies(tests ${name})
endfunction()

add_subdirectory(performance)
