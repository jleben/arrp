
set(ARRP_EXECUTABLE ${CMAKE_CURRENT_LIST_DIR}/../../../bin/arrp)
set(ARRP_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../include/)

# FIXME: Find jack library

function(arrp_to_jack name arrp_source)

  set(work_dir "${CMAKE_CURRENT_BINARY_DIR}/${name}.dir")
  file(MAKE_DIRECTORY ${work_dir})

  set(kernel_h ${work_dir}/${name}.h)
  set(jack_client_cpp ${work_dir}/${name}-jack-client.cpp)
  set(executable ${name})

  add_custom_command(
    OUTPUT
      ${kernel_h}
      ${jack_client_cpp}
    DEPENDS
      ${arrp_source}
    COMMAND ${ARRP_EXECUTABLE}
    ARGS
      ${CMAKE_CURRENT_SOURCE_DIR}/${arrp_source}
      --io-common-clock
      --interface jack
      --output ${name}
      --jack-name ${name}
    WORKING_DIRECTORY ${work_dir}
  )

  add_custom_target(${name}-arrp-outputs DEPENDS ${jack_client_cpp})

  add_executable(
    ${executable}
    ${jack_client_cpp}
  )

  target_include_directories(${executable} PRIVATE ${ARRP_INCLUDE_DIR})

  target_link_libraries(${executable} jack)

  add_dependencies(${executable} ${name}-arrp-outputs)

endfunction()


function(arrp_to_exe name arrp_source)

  set(work_dir "${CMAKE_CURRENT_BINARY_DIR}/${name}.dir")

  file(MAKE_DIRECTORY ${work_dir})

  set(main_cpp ${work_dir}/${name}-stdio-main.cpp)

  add_custom_command(
    OUTPUT
      ${main_cpp}
    DEPENDS
      ${arrp_source}
    COMMAND ${ARRP_EXECUTABLE}
    ARGS
      ${CMAKE_CURRENT_SOURCE_DIR}/${arrp_source}
      --interface stdio
      --output ${name}
    WORKING_DIRECTORY ${work_dir}
  )

  add_custom_target(${name}-arrp-outputs DEPENDS ${main_cpp})

  add_executable(
    ${name}
    ${main_cpp}
  )

  target_include_directories(${name} PRIVATE ${work_dir} ${ARRP_INCLUDE_DIR})

  add_dependencies(${name} ${name}-arrp-outputs)
endfunction()

function(arrp_to_pd name arrp_source)

  set(work_dir "${CMAKE_CURRENT_BINARY_DIR}/${name}.dir")
  file(MAKE_DIRECTORY ${work_dir})

  set(kernel_h ${work_dir}/${name}.h)
  set(pd_intf_cpp ${work_dir}/${name}-pd-interface.cpp)

  add_custom_command(
    OUTPUT
      ${pd_intf_cpp}
    DEPENDS
      ${arrp_source}
    COMMAND ${ARRP_EXECUTABLE}
    ARGS
      ${CMAKE_CURRENT_SOURCE_DIR}/${arrp_source}
      --io-common-clock
      --interface puredata
      --pd-name ${name}
      --output ${name}
    WORKING_DIRECTORY ${work_dir}
  )

  add_custom_target(${name}-arrp-outputs DEPENDS ${pd_intf_cpp})

  add_library(${name} SHARED
    ${ARRP_INCLUDE_DIR}/arrp/puredata_io/entry.cpp
    ${pd_intf_cpp}
  )

  target_include_directories(${name} PRIVATE ${ARRP_INCLUDE_DIR})

  target_link_libraries(${name} pcl)

  set_target_properties(${name} PROPERTIES
    OUTPUT_NAME "${name}~"
    PREFIX ""
  )

  if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    set_target_properties(${name} PROPERTIES SUFFIX ".pd_linux")
  elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set_target_properties(${name} PROPERTIES SUFFIX ".pd_darwin")
  endif()

  add_dependencies(${name} ${name}-arrp-outputs)

endfunction()
