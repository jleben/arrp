include(streams)

function(add_integration_test name source func args driver)
  set(kernel ${name}_kernel)
  set(meta ${name}.meta)
  set(exe test-${name})

  compile_stream_func( ${kernel}
    ${source} ${func} "${args}"
    "-m;${meta};--cpp;${name}.h"
  )

  include_directories(${CMAKE_CURRENT_BINARY_DIR})

  add_executable( ${exe} ${driver} ${${kernel}} )

  target_link_libraries( ${exe} streamc-interface-cpp )

  add_test(NAME ${name} COMMAND $<TARGET_FILE:${exe}>)
endfunction()

add_integration_test(sum
  ${CMAKE_SOURCE_DIR}/examples/audio-features.in
  sum "[10]"
  sum.cpp
)

add_integration_test(rms
  ${CMAKE_SOURCE_DIR}/examples/audio-features.in
  rms "[10]"
  rms.cpp
)

add_integration_test(matrix-mult
  ${CMAKE_SOURCE_DIR}/examples/matrix-mult.in
  matrix_multiply "[5,2,3];[5,3,2]"
  test-matrix-mult.cpp
)
