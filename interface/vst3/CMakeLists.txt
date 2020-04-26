

add_library(arrp-io-vst3-lib
    generate.cpp
)

install(FILES factory_template.cpp processor.h DESTINATION include/arrp/vst3)
