

set(test_names
  text-stream
  text-stream-noinput
  text-scalar
  input-literal-scalar
  input-literal-array
  input-literal-stream
  binary-unbuffered-stream
  binary-unbuffered-stream-noinput
  binary-buffered-stream
  binary-buffered-stream-noinput
  explicit-format
  file-text
  file-binary
  file-binary-unbuffered
  file-text-binary-bool
  multi-input
  boolean-text-io
)

foreach(test_name ${test_names})
  set(name "exe.${test_name}")
  add_test(
    NAME "${name}"
    COMMAND
      /usr/bin/env python3
      "${CMAKE_SOURCE_DIR}/test/exe/executable_tests.py" "${test_name}"
  )
  set_property(TEST ${name}
    PROPERTY ENVIRONMENT
      CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}
      CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}
      CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      ARRP_INSTALL_DIR=${CMAKE_INSTALL_PREFIX}
  )
endforeach()
