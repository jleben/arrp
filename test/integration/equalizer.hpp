#ifndef STREAM_TEST_EQUALIZER_INCLUDED
#define STREAM_TEST_EQUALIZER_INCLUDED

#include <vector>

namespace equalizer {

using std::vector;

static vector<double> low_pass_coef = {
    0.74779, 0.27221, // a
    0.50500, 1.01000, 0.50500 // b
};

static vector<double> high_pass_coef ={
    -1.56102,  0.64135, // a
    0.80059, -1.6011, 0.80059 // b
};

static  vector<double> band_stop_coef = {
    -0.27346, 0.72654, // a
    0.86327, -0.27346, 0.86327 // b
};

}

#endif
