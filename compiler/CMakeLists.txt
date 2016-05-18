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
  ../common/func_types.cpp
  ../common/func_model_printer.cpp
  ../common/ph_model.cpp
  ../common/module.cpp
  ../frontend/module_parser.cpp
  ../frontend/prim_reduction.cpp
  ../frontend/functional_gen.cpp
  ../frontend/func_copy.cpp
  ../frontend/func_reducer.cpp
  ../frontend/linear_expr_gen.cpp
  ../frontend/array_reduction.cpp
  ../frontend/array_transpose.cpp
  ../frontend/type_check.cpp
  ../frontend/ph_model_gen.cpp
  ../polyhedral/scheduling.cpp
  ../polyhedral/storage_alloc.cpp
  ../polyhedral/modulo_avoidance.cpp
  ../polyhedral/isl_ast_gen.cpp
  ../cpp/cpp_target.cpp
  ../cpp/cpp_from_polyhedral.cpp
  ../cpp/cpp_from_isl.cpp
  ../cpp/name_mapper.cpp
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

target_link_libraries( streamc-lib ${ISL_LIBRARY} isl-cpp json m )

# Executable

add_executable(streamc main.cpp)
target_link_libraries(streamc streamc-lib)
