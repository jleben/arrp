add_custom_target(app_tests)

function(add_app_test name source)
  set(test_name "app.${name}")
  add_output_test(${test_name} ${source} ${ARGN})
  add_dependencies(app_tests test_${test_name})
endfunction()

#add_subdirectory(fm_radio)
# FIXME: autocorrelation: Broken scheduling (infinite statements not merged).
#add_subdirectory(autocorrelation)
add_subdirectory(upsample)
add_subdirectory(lp)
add_subdirectory(fir)
add_subdirectory(osc)
add_subdirectory(wavetable_osc)
add_subdirectory(fft)
add_subdirectory(mfcc)
