#include "biome_taiga.h"
#include "noise.h"
#include "../config.h"
#include "../item.h"

void generateTaiga(int dx, int dz, int x, int z, int start_h, int h, int flag, world_func func, void *arg) {
    for (int y = start_h; y < h - 1; y++) {
        func(x, y, z, Item_DIRT * flag, arg);
    }
    func(x, h - 1, z, Item_SNOW * flag, arg);

    // grass
    if (simplex2(-x * 0.1, z * 0.1, 4, 0.8, 2) > 0.7) {
        func(x, h, z, 17 * flag, arg);
    }

    // trees
    int ok = 1;
    if (dx - 4 < 0 || dz - 4 < 0 ||
        dx + 4 >= CHUNK_SIZE || dz + 4 >= CHUNK_SIZE)
    {
        ok = 0;
    }
    if (ok && simplex2(x, z, 6, 0.5, 2) > 0.8) {
        for (int y = h + 1; y < h + 29; y += 4) {
            for (int ox = -3; ox <= 3; ox++) {
                for (int oz = -3; oz <= 3; oz++) {
                    int d = (ox * ox) + (oz * oz) +
                        (y - 20 - (h + 4)) * (y - 20 - (h + 4));
                    if (d < 400) {
                        func(x + ox, y, z + oz, 15, arg);
                    }
                }
            }
        }
        for (int y = h; y < h + 26; y++) {
            for (int ox = -1; ox < 1; ox++) {
                for (int oz = -1; oz < 1; oz++) {
                    func(x + ox, y, z + oz, 5, arg);
                }
            }
        }
    }
}

