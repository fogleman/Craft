## Craft

[![Build Status](https://travis-ci.org/nsg/Craft.svg?branch=master)](https://travis-ci.org/nsg/Craft)

Minecraft clone for Windows, Mac OS X and Linux. Just a few thousand lines of C using modern OpenGL (shaders). Online multiplayer support is included using a Python-based server.

This is a fork of [fogleman/Craft](https://github.com/fogleman/Craft) created for experiments and fun. We are trying to dumb the client down and move logic to the server. There are also plans for a more survival type of gameplay.

Note: Offline mode is removed in this fork, a server is required. You need to start a server on the same computer for single player.

![Screenshot](http://i.imgur.com/MCkqcUY.png)

### Features

* Simple but nice looking terrain generation using perlin / simplex noise.
* Supports plants (grass, flowers, trees, etc.) and transparency (glass).
* Simple clouds in the sky (they don't move).
* Day / night cycles and a textured sky dome.
* World changes persisted in a sqlite3 database.
* Survival type of gameplay (work in progress)

### Install Dependencies

#### Mac OS X

Note: OS X builds are untested on this fork.

Download and install [CMake](http://www.cmake.org/cmake/resources/software.html)
if you don't already have it. You may use [Homebrew](http://brew.sh) to simplify
the installation:

    brew install cmake

#### Linux (Ubuntu)

    sudo apt-get install cmake libglew-dev xorg-dev
    sudo apt-get build-dep glfw

#### Windows

Note: Windows builds are untested on this fork.

Download and install [CMake](http://www.cmake.org/cmake/resources/software.html)
and [MinGW](http://www.mingw.org/). Add `C:\MinGW\bin` to your `PATH`.

Use the following commands in place of the ones described in the next section.

    cmake -G "MinGW Makefiles"
    mingw32-make

### Compile and Run

Once you have the dependencies (see above), run the following commands in your
terminal.

    git clone https://github.com/fogleman/Craft.git
    cd Craft
    cmake .
    make
    ./craft

### Controls

- WASD to move forward, left, backward, right.
- Space to jump.
- Left Click to destroy a block.
- Right Click or Cmd + Left Click to create a block.
- Tab to toggle between walking and flying.
- ZXCVBN to move in exact directions along the XYZ axes.
- Left shift to zoom.
- F to show the scene in orthographic mode.
- O to observe players in the main view.
- P to observe players in the picture-in-picture view.
- Arrow keys emulate mouse movement.
- Enter emulates mouse click.

### Chat Commands

    /goto [NAME]

Teleport to another user.
If NAME is unspecified, a random user is chosen.

    /list

Display a list of connected users.

    /pq P Q

Teleport to the specified chunk.

    /spawn

Teleport back to the spawn point.

### Implementation Details

For a in depth description of the technical detals see [README.md at fogleman/Craft](https://github.com/fogleman/Craft/blob/master/README.md). fogleman (Michael Fogleman) has written most of the grafic stack and has a excellent description.

#### Dependencies

* GLEW is used for managing OpenGL extensions across platforms.
* GLFW is used for cross-platform window management.
* CURL is used for HTTPS / SSL POST for the authentication process.
* lodepng is used for loading PNG textures.
* sqlite3 is used for saving the blocks added / removed by the user.
* tinycthread is used for cross-platform threading.
