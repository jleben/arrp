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


# Discover Pluto

set(pluto_dir "~/code/pluto-0.11.0")
#set(pluto_dir "~/code/pluto")

include_directories(
  "${pluto_dir}/include"
  "${pluto_dir}/isl/include"
  "${pluto_dir}/openscop/include"
  "${pluto_dir}/cloog-isl/include"
)
link_directories(
  "${pluto_dir}/src/.libs"
  "${pluto_dir}/isl/.libs"
  "${pluto_dir}/cloog-isl/.libs"
)

# In-source dependencies

include_directories(extra)

# Subdirs

add_subdirectory(extra/isl-cpp)
add_subdirectory(frontend)
add_subdirectory(utility)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
