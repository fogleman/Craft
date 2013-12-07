## Craft

A simple Minecraft clone written in C using modern OpenGL (shaders).

### Features

* World changes persisted in a sqlite3 database.
* Simple but nice looking terrain generation using perlin / simplex noise.
* Nine types of blocks, but more can be added easily.
* Supports plants (grass, flowers, etc.) and transparency (glass).
* Simple clouds in the sky (they don't move).
* Shadow mapping for nice looking shadows (on the shadow branch).

### How to Build and Run

- Download and install [CMake](http://www.cmake.org/cmake/resources/software.html) and [Git](http://git-scm.com/downloads) if you don't already have them.
- From the terminal/command prompt, run `git clone --recursive https://github.com/fogleman/Craft.git`
    - You cannot download the ZIP; it will not work
- Download [sqlite3](http://www.sqlite.org/download.html) and unzip into a relative directory named "sqlite"

Then, run the following commands in your terminal.

    cmake .
    make
    ./craft

### Screenshot

![](https://raw.github.com/fogleman/Craft/master/screenshot.png)
