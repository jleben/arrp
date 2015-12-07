# Library

if(WIN32)
  set(lib_mode STATIC)
else()
  set(lib_mode SHARED)
endif()

# For parser
include_directories(../common)

add_library(streamc-lib ${lib_mode}
  ../utility/debug.cpp
  ../utility/cpp-gen.cpp
  ../common/primitives.cpp
  ../common/types.cpp
  ../common/environment.cpp
  #../common/polyhedral_model.cpp
  ../common/polyhedral_model_printer.cpp
  ../common/array_func_printer.cpp
  ../common/func_model_printer.cpp
  ../frontend/prim_reduction.cpp
  ../frontend/functional_gen.cpp
  ../frontend/func_reducer.cpp
  ../frontend/linear_expr_gen.cpp
  ../frontend/array_reduction.cpp
  ../frontend/ph_model_gen.cpp
  #../polyhedral/polyhedral-gen.cpp
  #../polyhedral/translator.cpp
  #../polyhedral/dataflow_model.cpp
  ../polyhedral/ast_generator.cpp
  ../llvm/llvm_ir_from_cloog.cpp
  #../llvm/llvm_from_polyhedral.cpp
  ../cpp/cpp_target.cpp
  ../cpp/cpp_from_cloog.cpp
  ../cpp/cpp_from_polyhedral.cpp
  #../interface/cpp-intf-gen.cpp
  compiler.cpp
)

target_link_libraries(streamc-lib parser)

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
