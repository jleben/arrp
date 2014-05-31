## Prerequisits

This project requires flexc++ and bisonc++ lexer and parser generators.
Information about these two programs is available online:
- http://flexcpp.sourceforge.net/
- http://bisoncpp.sourceforge.net/

## Building

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

## Filesystem:

- `frontend` - Contains code for the frontend (lexer + parser).
  - `scanner.l` - Input file for lexer generator flexc++
  - `parser.y` - Input file for parser generator bisonc++
  - `test.cpp` - Implement an executable parser which depends on output of flexc++ and bisonc++.

- `examples` - Contains example code in the language implemented by this project.
  - `matrix-mult.in` - Implements multiplication of two sequences of matrices.
  - `spectral-flux.in` - Implements "spectral flux", given a sequence of spectrums (results of DFT).
