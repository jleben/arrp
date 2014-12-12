## Introduction

This project implements a compiler for a new programming language
for stream processing.

This document provides instructions for building the project, as well
as the syntactical and semantical specification of the language.

### Author

Jakob Leben

### Online Repository

The entire project code is available online at:
[https://github.com/jleben/stream-lang](https://github.com/jleben/stream-lang)

### Table of Contents:

- [Building](#building)
- [Usage](#usage)
- [Example Usage](#example-usage)
- [Filesystem](#filesystem)
- [Language Specification](#the-language)


## Building

### Prerequisits

**NOTE:** The project requires a compiler with a good support for C++11.

#### Submodules

This project uses git submodules; make sure to get/update all of them:

    git submodule init
    git submodule update

#### isl (Integer Set Library)

1. Get git repository: https://github.com/jleben/isl.
2. Use branch: **topic/constraint-map**.
3. Build and install to `extra/isl` within this project's source.

Project website: [http://isl.gforge.inria.fr](http://isl.gforge.inria.fr/)

#### CLooG (Polyhedral Code Generator)

**NOTE:** CLooG itself depends on *isl*, so you have to configure its
build process to use the *isl* that you have just built and installed as
described above.

1. Get git repository: http://repo.or.cz/r/cloog.git
2. Use branch: **master**
3. Build using the installation of *isl* as described above.
4. Install to `extra/cloog` within this project's source.

Project website: [http://www.bastoul.net/cloog](http://www.bastoul.net/cloog)

#### Optional

Optionally, you may want to re-generate lexer and parser.
This requires the flexc++ lexer generator and the bisonc++ parser generator.
Information about these two programs is available online:

- http://flexcpp.sourceforge.net/
- http://bisoncpp.sourceforge.net/

### Building This Project

This project has a CMake build system.

After all the prerequisits are in place as described above, you can
build this project using the standard building procedure
on LINUX and Mac OS X:

    mkdir build
    cd build
    cmake ..
    make

This will produce the compiler executable:

    build/compiler/streamc

**NOTE:** You may need to adjust compiler and linker flags to enable C++11
support for your compiler. Please refer to the documentation of CMake and your
compiler for instructions.

### Options

The CMake build system provides the following options:

- `PARSER_REGENERATE` - Regenerate parser using flexc++ and bisonc++.
- `PARSER_PRINT_TOKENS` - Compile parser so as to enabled the option to print all parsed tokens.
- `BISON_VERBOSE` - Let bisonc++ produce detailed parser information in the file `frontend/parser.y.output`.
- `BUILD_TESTING` - Build tests.

When the project has been built with the option `BUILD_TESTING` enabled,
the tests can be run using the command:

    make test

## Usage

The building procedure desrcibed above will produce the language compiler
frontend executable: `build/compiler/streamc`.

The frontend takes a file with code in The Language as input and produces
a file with intermediate representation in form of the LLVM IR as output,
for a selected function or constant symbol.

Invoking the compiler with the `-h` option will print information about
its usage and available options:

    streamc -h

Simply invoking the compiler with an input file will perform syntactical
analysis and limited semantical analysis of input code, reporting any errors,
but otherwise not producing any output:

    streamc <input file>

LLVM IR is generated for a selected function or constant
symbol using the `-g` option. For function symbols, argument types must
be provided:

    streamc <input file> -g <symbol name> [<arg type> ...]

In addition, a C++ header file providing an interface to the produced
code can be generated usign the `--cpp` option:

    streamc <input file> -g <symbol name> [<arg type> ...] --cpp <header file>

The generated intermediate code can be compiled into an object file
(for example using the `llc` program) and linked with a hosting C or C++
program for execution.

### Example

Example code files are provided in the `examples` folder.

The following command will generate LLVM IR code and a C++ header for the
`matrix_multiply` function defined in the `matrix-multiply.in` example file:

    streamc matrix-mult.in -g matrix_multiply "[5,2,3]" "[5,3,2]" -o mm.ll --cpp matrix-mult.h

The function `matrix_multiply` multiplies two sequences of matrices, producing
a new sequence of matrix products. Hence, the above command will report
that the result type of the generated function is `[5,2,2]`, meaning that
the result is a stream of 5 matrices, each with as many rows as the
first matrix and as many columns as the second matrix given as arguments.

This will produce intermediate code in the file `mm.ll`. This file can further
be compiled into an object file for the target machine using the `llc` tool
provided by the LLVM project:

    llc -filetype=obj mm.ll -o mm.o

This will produce the object file `mm.o`, which can be linked with the hosting
C++ program to test the generated code for matrix multiplication provided in
`test/integration/`:

    g++ -std=c++11 -I. mm.o test-matrix-mult.cpp -o mm

This will produce the final executable `mm`. It executes the
matrix multiplication function once and than compares the result with
equal computation implemented independently in C++, reporting any discrepancy.

## The Language

The Language is targetting specifically stream processing applications.
A *stream* is represented in the language as a multidimensional array.
The following features allow stream processing to be expressed and executed
efficiently:

- The language is functional.
- Typing: strong, static, implicit
- Expression structures:
    * mapping (application of expression on consecutive ranges of streams)
    * reduction (reduction of streams to a single value by application of
      expression on individual stream values and results of previous applications)

### Type System

Typing is:

- Strong: A type of a value can never be changed.
- Static: Type checking is done completely at compile time.
- Implicit: See below.

Fully implicit typing means:

- Types do not appear in the syntax of the language.
- The result type of an expression is inferred from the types of its constituents.
- All functions are generic: a function definition defines a class of functions
  with all possible type assignments to its parameters that conform to typing
  rules.
- The result type of a function call is the result type of the
  function with parameter types matching types of function call arguments.

The Language has the following types:

- `int`: Integer number. Current implementation uses 32 bit integers to represent values of this type.
- `real`: Real number. Current implementation uses 64 bit floating points to represent values of this type.
- `range[<start>,<end>]`: Sequence of integer numbers defined by a pair of start and end numbers.
- `stream[<x>,<y>,<z>...]`:
  Multi-dimensional sequence of real numbers.
  Each number in brackets defines size in a dimension.
  A stream may also be infinite in one dimension.


### Syntax and Semantics

#### Module

Syntax:

    <module> = <statment list> [ ";" ]
    <statement list> = <statement> ( ";" <statement> )*

An input file represents a module, which consists of a list of statements.

Example:

    a = 3;
    b = a + 5

#### Statement

Syntax:

    <statemenet> = <id> [ <parameter list> ] "=" <statment body>
    <parameter list> = "(" <id> ( "," <id> )* ")"
    <statement body> = <complex expr> | <block>

A statement binds a name with a value of an expression or with a function.
The scope of the bound name is the code following the statment in the
`<module>` or the `<block>` in which the statement appears.

If the optional parameter list is empty, then the name represents the
resulting value of the statement body and can be used alone as an
expression.

It the parameter list is not empty, the name represents a function and
can be used in a function call. In this case the statement is also called
a *function definition*. Identifiers in the paremeter list are
names of function parameters; their scope is the statement body.

Examples:

- bound expression: `a = 10 + b * 3`
- function definition: `f(x) = x * 3`

#### Block

Syntax:

    <block> = "{" [ <let list> ";" ] <complex expr> "}"
    <let list> = <let stmt> ( ";" <let stmt> )*
    <let stmt> = "let" <statement> | "let" "{" <statement list> "}"

A block may contain a number of statements, each binding a name of which scope
spans from the statement to the end of the block.

The block must end in exactly one expression. The value of the entire block
is the value of that expression.

Example:

    f(x) = {
        let { a = x + 10; b = x + 5 };
        let c = a * b;
        c * c
    }

#### Expression

Syntax:

    <expr> =
        <id>
      | <number>
      | <range>
      | <binary operation>
      | <transpose>
      | <slice>
      | <extent>
      | <call>

    <complex expr> =
        <expr>
      | <mapping>
      | <reduction>

The type of an expression is inferred from the types of its constituents
and the expression type.

#### Identifier

Syntax:

    <id> = [A-Za-z_][A-Za-z0-9_]*

When an indentifier is used as an expression, it represents the value of an
expression bound to the identifier by a statement elsewhere.

#### Number

Syntax:

    <number> = <int> | <real>
    <int> = [0-9]+
    <real> = [0-9]+\.[0-9]+

A `<number>` represents a constant number with a value literally defined by its text.

An `<int>` represents an integer number, and its type is int, for example `123`.

A `<real>` represents a real number, and its type is real, for example `123.456`.

#### Range

Syntax:

    <range> = <s:expr> .. <e:expr>

A `<range>` represents a value of type `range[<s>,<e>]`. The types of `<s>`
and `<e>` must both be int.

Example:

    a = 1..10 * 5; // A range between 1 and 10 multiplied by 5.

#### Binary Operation

Syntax:

    <binary operation> = <lhs:expr> <binary operator> <rhs:expr>
    <binary operator> = "+" | "-" | "*" | "/" | ":" | "^"

The type of result value depends on the types of operands `<lhs>` and `<rhs>`
as well as the type of operation.

This table provides type relations for scalar types:

 `<lhs>` | `<rhs>` | `+`  |  `-` | `*`  | `^`  | `/`  | `:`
:-------:|:-------:|:----:|:----:|:----:|:----:|:----:|:----:
 int     |  int    | int  | int  | int  | int  | real | int
 real    |  int    | real | real | real | real | real | int
 int     |  real   | real | real | real | real | real | int
 real    |  real   | real | real | real | real | real | int

Note that `/` is common division which always results in a real value,
whereas `:` is integer division, which always results in an integer.

An operation between a stream or range and a scalar is performed
once for each element in the stream or range as one operand, and
every time the same scalar as the second operand. The result is a
stream of the same size as the operand stream or range.

When both operands are streams or ranges, they must be of the same size.
The operation is performed for each pair of elements in one and the other
stream or range at an equal position.

Elements of a stream are of type `real` and
elements of a range are of type `int`.

#### Transposition

Syntax:

    <transpose> = <s:expr> "{" <d:int> ( "," <d:int> )* "}"

The type of `<s>` must be a stream. The result is a transposition of this
stream, i.e. a stream with the same elements, but changed order of dimensions.
The transposed dimension order is defined by the list of integers `<d>`,
each one specifying an index of one of the original dimensions. Elements of
the list must be unique. If the list does not contain all dimensions, the
remaining dimensions are added to the new order in their relative original
order.

Example:

If `<s>` is `stream[10,20,30]`, then `<s>{2,3}` is `stream[20,30,10]`

#### Slice

Syntax:

    <slice> = <s:expr> "[" <r:expr> ( "," <r:expr> )* "]"

The type of `<s>` must be a stream. The type of each `<r>` must be either
an integer or a range. The result is a stream with a selection of elements
from `<s>`: in each dimension, only elements with indices within the range `<r>`
or equal to integer `<r>` are retained, for `<r>` at position matching the
index of the dimension. If the number of `<r>` is less then the number of
dimensions of `<s>`, then all elements in the remaining dimensions are
retained. Any dimension of the resulting stream with size 1 is omitted.

Example:

If `<s>` is `stream[10,20,30]`, then `<s>[1..5, 8]` is `stream[5,30]`.
Note that the size in the first dimension is reduced to 5 because
the range of indexes 1 to 5 was selected; however, the second dimension is
omitted because only a single index 8 in that dimension was selected; the
last dimension thus becomes the second dimension, with original size.

#### Extent

    <extent> = "#" <s:expr>

The type of `<s>` must be a stream. The result is an integer of the size of
`<s>` in first dimension.

Example:

If `<s>` is `stream[10,5]`, then `#<s>` is the integer `10`.

#### Function Call

Syntax:

    <call> = <f:id> "(" <a:complex expr> ( "," <a:complex expr> )* ")"

A function call represents the value of the body of the function which is
bound to the name `<f>` while the function's paremeters are assigned values of
the function call arguments `<a>`, by matching positions. The number of
arguments must be equal to the number of parameters.

#### Mapping

Syntax:

    <mapping> = "for" "each" "(" <m:mapping source> ( ; <m:mapping source> )* ") <mapping body>
    <mapping source> = [ <v:id> ] [ "takes" <n:expr> ] [ "every" <h:expr> ] in <d:expr>
    <mapping body> = <complex expr> | <block>

The result of a mapping is a stream composed of values computed from
values of one or more other streams by code in the mapping body.

The body is executed multiple times, each time taking a different slice of
elements from each source stream, processing it, and appending the result to
the final stream.

Each source `<m>` is defined by a stream `<d>`, also named a mapping *domain*,
the integer size `<n>` of the slice of its elements in first dimension
to process at a time, the integer number `<h>` of elements to advance the slice
by each time, and the name `<v>` to which the slice is assigned so that it can
be used in the mapping body. The scope of each name `<v>` is limited to the
mapping body.

#### Reduction

Syntax:

    <reduction> = "reduce" "(" <v1:id> "," <v2:id> "in" <d:expr> ")" <reduction body>
    <reduction body> = <complex expr> | <block>

The result of a reduction is a single real number computed by repeatedly
executing the reduction body over different values of a stream and values
of previous executions of the body.

First, initial two elements in the first dimension of the *domain* stream `<d>`
are bound to `<v1>` and `<v2>`, and the body is executed. Then, the result is
bound to `<v1>` and the next element from the stream is bound to `<v2>`. This is
repeated until reaching the end of stream.

Currently, the domain must be a single-dimension stream, so `<v1>` and `<v2>`
will always be real values. The result type of the body may be a real or an int;
in the latter case it is converted to a real before being bound to `<v1>` for
the next execution of the body.

