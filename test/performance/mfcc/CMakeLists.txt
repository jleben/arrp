 
add_perf_test(mfcc_perf mfcc.arrp mfcc_io.hpp "-lfftw3")
add_output_test(mfcc_debug mfcc.arrp mfcc_io.hpp "-lfftw3")
