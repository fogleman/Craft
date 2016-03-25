# Install and Dependencies

Clone the git repository and download/update all submodules.

    git submodule update --init --recursive

## Mac OS X

Download and install [CMake](http://www.cmake.org/cmake/resources/software.html)
if you don\'t already have it. You may use [Homebrew](http://brew.sh) to simplify
the installation:

    brew install cmake

## Ubuntu Linux

    sudo apt-get install cmake libglew-dev xorg-dev
    sudo apt-get build-dep glfw

## Windows

Note: Windows builds are untested on this fork. The official builds are cross compiled, for more information see [package/README.md](package/README.md).

Download and install [CMake](http://www.cmake.org/cmake/resources/software.html)
and [MinGW](http://www.mingw.org/). Add `C:\MinGW\bin` to your `PATH`.

Use the following commands in place of the ones described in the next section.

    cmake -G "MinGW Makefiles"
    mingw32-make

## Compile and Run

Once you have the dependencies, run the following commands in your
terminal.

    mkdir build && cd build
    cmake .. && make
    ./konstructs -h
