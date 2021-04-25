# User Stories and Tasks

## US - User Stories
### 1. As a player, I want the clouds to move so that the environment is more immersive.
1. Dynamic Clouds
    1. ~~Add Cloud Block - 3~~
    2. Add Cloud Movement - ~~3~~
        1. Add function to move cloud at same altitude - 2
        2. Add function to test cloud motion functtion - 2
        3. Implement functions into map.cpp - 8
    4. ~~Dynamic Cloud Shape Generation - 5~~ (Already implemented)
    5. Bound Cloud Movement and Generation - 8
    6. Continuous Cloud Generation during Runtime - 5
    7. Convert files the Cloud source code is dep. on to C++ compilation - ~~3~~
        1.  Create cloud.h - 2
        2.  Create cloud.cpp - 2
        3.  Allow program to be called by C code - 2
        4.  Implement into map.cpp and main.cpp - 3
### 2. As a player, I want a weather system so that there's more variety day-to-day.
1. Rain
    1. Create Dark Cloud Texture - 2
    2. Create Rain Texture - 2
    3. Add Rain Effect to the Game - 5
    4. Add a button or command to change weather - 5
    5. Research Weather Cycle or Random Generation - 5
2. Fog
    1. ~~Add Fog Effect - 3~~
    2. Fog dependent on time of day - 3
        1. Alter map functions to accept time as an argument - 2
        2. Write function to change cloud/fog depending on time - 2
        3. Access time passed in game world - 5
        4. Implement time dependent functions - 5
    4. Make Fog Transparent - 3
    5. Clouds Lower to Fog, Fog Rises to Clouds - 3
        1. Write function to get cloud blocks - 2
        2. Write function to lower cloud blocks - 2
        3. Write function to raise cloud blocks - 2
        4. Write function to test cloud position change - 2
        5. Implement functions into main.cpp and map.cpp - 8
### 3. As a player, I want more biomes so that exploration is more rewarding.
1. Snow Blocks
    1. ~~Add Snow Block to Textures - 1~~
    2. ~~Add Snow Block Texture to Bitmap - 2~~ (Trivial after more research)
    3. ~~Add Snow Block - 3~~ (Trivial after more research)
2. Ice Block
    1. ~~Add Ice Block to Textures - 1~~
    2. Graphics: Make it transparent - 2
    3. Effect: Slow down player movement - 2
    4. Advanced Effect: Momentum - 5
### 4. As a player, I want the world to endlessly generate so that I can explore forever.
1. Find where world generation happens in source files - 8
2. Figure out how world is generated in craft - 8
### 5. As a player, I want TNT so that I can blow things up.
1. Add TNT Block
    1. ~~Add the TNT Block - 3~~
    2. ~~Effect: Destroy Blocks in Radius - 2~~
    3. ~~Trigger: Right-Click TNT Block - 2~~
2. Refined TNT Effects
    1. ~~Chain Trigger: TNT explodes other TNT - 3~~
    2. Graphics: Add an Explosion effect - 5
    3. Sound: Add an explosion sound - 5
    4. Add radial vector explosion for dynamic craters - 8

## DS - Dev Stories
### 1. As a dev, I want our tasks up on a kanban board so that they are easier to assign and visualize. <br>
1. ~~Add tasks to the kanban board - 1~~
### 2. As a dev, I want us to use Doxygen or some other documentation generating software so that we can have fancy .htmls that track our design.
1. ~~Research Doxygen and Alternatives - 5~~
2. ~~Get Doxygen up and running - 2~~
3. ~~Decide on Doxygen standards between the two of us - 2~~
4. ~~Figure out what can be added to .gitignore. - 3~~
### 3. As a dev, I want pertinent documents up on github so that the team always has the most current version to look to for reference.
1. Look into using wiki to store our documents - 2
2. ~~Generate a UserStories markdown file for github - 1~~
### 4. As a dev, I want to learn more about OpenGL so that I feel confident adding new textures and effects to Craft.
1. ~~Alex Completes Watching [Cherno](https://www.youtube.com/playlist?list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2) - 5~~
2. ~~Michael Completes Watching [Cherno](https://www.youtube.com/playlist?list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2) - 5~~
### 5. As a dev, I want a solid branching strategy for our repository so that our version control is stable and effective.
1. ~~Generate branches for repository - 1~~
### 6. As a dev, I want a document that tracks our velocity so that we can measure our progress.
1. ~~Generate SprintTracker.md - 1~~
2. ~~Add it to github - 1~~
### 7. As a dev., I want the project code to be more easily read and highly refactored.
1. Refactor World.c functions
    1. Refactor create_wrold() - 1
2. Refactor map.cpp functions
    1. map_get() - 1
    2. map_set() - 1
3. Refactor main.cpp functions
    1. Refactor _hit_test() - 1
    2. Refactor compute_chunks() - 1
    3. Refactor delete_chunks() - 1
    4. Refactor check_workers() - 1
    5. Refactor set_lights() - 1
    6. Refactor cylinder() - 1
    7. Refactor sphere() - 1
    8. Refactor cube() - 1
