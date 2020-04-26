add_library(arrp-meta-json STATIC meta-json-parser.cpp)
target_link_libraries(arrp-meta-json json)

add_subdirectory(raw)
add_subdirectory(jack)
add_subdirectory(puredata)
add_subdirectory(vst3)

install(FILES linear_buffer.h DESTINATION include/arrp)
