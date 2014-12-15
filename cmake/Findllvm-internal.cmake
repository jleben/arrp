
find_package(LLVM QUIET CONFIG)

if(LLVM_FOUND)
  message(STATUS "Found LLVM version: ${LLVM_VERSION}")
  return()
endif()

find_program(llvm_config_program llvm-config)

if (NOT llvm_config_program)
  message(FATAL_ERROR "Could not find the \"llvm-config\" program.")
endif()

function(llvm_find what variable description)
  if(DEFINED ${variable})
    return()
  endif()

  execute_process(
    COMMAND ${llvm_config_program} ${what}
    OUTPUT_VARIABLE ${variable}
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(${variable} ${${variable}} CACHE STRING ${description})
  mark_as_advanced(${variable})
  message("${variable} = ${${variable}}")
endfunction()

llvm_find(--version LLVM_VERSION "LLVM version.")
llvm_find(--includedir LLVM_INCLUDE_DIRS "LLVM include directories.")
llvm_find(--cxxflags LLVM_COMPILE_FLAGS "LLVM compile flags.")
llvm_find(--libdir LLVM_LINK_DIRS "LLVM link directories.")
llvm_find(--ldflags LLVM_LINK_FLAGS "LLVM link flags.")
llvm_find("--libs;core" LLVM_LIBRARIES "LLVM libraries.")
