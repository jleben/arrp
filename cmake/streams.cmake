function(compile_stream_func target_name source_file sym_name args options)

  set(llvm_file ${target_name}.ll)

  add_custom_command(OUTPUT ${llvm_file}
    COMMAND $<TARGET_FILE:streamc>
    ARGS ${source_file} -g ${sym_name} ${args} -o ${llvm_file} ${options}
    DEPENDS ${source_file} streamc
  )

  set(object_file ${target_name}.o)

  if(CMAKE_CL_64)
    set(arch -march=x86-64)
  endif()

  add_custom_command(OUTPUT ${object_file}
    COMMAND ${llc_program}
    ARGS ${arch} -filetype=obj -o ${object_file} ${llvm_file} $<$<CONFIG:Release>:-O=3>
    DEPENDS ${llvm_file}
  )

  set(${target_name}_object ${object_file} PARENT_SCOPE)

endfunction()
