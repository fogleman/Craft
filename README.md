## Konstructs

[![Join the chat at https://gitter.im/konstructs/client](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/konstructs/client?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge) [![Build Status](https://travis-ci.org/konstructs/client.svg?branch=master)](https://travis-ci.org/konstructs/client)

Minecraft clone for Windows, Mac OS X and Linux. Just a few thousand lines of C using modern OpenGL (shaders).

This is a fork of [fogleman/Craft](https://github.com/fogleman/Craft) created for experiments and fun. We are trying to dumb the client down and move logic to the server. There are also plans for a more survival type of gameplay.

Note: Offline mode is removed in this fork, a server is required. You need to start a server on the same computer for single player.

[![Video](http://i.imgur.com/ciU1c0l.png)](https://www.youtube.com/watch?v=KX4UyhOuuh0)

### Features

* Supports normal blocks, plants (grass, flowers, etc.) and transparency (glass).
* Day / night cycles and a textured skybox.
* No height limit.
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
- Right Click or Ctrl* + Left Click to create a block. (\* Use `⌘ Cmd` on Mac instead)
- Tab to toggle between walking and flying.
- Left shift to zoom.
- F to show the scene in orthographic mode.
- O to observe players in the main view.
- P to observe players in the picture-in-picture view.
- Arrow keys emulate mouse movement.
- Enter emulates left mouse click.
- 1 to 9 to select a item slot.
- F3 to toggle debug mode.

### Purpose

The project aims at building a client that can render and interact with a block-based world. The goal is to keep the client as simple as possible leaving the game logic to the server.

We also like the server to support a survival type of game with inventories and crafting. To this extent we are also extending the client to handle this.

### History

The project is based on [Craft](https://github.com/fogleman/Craft) written by Michael Fogleman. The original project was primaly focused on single player. We forked Craft to focus on simplifying the client for a multiplayer-only game. You can still start a local server if you like to play single player.

### Protocol changes

To let the server manage the world, we needed to change the text based protocol to allow binary extensions. This has been done by introducing a four byte header to all commands. We changed only the `C` (chunk) command to transfer compressed binary data, this reduces the amount of data being sent and greatly improves the speed of the client.

### Textures

In long term, we want textures to be managed and sent by the server. It should be up to the server administrator to choose what type of blocks and textures are available.

### Compile and/or Install

#### Install binaries

##### Debian (Ubuntu)

Import Bintray's signing key
```
sudo apt-key adv --keyserver pgp.mit.edu --recv-key 379CE192D401AB61
```

Add the repository
```
echo 'deb https://dl.bintray.com/konstructs/debian jessie main' | sudo tee /etc/apt/sources.list.d/konstructs.list
```

Update the package database and install the package `konstructs-client`:

```
sudo apt-get update
sudo apt-get install konstructs-client
```

##### Linux (tar archive)

We have a Linux build ready at [bintray.com/konstructs/linux/client](https://bintray.com/konstructs/linux/client/view#files).

##### Windows (zip archive)

We have a Windows build ready at [bintray.com/konstructs/windows/client](https://bintray.com/konstructs/windows/client/view#files).

##### OS X

Sorry, we have no automatic builds for OS X. You need to build it from source.

#### Build from source

See [BUILD.md](BUILD.md)

### How to play

See http://www.konstructs.org/download/

### Connecting to a Server

There are multiple ways to connect to a server. The most common way is to just start the game and follow the on-screen prompts.

A more advanced way is through commandline arguments. The advantage of this is, that commandline arguments can be included in shortcut files (on Windows `.lnk`, on Linux `.desktop`) to automatically connect to your favorite server!

Possible arguments are:
```
./konstructs
    [(-h/--help)]
    [(-s/--server) <serveraddress>]
    [(-u/--user) <username>]
    [(-p/--password) <password>]
```
Every singe argument is optional, so it is possible to choose which arguments you want to supply.

### Server

You need the [konstructs/server](https://github.com/konstructs/server) to play the game.

You can also download a compiled JAR from [bintray.com/konstructs/jars/server/](https://bintray.com/konstructs/jars/server/view#files), which requires [Java](http://java.com).

Start it with `java -jar konstructs-server*.jar` and connect to it with the client.
