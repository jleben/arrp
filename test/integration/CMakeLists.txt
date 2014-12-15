if (DEFINED TEST_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE ${TEST_BUILD_TYPE})
endif()

include(streams)

function(add_integration_test target_name source func args driver)
  set(kernel_target_name ${target_name}-kernel)
  set(meta ${target_name}.meta)

  compile_stream_func( ${kernel_target_name}
    ${source} ${func} "${args}"
    "-m;${meta};--cpp;${target_name}.h"
  )

  include_directories(${CMAKE_CURRENT_BINARY_DIR})

  add_executable( ${target_name} ${driver} ${${kernel_target_name}_object} )

  target_link_libraries( ${target_name} streamc-interface-cpp )

  add_test(NAME ${target_name} COMMAND $<TARGET_FILE:${target_name}>)
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

add_integration_test(centroid
  ${CMAKE_SOURCE_DIR}/examples/audio-features.in
  centroid "[100]"
  centroid.cpp
)

add_integration_test(mat-mul
  ${CMAKE_SOURCE_DIR}/examples/matrix-mult.in
  matrix_multiply "[10,128,256];[10,256,128]"
  mat-mul.cpp
)

add_integration_test(flux
  ${CMAKE_SOURCE_DIR}/examples/spectral-flux.in
  flux "[1000,1000]"
  flux.cpp
)

add_integration_test(flux-no-log
  ${CMAKE_SOURCE_DIR}/examples/spectral-flux.in
  flux_no_log "[1000,1000]"
  flux.cpp
)
target_compile_definitions(flux-no-log PRIVATE NO_LOGARITHM)

add_integration_test(flux-stream
  ${CMAKE_SOURCE_DIR}/examples/spectral-flux.in
  flux "[inf,1000]"
  flux.cpp
)
target_compile_definitions(flux-stream PRIVATE STREAMING)
