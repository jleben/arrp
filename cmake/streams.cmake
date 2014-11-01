function(compile_stream_func out_file_var source_file options)
  get_filename_component(source_file_base ${source_file} NAME_WE)

  set(llvm_file ${source_file_base}.ll)

  add_custom_command(OUTPUT ${llvm_file}
    COMMAND $<TARGET_FILE:streamc>
    ARGS ${source_file} -o ${llvm_file} ${options}
    DEPENDS ${source_file} frontend
  )

  set(object_file ${source_file_base}.o)

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
