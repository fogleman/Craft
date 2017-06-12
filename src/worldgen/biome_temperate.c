#include "biome_temperate.h"
#include "noise.h"
#include "../config.h"
#include "../item.h"

void generateTemperate(int dx, int dz, int x, int z, int start_h, int h, int flag, world_func func, void *arg) {
    for (int y = start_h; y < h - 1; y++) {
        func(x, y, z, Item_DIRT * flag, arg);
    }
    func(x, h - 1, z, Item_GRASS * flag, arg);

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
    int ok = 1;
    if (dx - 4 < 0 || dz - 4 < 0 ||
        dx + 4 >= CHUNK_SIZE || dz + 4 >= CHUNK_SIZE)
    {
        ok = 0;
    }

    if (ok && simplex2(x, z, 6, 0.5, 2) > 0.84) {
        for (int y = h + 3; y < h + 8; y++) {
            for (int ox = -3; ox <= 3; ox++) {
                for (int oz = -3; oz <= 3; oz++) {
                    int d = (ox * ox) + (oz * oz) +
                        (y - (h + 4)) * (y - (h + 4));
                    if (d < 11) {
                        func(x + ox, y, z + oz, 15, arg);
                    }
                }
            }
        }
        for (int y = h; y < h + 7; y++) {
            func(x, y, z, 5, arg);
        }
    }
}
