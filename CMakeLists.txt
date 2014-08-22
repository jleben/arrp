cmake_minimum_required(VERSION 2.8)

project(stream-lang)

enable_testing()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

add_subdirectory(frontend)
add_subdirectory(utility)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
