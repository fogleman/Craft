# Install and Dependencies

## Mac OS X

Download and install [CMake](http://www.cmake.org/cmake/resources/software.html)
if you don\'t already have it. You may use [Homebrew](http://brew.sh) to simplify
the installation:

    brew install cmake

## Linux (Ubuntu)

    sudo apt-get install cmake libglew-dev xorg-dev
    sudo apt-get build-dep glfw

## Windows

Note: Windows builds are untested on this fork.

Download and install [CMake](http://www.cmake.org/cmake/resources/software.html)
and [MinGW](http://www.mingw.org/). Add `C:\MinGW\bin` to your `PATH`.

Use the following commands in place of the ones described in the next section.

    cmake -G "MinGW Makefiles"
    mingw32-make

## Compile and Run

Once you have the dependencies, run the following commands in your
terminal.

    git clone https://github.com/konstructs/client.git
    cd client
    cmake .
    make
    ./craft

## Dependencies

* GLEW is used for managing OpenGL extensions across platforms.
* GLFW is used for cross-platform window management.
* lodepng is used for loading PNG textures.
* tinycthread is used for cross-platform threading.
* zlib for chunk data decompression (server sends compressed chunks).

