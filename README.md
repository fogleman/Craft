## Craft

A simple Minecraft clone written in C using modern OpenGL (shaders).

![](https://raw.github.com/fogleman/Craft/master/screenshot.png)

### Features

* World changes persisted in a sqlite3 database.
* Simple but nice looking terrain generation using perlin / simplex noise.
* Nine types of blocks, but more can be added easily.
* Supports plants (grass, flowers, etc.) and transparency (glass).
* Simple clouds in the sky (they don't move).
* Shadow mapping for nice looking shadows (on the shadow branch).

### How to Run

- Download and install [CMake](http://www.cmake.org/cmake/resources/software.html) if you don't already have it.
- All other dependencies are included in the repository.

Then, run the following commands in your terminal.

    git clone https://github.com/fogleman/Craft.git
    cmake .
    make
    ./craft

### Multiplayer

Multiplayer support is being developed on the "socket" branch. You can run your own server or connect to mine.

#### Client

    git checkout socket
    cmake .
    make
    ./craft 199.115.118.225 16018

#### Server

    git checkout socket
    pip install sqlalchemy
    python server.py [HOST [PORT]]

### Controls

- WASD to move forward, left, backward, right.
- Space to jump.
- Left Click to destroy a block.
- Right Click or Cmd + Left Click to create a block.
- 1-9 to select the block type to create.
- E to cycle through the block types.
- Tab to toggle between walking and flying.
- ZXCVBN to move in exact directions along the XYZ axes.
- Left shift to zoom.
- F to show the scene in orthographic mode.
