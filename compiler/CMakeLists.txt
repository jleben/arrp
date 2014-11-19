# Library

if(WIN32)
  set(lib_mode STATIC)
else()
  set(lib_mode SHARED)
endif()

add_library(streamc-lib ${lib_mode}
  ../frontend/lex.cc
  ../frontend/parse.cc
  ../frontend/environment.cpp
  ../frontend/types.cpp
  ../frontend/type_checker.cpp
  #../frontend/ir-generator.cpp
  ../polyhedral/model.cpp
  ../polyhedral/translator.cpp
  ../polyhedral/dataflow_model.cpp
  ../polyhedral/printer.cpp
  ../polyhedral/ast_generator.cpp
  #../polyhedral/llvm_ir_generator.cpp
  ../polyhedral/llvm_ir_from_cloog.cpp
  ../polyhedral/llvm_from_model.cpp
)

set_property( TARGET streamc-lib PROPERTY OUTPUT_NAME streamc )

#set_property( TARGET streamc-lib APPEND PROPERTY COMPILE_FLAGS ${llvm-compile-flags} )

if(WIN32)
  llvm_config(streamc-lib core)
else()
  target_link_libraries( streamc-lib ${llvm-link-libs} ${llvm-link-flags}  )
endif()

target_link_libraries( streamc-lib isl isl-cpp cloog-isl )

# Executable

add_executable(streamc compiler.cpp)
target_link_libraries(streamc streamc-lib)
