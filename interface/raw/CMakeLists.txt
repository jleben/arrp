


add_custom_command(
    OUTPUT embedded-interface-template.cpp
    DEPENDS interface-template.txt embed_template.py
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/embed_template.py
        ${CMAKE_CURRENT_SOURCE_DIR}/interface-template.txt
        ${CMAKE_CURRENT_BINARY_DIR}/embedded-interface-template.cpp
    )

add_library(arrp-io-generic-lib
    generator.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/embedded-interface-template.cpp
    ../../utility/subprocess.cpp
)

#add_executable(arrp-io-raw generator.cpp ${CMAKE_CURRENT_BINARY_DIR}/embedded-interface-template.cpp)
#set_target_properties(arrp-io-raw PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

configure_file(interface.h ${CMAKE_BINARY_DIR}/include/arrp/generic_io/interface.h COPYONLY)
configure_file(main.cpp ${CMAKE_BINARY_DIR}/include/arrp/generic_io/main.cpp COPYONLY)

install(FILES interface.h main.cpp DESTINATION include/arrp/generic_io)
