cmake_minimum_required(VERSION 2.8)

project(stream-lang)

enable_testing()

# Options

option(BUILD_TESTING "Build tests" ON)
set(TEST_BUILD_TYPE Release CACHE STRING "Build type for tests (affects performance evaluations).")

# CMake setup

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 11)

if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wno-sign-compare")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w44267 /w44244")
endif()

if(PARSER_REGENERATE)
  find_package(Flex REQUIRED)
endif()

# For FlexLexer.h:
include_directories(frontend)

# Prepare isl

find_package(isl REQUIRED)
include_directories(${ISL_INCLUDE_DIR})

# Prepare CLooG

#find_package(cloog REQUIRED)
#add_definitions(-DCLOOG_INT_GMP)
#include_directories(${CLOOG_INCLUDE_DIR})

# Prepare LLVM

if(FALSE)

find_package(llvm-internal REQUIRED)

find_program(llc_program llc HINTS ${LLVM_TOOLS_BINARY_DIR})

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

endif()

# Prepare isl-cpp

include_directories(extra)

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
