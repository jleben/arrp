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

# Discover isl

find_package(isl REQUIRED)
message(STATUS "isl include dir: ${ISL_INCLUDE_DIR}")
message(STATUS "isl lib: ${ISL_LIBRARY}")

# Discover LLVM dependencies

if(WIN32)

  find_package(llvm REQUIRED)
  include_directories(${LLVM_INCLUDE_DIRS})
  link_directories(${LLVM_LIBRARY_DIRS})

else()

#if (NOT DEFINED llvm-compile-flags)
  execute_process(
    COMMAND llvm-config --cxxflags
    OUTPUT_VARIABLE llvm-compile-flags
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
#endif()

#if (NOT DEFINED llvm-link-flags)
  execute_process(
    COMMAND llvm-config --ldflags
    OUTPUT_VARIABLE llvm-link-flags
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  execute_process(
    COMMAND llvm-config --libs core
    OUTPUT_VARIABLE llvm-link-libs
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
#endif()

endif() # Windowz

find_program(llc_program llc HINTS ${LLVM_TOOLS_BINARY_DIR})

# Dependencies

include_directories(
  extra   # for isl-cpp
  extra/isl/include
  extra/cloog/include
)

link_directories(
  ${CMAKE_SOURCE_DIR}/extra/isl/lib
  ${CMAKE_SOURCE_DIR}/extra/cloog/lib
)

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
