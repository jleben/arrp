
add_library(arrp-io-jack-lib
    generator.cpp
)

install(DIRECTORY target/ DESTINATION include/arrp/jack_io)
