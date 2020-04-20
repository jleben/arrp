# Generating a Jack Client

The Arrp compiler can generate C++ code for a Jack client using the option `--interface jack`.

CMake helps automate the process of generating this C++ code and further compiling it into an executable.

## Example

This example demonstrates how to create a Jack client from simple Arrp code that outputs a sine wave at 440 Hz.

### Prerequisites:

- CMake ([Ubuntu](https://packages.ubuntu.com/bionic/cmake), [Mac OS](https://cmake.org/download/))
- Jack development library ([Ubuntu - jack1](https://packages.ubuntu.com/bionic/libjack-dev), [Ubuntu - jack2](https://packages.ubuntu.com/bionic/libjack-jackd2-dev), [Mac OS](https://jackaudio.org/downloads/), [Mac OS - Homebrew](https://formulae.brew.sh/formula/jack))

### Code

Place the following files into the same directory:

- osc.arrp
- CMakeLists.txt

**osc.arrp:**

    input samplerate : int;
    output y : [~]real64;

    y = osc(440.0, 0.0);

    osc(freq, init_ph) =
      sin(ph * 2 * pi) where
        ph = phase(freq, init_ph);

    phase(freq, init) = p where
    {
        p[0] = init;
        p[n] = wrap(p[n-1] + freq[n-1] / samplerate);
    };

    wrap(x) = x - floor(x);

    pi = atan(1) * 4;

**CMakeLists.txt:**

    cmake_minimum_required(VERSION 3.0)

    project(arrp-osc)

    # Uncomment and adjust the following line
    # to specify non-standard location of the Arrp compiler
    # set(CMAKE_PREFIX_PATH "path where Arrp is installed")

    find_package(Arrp REQUIRED)

    arrp_to_jack(osc-jack osc.arrp)

### Procedure (Linux, Mac OS)

1. In the directory containing the above files, run the following commands to generate a Jack client executable `osc-jack`.

        mkdir build
        cd build
        cmake -D CMAKE_BUILD_TYPE=Release ..
        make osc-jack

    Note: If you want machine code that's easier to debug, replace the cmake command with:

        cmake -D CMAKE_BUILD_TYPE=Debug ..

1. With the Jack server running, execute `osc-jack`.

1. A sine wave at 440 Hz will now be playing on the first default Jack output channel.
