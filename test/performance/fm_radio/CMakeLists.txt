

add_stream_test(fm_radio_s fm_radio.in "" fm_radio_driver.cpp)

add_test(fm_radio_c streamit/fm_radio_streamit.c)
target_link_libraries(fm_radio_c m)
