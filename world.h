#ifndef _world_h_
#define _world_h_

#include "map.h"

typedef enum {
    Unknown = -1,
    Air = 0,
    
    Grass,
    Sand,
    Slab,
    Brick,
    Wood,
    Stone,
    Dirt,
    Plank,
    Snow,
    Glass,
    Cobblestone,
    
    Head = 14,
    Leaf,
    Cloud,
    TallGrass,
    Flower1,
    Flower2,
    Flower3,
    Flower4,
    Flower5,
    Flower6,
    Flower7
} BlockType;

void create_world(Map *map, int p, int q);

#endif
