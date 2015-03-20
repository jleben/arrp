# Library

if(WIN32)
  set(lib_mode STATIC)
else()
  set(lib_mode SHARED)
endif()

add_library(streamc-lib ${lib_mode}
  ../utility/debug.cpp
  ../utility/cpp-gen.cpp
  ../common/primitives.cpp
  ../frontend/lex.cc
  ../frontend/parse.cc
  ../frontend/environment.cpp
  ../frontend/types.cpp
  ../frontend/type_checker.cpp
  ../polyhedral/model.cpp
  ../polyhedral/translator.cpp
  ../polyhedral/dataflow_model.cpp
  ../polyhedral/printer.cpp
  ../polyhedral/ast_generator.cpp
  ../polyhedral/llvm_ir_from_cloog.cpp
  ../polyhedral/llvm_from_model.cpp
  ../interface/cpp-intf-gen.cpp
  compiler.cpp
)

add_dependencies(streamc-lib parser)

set_property( TARGET streamc-lib PROPERTY OUTPUT_NAME streamc )

if(DEFINED LLVM_VERSION AND LLVM_VERSION VERSION_GREATER 3.4)
  message(STATUS "Auto-configuring LLVM dependency.")
  llvm_config(streamc-lib core)
else()
  message(STATUS "Manually configuring LLVM dependency.")
  target_link_libraries( streamc-lib ${LLVM_LINK_FLAGS} ${LLVM_LIBRARIES} )
endif()

target_link_libraries( streamc-lib ${ISL_LIBRARY} isl-cpp ${CLOOG_LIBRARY} json m )

# Executable

add_executable(streamc main.cpp)
target_link_libraries(streamc streamc-lib)
