cmake_minimum_required(VERSION 2.8)

project(stream-lang)

enable_testing()

# Options

option(BUILD_TESTING "Build tests" ON)

# CMake setup

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w44267 /w44244")
endif()

# Discover isl and cloog

find_package(isl REQUIRED)
find_package(cloog REQUIRED)

# Discover LLVM dependencies

find_package(llvm REQUIRED)
find_program(llc_program llc HINTS ${LLVM_TOOLS_BINARY_DIR})

# Dependencies

include_directories(
  extra   # for isl-cpp
  ${ISL_INCLUDE_DIR}
  ${CLOOG_INCLUDE_DIR}
  ${LLVM_INCLUDE_DIRS}
)

link_directories(${LLVM_LIBRARY_DIRS})

# Global compile options

add_definitions(-DCLOOG_INT_GMP)

# Subdirs

add_subdirectory(extra/isl-cpp)
add_subdirectory(extra/json++)
add_subdirectory(frontend)
add_subdirectory(utility)
add_subdirectory(compiler)
add_subdirectory(interface)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
