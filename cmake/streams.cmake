function(compile_stream_func out_file_var source_file name args options)
  #get_filename_component(source_file_base ${source_file} NAME_WE)

  set(llvm_file ${name}.ll)

  add_custom_command(OUTPUT ${llvm_file}
    COMMAND $<TARGET_FILE:streamc>
    ARGS ${source_file} -g ${name} ${args} -o ${llvm_file} ${options}
    DEPENDS ${source_file} streamc
  )

  set(object_file ${name}.o)

  if(CMAKE_CL_64)
    set(arch -march=x86-64)
  endif()

  add_custom_command(OUTPUT ${object_file}
    COMMAND ${llc_program}
    ARGS ${arch} -filetype=obj -o ${object_file} ${llvm_file}
    DEPENDS ${llvm_file}
  )

  set(${out_file_var} ${object_file} PARENT_SCOPE)
endfunction()
