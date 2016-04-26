
add_executable(fm_radio_c
  streamit/fm_radio_streamit.c
)

target_link_libraries(fm_radio_c m)

stream_to_cpp(fm_radio.in fm_radio "")

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(fm_radio
  #${CMAKE_CURRENT_BINARY_DIR}/fm_radio.cpp
  fm_radio_driver.cpp
)

set_source_files_properties(fm_radio_driver.cpp PROPERTIES OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/fm_radio.cpp")
#add_dependencies(fm_radio ${CMAKE_CURRENT_BINARY_DIR}/fm_radio.cpp)
