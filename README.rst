Introduction
############

This project implements a compiler for a new programming language
for stream processing.

Table of Contents:
##################

- `Building <#building>`_
- `Usage <#usage>`_
- `Language <#language>`_
- `Targets <#targets>`_

Building
########

Prerequisits
============

**NOTE:** The project requires a compiler with a good support for C++11.

Submodules
----------

This project uses git submodules; make sure to get/update all of them:

    git submodule init
    git submodule update

isl (Integer Set Library)
-------------------------

1. Get git repository: https://github.com/jleben/isl.git.
3. Build and install to `extra/isl` within this project's source.

Project website: [http://isl.gforge.inria.fr](http://isl.gforge.inria.fr/)

(Optional) Bison and Flex
-------------------------

Optionally, you may want to re-generate lexer and parser.
This requires the *flex* lexer generator and the *bison* parser generator.

Building
========

This project has a CMake build system.

After all the prerequisits are in place as described above, you can
build this project using the standard building procedure
on Linux and Mac OS X:

    mkdir build
    cd build
    cmake ..
    make

This will produce the compiler executable:

    build/compiler/streamc

**NOTE:** You may need to adjust compiler and linker flags to enable C++11
support for your compiler. Please refer to the documentation of CMake and your
compiler for instructions.

Options
=======

The CMake build system provides the following options:

- PARSER_REGENERATE - Regenerate lexer and parser using flex and bison.
- PARSER_GENERATOR_OUTPUT_DESCRIPTION - When generating parser, output parser description.
- BUILD_TESTING - Build tests.


Usage
#####

When the project is built, the following compiler executable is generated:
``build/compiler/streamc``.

The compiler takes a file with code in The Language as input and generates
C++ code for the expression named "main".

Invoking the compiler with the `-h` option will print information about
its usage and available options::

    streamc -h

Invoking the compiler with an input file will perform
syntactical and semantical analysis of the program, output any errors,
and perform all the following stages of compilation, except for generating
an output file::

    streamc <input file>

To generate C++ code, add the ``--cpp <namespace>`` option, which
will generate a file ``<namespace>.cpp``, containing C++ code in the
desired namespace::

    streamc <input file> --cpp <namespace>

Read the `C++ target`_ document for details on how to use the generated
C++ code.


Language
########

For details about the programming language,
please refer to the following documents:

- `Language Syntax`_
- `Language Semantics`_

Targets
#######

The only currently available target is C++. Please refer to the
`C++ target`_ document for details.

.. _Language Syntax: doc/syntax.rst
.. _Language Semantics: doc/semantics.rst
.. _C++ Target: doc/target-cpp.rst
