## Konstructs

[![Join the chat at https://gitter.im/konstructs/client](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/konstructs/client?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge) [![Build Status](https://travis-ci.org/konstructs/client.svg?branch=master)](https://travis-ci.org/konstructs/client)

This is a Infiniminer/Minecraft inspired multiplayer open source game, focused on massive gameplay. We are trying to remove some of the limitations built into the original games.

Just a few thousand lines of modern C++ using modern OpenGL 3.3 (shaders).

This client requires a server to run. You need to start a server on the same computer for a single player like experience.


For more information see our web page at http://www.konstructs.org

### Controls

- WASD to move forward, left, backward, right.
- Space to jump.
- Left Click to destroy a block.
- Right Click or Ctrl* + Left Click to create a block. (\* Use `⌘ Cmd` on Mac instead)
- Middle Click or E to trigger usage function, like open a chest.
- 1 to 9 or use the scroll wheel to select a item slot.

### Purpose

The project aims at building a client that can render and interact with a block-based world. The goal is to keep the client as simple as possible leaving the game logic to the server.

We also like the server to support a survival type of game with inventories and crafting. To this extent we are also extending the client to handle this.

### History

The project is based on [Craft](https://github.com/fogleman/Craft) written by Michael Fogleman. The original project was primaly focused on single player. We forked Craft to focus on simplifying the client for a multiplayer-only game. You can still start a local server if you like to play single player.

### Compile and/or Install

You have binary builds over at our website at http://www.konstructs.org/download/

### Build from source

See [BUILD.md](BUILD.md)

### Server

You need the [konstructs/server](https://github.com/konstructs/server) to play the game.

You can also download a compiled JAR from [bintray.com/konstructs/jars/server/](https://bintray.com/konstructs/jars/server/view#files), which requires [Java](http://java.com).

Start it with `java -jar konstructs-server*.jar` and connect to it with the client.
