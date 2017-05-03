#include "base_terrain.h"
#include "noise.h"
#include "../config.h"
#include "../item.h"

void generateBaseTerrain(int dx, int dz, int x, int z, int start_h, int h, int flag, world_func func, void *arg) {
    for(int y = start_h; y < h; y++) {
        func(x, y, z, Item_STONE, arg);
    }
}
