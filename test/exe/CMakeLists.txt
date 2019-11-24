

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
  file
  multi-input
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
      ARRP_HOME=${CMAKE_BINARY_DIR}
      CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}
      CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}
  )
endforeach()

