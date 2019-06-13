# For parser
include_directories(../common)

add_library(arrp-lib STATIC
  ../utility/debug.cpp
  ../utility/cpp-gen.cpp
  ../common/primitives.cpp
  ../common/func_types.cpp
  ../common/func_model_printer.cpp
  ../common/ph_model.cpp
  ../common/module.cpp
  ../frontend/error.cpp
  ../frontend/module_parser.cpp
  ../frontend/prim_reduction.cpp
  ../frontend/functional_gen.cpp
  ../frontend/scope_cleanup.cpp
  ../frontend/func_copy.cpp
  ../frontend/reference_analysis.cpp
  ../frontend/func_reduction.cpp
  ../frontend/type_check.cpp
  ../frontend/linear_expr_gen.cpp
  ../frontend/array_to_isl.cpp
  ../frontend/array_bounding.cpp
  ../frontend/array_reduction.cpp
  ../frontend/array_inflate.cpp
  ../frontend/array_transpose.cpp
  ../frontend/ph_model_gen.cpp
  ../polyhedral/utility.cpp
  ../polyhedral/scheduling.cpp
  ../polyhedral/storage_alloc.cpp
  #../polyhedral/modulo_avoidance.cpp
  ../polyhedral/isl_ast_gen.cpp
  ../cpp/cpp_target.cpp
  ../cpp/cpp_from_polyhedral.cpp
  ../cpp/cpp_from_isl.cpp
  ../cpp/name_mapper.cpp
  ../cpp/collect_names.cpp
  arg_parser.cpp
  report.cpp
  compiler.cpp
)

target_link_libraries(arrp-lib parser)

set_property( TARGET arrp-lib PROPERTY OUTPUT_NAME arrp )

if(FALSE)
if(DEFINED LLVM_VERSION AND LLVM_VERSION VERSION_GREATER 3.4)
  message(STATUS "Auto-configuring LLVM dependency.")
  llvm_config(arrp-lib core)
else()
  message(STATUS "Manually configuring LLVM dependency.")
  target_link_libraries( arrp-lib ${LLVM_LINK_FLAGS} ${LLVM_LIBRARIES} )
endif()
endif()

# NOTE: gmp needed by isl
target_link_libraries( arrp-lib ${ISL_LIBRARY} isl-cpp gmp m)

# Executable

add_executable(arrp main.cpp)
target_link_libraries(arrp arrp-lib)

# Installation

install(TARGETS arrp
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(FILES ${CMAKE_SOURCE_DIR}/cpp/arrp.hpp DESTINATION include)
