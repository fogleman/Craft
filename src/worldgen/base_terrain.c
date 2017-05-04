#include "base_terrain.h"
#include "noise.h"
#include "../config.h"
#include "../item.h"
#include "../util.h"

void generateBaseTerrain(int dx, int dz, int x, int z, int start_h, int h, int flag, world_func func, void *arg) {
    for(int y = start_h; y < h; y++) {
        /*
        //float cave1 = simplex3(x, y, z, 3, 0.8, 0.5);
        //float cave2 = simplex3(x, y + 10, z, 3, 0.8, 0.5);
        float cave1 = simplex2(x, y, 3, 0.8, 0.5);
        float cave2 = simplex2(x + 10, y, 3, 0.8, 0.5);

        if(ABS(cave1 - cave2) < 0.1) {
            func(x, y, z, Item_STONE, arg);
        }
        */

        if ((y < 4 || y > h - 2) || (simplex3(x * 0.005, y * 0.005, z * 0.02, 16, 0.05, 5)
            + simplex3(x * 0.02, y * 0.005, z * 0.005, 16, 0.05, 5) < 1.25)) {
            func(x, y, z, Item_STONE, arg);
        }
    }
}
