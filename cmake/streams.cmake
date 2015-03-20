function(stream_kernel_opt out in)
  add_custom_command(OUTPUT ${out}
    COMMAND opt
    ARGS -S -scalarrepl -std-compile-opts ${in} -o ${out}
    DEPENDS ${in}
  )
endfunction()

function(stream_kernel_object out in)
  if(CMAKE_CL_64)
    set(arch -march=x86-64)
  endif()

  add_custom_command(OUTPUT ${out}
    COMMAND ${llc_program}
    ARGS ${arch} -filetype=obj -o ${out} ${in} $<$<CONFIG:Release>:-O=3>
    DEPENDS ${in}
  )
endfunction()

function(compile_stream_func target_name source_file sym_name args options)

  set(llvm_file ${target_name}.ll)

  add_custom_command(OUTPUT ${llvm_file}
    COMMAND $<TARGET_FILE:streamc>
    ARGS ${source_file} -g ${sym_name} ${args} -o ${llvm_file} ${options}
    DEPENDS ${source_file} streamc
  )

  set(llvm_opt_file ${target_name}-opt.ll)
  add_custom_command(OUTPUT ${llvm_opt_file}
    COMMAND opt
    ARGS -S -scalarrepl -std-compile-opts ${llvm_file} -o ${llvm_opt_file}
    DEPENDS ${llvm_file}
  )

  set(object_file ${target_name}.o)

  if(CMAKE_CL_64)
    set(arch -march=x86-64)
  endif()

  add_custom_command(OUTPUT ${object_file}
    COMMAND ${llc_program}
    ARGS ${arch} -filetype=obj -o ${object_file} ${llvm_opt_file} $<$<CONFIG:Release>:-O=3>
    DEPENDS ${llvm_opt_file}
  )

  set(${target_name}_object ${object_file} PARENT_SCOPE)

endfunction()
