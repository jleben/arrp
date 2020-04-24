
function(add_lib_test name source compile_opt run_opt)
  add_output_test(${name} "${CMAKE_CURRENT_SOURCE_DIR}/${source}" "${compile_opt}" "${run_opt}")
endfunction()

add_lib_test(lib.fir fir.arrp "" "")
add_lib_test(lib.iir iir.arrp "" "")
add_lib_test(lib.sine sine.arrp "" "")
