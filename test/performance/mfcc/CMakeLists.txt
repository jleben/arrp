 
add_perf_test(mfcc_perf mfcc.arrp mfcc_io.hpp "-lfftw3")
add_output_test(mfcc_perf_debug mfcc.arrp mfcc_io.hpp "-lfftw3")

add_perf_test(mfcc_perf_ref mfcc_ref.cpp "" "-lfftw3")
add_output_test(mfcc_perf_ref_debug mfcc_ref.cpp "" "-lfftw3")
