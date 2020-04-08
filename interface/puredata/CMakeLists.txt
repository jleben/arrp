

add_library(arrp-io-pd-lib
    generate.cpp
)

install(DIRECTORY target/ DESTINATION include/arrp/puredata_io)
