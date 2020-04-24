
function(add_lib_test name source compile_opt run_opt)
  add_output_test(${name} "${CMAKE_CURRENT_SOURCE_DIR}/${source}" "${compile_opt}" "${run_opt}")
endfunction()

add_lib_test(lib.fir fir.arrp "" "")
add_lib_test(lib.iir iir.arrp "" "")
add_lib_test(lib.signal.phase signal.phase.arrp "" "")
add_lib_test(lib.signal.sine signal.sine.arrp "" "")
add_lib_test(lib.signal.triangle signal.triangle.arrp "" "")
add_lib_test(lib.signal.square signal.square.arrp "" "")
add_lib_test(lib.sum-1d sum-1d.arrp "" "")
add_lib_test(lib.sum-md sum-md.arrp "" "")
# FIXME: Shows a problem with automatic stream transposition:
#add_lib_test(lib.sum-stream sum-stream.arrp "" "")
