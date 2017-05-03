#include "biome_rainforest.h"
#include "noise.h"
#include "../config.h"
#include "../item.h"

void generateRainforest(int dx, int dz, int x, int z, int start_h, int h, int flag, world_func func, void *arg) {
    for (int y = start_h; y < h; y++) {
        func(x, y, z, 1 * flag, arg);
    }

    // grass
    if (simplex2(-x * 0.1, z * 0.1, 4, 0.8, 2) > 0.6) {
        func(x, h, z, 17 * flag, arg);
    }
    // flowers
    if (simplex2(x * 0.05, -z * 0.05, 4, 0.8, 2) > 0.7) {
        int w = 18 + simplex2(x * 0.1, z * 0.1, 4, 0.8, 2) * 7;
        func(x, h, z, w * flag, arg);
    }

    // trees
    /** TODO **/
    // Is removing this ok?
    int ok = 1;
    if (dx - 4 < 0 || dz - 4 < 0 ||
        dx + 4 >= CHUNK_SIZE || dz + 4 >= CHUNK_SIZE)
    {
        ok = 0;
    }
    if (ok && simplex2(x, z, 6, 0.5, 2) > 0.82) {
        //Leaves
        for (int y = h + 46; y < h + 54; y++) {
            for (int ox = -6; ox <= 6; ox++) {
                for (int oz = -6; oz <= 6; oz++) {
                    int d = (ox * ox) + (oz * oz) +
                        (y - (h + 46)) * (y - (h + 46));
                    if (d < 22) {
                        func(x + ox, y, z + oz, 15, arg);
                        func(x + ox, y - (15 + (x % 2) * 2 + (z % 2) * 2), z + oz, 15, arg);
                    }
                }
            }
        }
        //Vines
        for (int y = h + 35; y < h + 46; y++) {
            for (int ox = -6; ox <= 6; ox++) {
                for (int oz = -6; oz <= 6; oz++) {
                    if (y >= h + 43) {
                        int d = (ox * ox) + (oz * oz) +
                            (y - (h + 46)) * (y - (h + 46));
                        if (d < 12) {
                            func(x + ox, y, z + oz, Item_VINE, arg);
                            func(x + ox, y - (15 + (x % 2) * 2 + (z % 2) * 2), z + oz, Item_VINE, arg);
                        }
                    }

                    int d = (ox * ox) + (oz * oz);
                    if (d > 15 && d < 22 && ox % 4 && oz % 4) {
                        func(x + ox, y, z + oz, Item_VINE, arg);
                        func(x + ox, y - (15 + (x % 2) * 2 + (z % 2) * 2), z + oz, Item_VINE, arg);
                    }
                }
            }
        }
        //Trunk
        for (int y = h; y < h + 46; y++) {
            for (int ox = -1; ox < 1; ox++) {
                for (int oz = -1; oz < 1; oz++) {
                    func(x + ox, y, z + oz, 5, arg);
                }
            }
        }
    }
    //Generate large normal trees as well.
    if (simplex2(x, z, 6, 0.5, 2) > 0.72) {
        for (int y = h + 6; y < h + 16; y++) {
            for (int ox = -8; ox <= 8; ox++) {
                for (int oz = -8; oz <= 8; oz++) {
                    int d = (ox * ox) + (oz * oz) +
                        (y - (h + 6)) * (y - (h + 6));
                    if (d < 11) {
                        if((dx + ox >= 0) && (dz + oz >= 0) &&
                                (dx + ox < CHUNK_SIZE) && (dz + oz < CHUNK_SIZE)) {
                            func(x + ox, y, z + oz, 15, arg);
                        }
                    }
                }
            }
        }
        for (int y = h; y < h + 6; y++) {
            if((dx >= 0) && (dz >= 0) &&
                    (dx < CHUNK_SIZE) && (dz < CHUNK_SIZE)) {
                func(x, y, z, 5, arg);
            }
        }
    }
}
