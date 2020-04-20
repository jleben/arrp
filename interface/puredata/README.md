# Generating a Pure Data Object

The Arrp compiler can generate C++ code for a Pure Data external object using the option `--interface puredata`.

CMake helps automate the process of generating this C++ code and further compiling it into a dynamic library.

## Example

This example demonstrates how to create a Pure Data external object from simple Arrp code that outputs a sine wave at 440 Hz.

### Prerequisites:

- CMake ([Ubuntu](https://packages.ubuntu.com/bionic/cmake), [Mac OS](https://cmake.org/download/))
- Pure Data development headers (come with Pure Data) ([Ubuntu](https://packages.ubuntu.com/bionic/puredata-dev), [Mac OS](https://puredata.info/docs/faq/macosx))
- Portable Coroutine Library ([Ubuntu](https://packages.ubuntu.com/bionic/libpcl1-dev), [Mac OS - Homebrew](https://formulae.brew.sh/formula/libpcl))

### Code

Place the following files into the same directory:

- osc.arrp
- CMakeLists.txt

**osc.arrp:**

    input samplerate : int;
    input freq : [~]real64;
    output y : [~]real64;

    y = osc(freq, 0.0);

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

    arrp_to_pd(arrp_osc osc.arrp)

### Procedure (Linux, Mac OS)


1. In the directory containing the above files, use the following commands to generate a Pure Data library named `arrp_osc~.pd_linux` on Linux or `arrp_osc~.pd_darwin` on Mac OS:

        mkdir build
        cd build
        cmake -D CMAKE_BUILD_TYPE=Release ..
        make arrp_osc

    Note: If you want machine code that's easier to debug, replace the cmake command with:

        cmake -D CMAKE_BUILD_TYPE=Debug ..

1. Copy the generated library into your Pure Data external location.

1. In Pure Data, you can now create an object `arrp_osc~`. The object has 1 signal outlet producing a sine wave, and 1 signal inlet controlling the sine wave frequency.
