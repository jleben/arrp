

add_stream_test(fm_radio_s fm_radio.in "--separate-loops" fm_radio_driver.cpp)

add_test(fm_radio_c fm_radio_ref.cpp)
