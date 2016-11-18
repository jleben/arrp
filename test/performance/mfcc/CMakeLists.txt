 
add_perf_test(mfcc_perf mfcc.arrp mfcc_io.hpp "-lfftw3")

add_perf_test(mfcc_perf_ref mfcc_ref.cpp "" "-lfftw3")
