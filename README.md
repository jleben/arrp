This project provides a CMake build system.

The standard building procedure is:
```
mkdir build
cd build
cmake ..
make
```

This will produce the following executables:
- `build/frontend/test` - Test the frontend (lexer + parser) on an input file.

CMake options:
- `FRONTEND_PRINT_TOKENS` - Compile frontend so that it prints all tokens on command-line.
