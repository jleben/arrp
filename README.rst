Introduction
############

Arrp is a functional programming language for digital signal processing.
This project provides a compiler for Arrp.

For more information on the language Arrp, visit the `Arrp website`_.

Table of Contents:
##################

- `Installation <#installation>`_
- `Usage <#usage>`_
- `Building from Source <#building-from-source>`_

Installation
#############

Installation packages are available `on GitHub <https://github.com/jleben/arrp/releases>`_ as release assets.

- Debian package:

  Depending on your Linux setup, double-clicking may open a graphical installer.

  Alternatively, the following command installs the package using ``dpkg``::

    dpkg -i package_name.deb

- Mac OS ZIP archive:

  - Unzip the folder.
  - The compiler executable is at ``bin/arrp`` in the unzipped folder. Optionally, add the ``bin`` dir to the ``PATH`` environment variable.
  - If you wish ``arrp`` to generate executables, either set the environment variable ``ARRP_HOME`` to the unzipped folder, or add the ``include`` subfolder to the ``CPATH`` environment variable.

Usage
#####

Invoking the compiler with the `-h` option will print information about
its usage and available options::

    arrp -h

To generate C++ from Arrp code (replacing ``program...`` with the desired file name)::

    arrp program.arrp --cpp program.cpp

The Arrp compiler can also generate an executable (this also requires a C++ compiler)::

    arrp program.arrp --exe program

Run the executable with the `-h` option to get help on how to use it::

    ./program -h

See the `documentation <http://arrp-lang.info/doc>`_
for more information about the generated C++ code and executables.

.. _Arrp website: http://arrp-lang.info


Building from Source
####################

Prerequisites
=============

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
    make install

You can control where the compiler is installed using the CMake variable `CMAKE_INSTALL_PREFIX <https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX.html>`_.

After installation, the Arrp compiler executable is located at ``CMAKE_INSTALL_PREFIX/bin/arrp``.


Running tests
=============

1. Build and install the compiler.
1. Run tests using: ``ctest``


Options
=======

The CMake build system provides the following options:

- PARSER_REGENERATE - Regenerate lexer and parser using flex and bison.
- PARSER_GENERATOR_OUTPUT_DESCRIPTION - When generating parser, output parser description.
