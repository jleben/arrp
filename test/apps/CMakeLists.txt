add_custom_target(app_tests)

function(add_app_test name source)
  if (ARGN)
      list(GET ARGN 0 compile_opt)
      list(GET ARGN 1 run_opt)
  endif()

  add_output_test(app.${name} "${CMAKE_CURRENT_SOURCE_DIR}/${source}" "${compile_opt}" "${run_opt}")
endfunction()

#add_subdirectory(fm_radio)
# FIXME: autocorrelation: Broken scheduling (infinite statements not merged).
#add_subdirectory(autocorrelation)
add_subdirectory(upsample)
add_subdirectory(lp)
add_subdirectory(wavetable_osc)
#add_subdirectory(fft)
#add_subdirectory(mfcc)
add_subdirectory(arg_max)
