## Craft

[![Build Status](https://travis-ci.org/nsg/Craft.svg?branch=master)](https://travis-ci.org/nsg/Craft)

Minecraft clone for Windows, Mac OS X and Linux. Just a few thousand lines of C using modern OpenGL (shaders).

This is a fork of [fogleman/Craft](https://github.com/fogleman/Craft) created for experiments and fun. We are trying to dumb the client down and move logic to the server. There are also plans for a more survival type of gameplay.

Note: Offline mode is removed in this fork, a server is required. You need to start a server on the same computer for single player.

![Screenshot](http://i.imgur.com/MCkqcUY.png)

### Features

* Support normal blocks, plants (grass, flowers, etc.) and transparency (glass).
* Day / night cycles and a textured sky dome.
* No height limit (65536 blocks really).
* Survival type of gameplay (work in progress).
* Simple player inventory.

#### Unique to this fork

* Compressed binary chunk data (we are sending full chunks from the server).
* The client is just a viewer, the server decides what happens in the world.
* Auth against the server, no central registry.

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
- Enter emulates left mouse click.
- 1 to 9 to select a item slot.
- F3 to toggle debug mode.

### Purpose

The project aims at building a client that can render and interact with a block based world. The goal is to keep the client as simple as possible leavning the game logic to the server.

We also like the server to support a survival type of game with inventories and crafting. To this extent we are also extending the client to handle this.

### History

The project is based of [Craft](https://github.com/fogleman/Craft) written by Michael Fogleman. The original project was primaly focused on single player. We forked Craft to focus on simplifying the client for a multiplayer only game. You can still start a local server if you like to run singleplayer.

### Protocol changes

To let the server manage the world we needed to change the text based protocol to allow binary extensions. This has been done by intruducing a four byte header to all commands. We changed only the `C` (chunk) command to transfer compressed binary data, this reduces the amount of data being sent and greatly improves the speed of the client.

### Textures

In the long term we want textures to be managed and sent by the server.  It should be up to the server administrator to choose what type of blocks and textures that are avaiable.

### Compile and Install

See [INSTALL.md](INSTALL.md)

### Server

You need the [petterarvidsson/craft-akka-server](https://github.com/petterarvidsson/craft-akka-server) to play the game. You can download a compiled JAR from [bintray](https://bintray.com/nsg/craft/craft-server/view).

Start it with `java -jar craft-server*.jar` and connect to it with the client, for example: `./craft localhost nsg mypassword`.

