#add_executable(test-visitor EXCLUDE_FROM_ALL test-visitor.cpp)
#add_executable(test-dispatch EXCLUDE_FROM_ALL test-dispatch.cpp)
#add_executable(test-introspec EXCLUDE_FROM_ALL test-introspectible.cpp)


add_library(arrp-utils STATIC
  debug.cpp
  cpp-gen.cpp
  filesystem.cpp
  subprocess.cpp
  find_files.cpp
)
