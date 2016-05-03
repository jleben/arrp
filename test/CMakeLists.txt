function(stream_to_cpp source_file_name cpp_namespace options)

  set(source_file ${CMAKE_CURRENT_SOURCE_DIR}/${source_file_name})

  add_custom_command(OUTPUT ${cpp_namespace}.cpp
    COMMAND $<TARGET_FILE:streamc>
    ARGS ${source_file} --cpp ${cpp_namespace} ${options}
    DEPENDS ${source_file} streamc
  )

endfunction()

add_subdirectory(performance)
