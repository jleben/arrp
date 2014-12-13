
if(WIN32)

  if (llvm_FIND_REQUIRED)
    find_package(llvm REQUIRED)
  else()
    find_package(llvm)
  endif()

else()

function(llvm_find what variable description)
  if(DEFINED ${variable})
    return()
  endif()

  execute_process(
    COMMAND llvm-config ${what}
    OUTPUT_VARIABLE ${variable}
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(${variable} ${${variable}} CACHE STRING ${description})
  mark_as_advanced(${variable})
  message("${variable} = ${${variable}}")
endfunction()

llvm_find(--includedir LLVM_INCLUDE_DIRS "LLVM include directories.")
llvm_find(--cxxflags LLVM_COMPILE_FLAGS "LLVM compile flags.")
llvm_find(--libdir LLVM_LINK_DIRS "LLVM link directories.")
llvm_find(--ldflags LLVM_LINK_FLAGS "LLVM link flags.")
llvm_find("--libs;core" LLVM_LIBRARIES "LLVM libraries.")

endif() # NOT WIN32
