Introduction
############

Arrp is a functional programming language for digital signal processing.
This project provides a compiler for Arrp.

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

To compile Arrp code from a file, invoke the compiler with the file name
as argument::

    arrp <input file>

This will perform
syntactical and semantical analysis of the program, output any errors,
and perform all the following stages of compilation, except for generating
an output file.

To generate C++ code, add the option ``--cpp``::

    arrp <input file> --cpp

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

.. _Language Syntax: http://webhome.csc.uvic.ca/~jleben/farm2016/syntax.html
.. _Language Semantics: doc/semantics.rst
.. _C++ Target: doc/target-cpp.rst
