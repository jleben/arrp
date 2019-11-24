
set(run_opt "coefs_a=\"1 0.5\" coefs_b=\"1 0.5 0.33333333333333333333\"")
add_app_test(biquad biquad.arrp "" "${run_opt}")
