Introduction
############

Arrp is a functional programming language for digital signal processing.
This project provides a compiler for Arrp.

For more information on the language Arrp, visit the `Arrp website`_.

**NOTE: The language and compiler are currently in development towards the release of version 1.0.0 and the documentation may be out of date.**

Table of Contents:
##################

- `Building <#building>`_
- `Usage <#usage>`_

Building
########

Prerequisits
============

**NOTE:** The project requires a compiler with a good support for C++11.

Submodules
----------

This project uses git submodules; make sure to get/update all of them::

    git submodule init
    git submodule update

(Optional) Bison and Flex
-------------------------

Optionally, you may want to re-generate lexer and parser.
This requires the *flex* lexer generator and the *bison* parser generator.

Building
========

isl (Integer Set Library)
-------------------------

The isl library must be built manually, because it is not integrated with the CMake build system of this project.

Execute the following commands, starting in the root of this project::

    cd extra/isl
    mkdir build
    ./autogen.sh
    ./configure --prefix=$(pwd)/build
    make install

This will build isl in the directory ``extra/isl/build``.

Arrp compiler
-------------

After all the prerequisits are in place as described above, you can
build the Arrp compiler using the CMake build system.

On Linux and Mac OS X, execute the following commands, starting in the root of this project::

    mkdir build
    cd build
    cmake ..
    make

This will produce the Arrp compiler executable::

    build/compiler/arrp

Options
=======

The CMake build system provides the following options:

- PARSER_REGENERATE - Regenerate lexer and parser using flex and bison.
- PARSER_GENERATOR_OUTPUT_DESCRIPTION - When generating parser, output parser description.
- BUILD_TESTING - Build tests.


Usage
#####

The Arrp compiler executable is built in this location:
``build/compiler/arrp``.

Invoking the compiler with the `-h` option will print information about
its usage and available options::

    arrp -h

To compile Arrp code from a file and generate C++, invoke the compiler with the
file name and the "--cpp" option::

    arrp <input file> --cpp

Further information about the usage of the generated C++ code is available
in the `Arrp documentation <http://arrp-lang.info/doc/target-cpp>`_.

.. _Arrp website: http://arrp-lang.info
