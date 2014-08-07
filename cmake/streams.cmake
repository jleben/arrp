function(compile_stream_func out_file_var source_file options)
  get_filename_component(source_file_base ${source_file} NAME_WE)

  set(llvm_file ${source_file_base}.ll)

  add_custom_command(OUTPUT ${llvm_file}
    COMMAND $<TARGET_FILE:frontend>
    ARGS ${source_file} -o ${llvm_file} ${options}
    DEPENDS ${source_file} frontend
  )

  set(object_file ${source_file_base}.o)

  add_custom_command(OUTPUT ${object_file}
    COMMAND llc
    ARGS -filetype=obj -o ${object_file} ${llvm_file}
    DEPENDS ${llvm_file}
  )

  message("object file: ${object_file}")

  set(${out_file_var} ${object_file} PARENT_SCOPE)
endfunction()
