#ifndef __TEXTURES_H__
#define __TEXTURES_H__

#include "tiny_obj_loader.h"

namespace konstructs {
    #define BLOCK_TEXTURES 5
    #define SKY_TEXTURE 2
    #define FONT_TEXTURE 3
    #define INVENTORY_TEXTURE 4
    #define PLAYER_TEXTURE 6
    void load_textures();
    tinyobj::shape_t load_player();
};
#endif
