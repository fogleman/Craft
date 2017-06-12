#include "biome_desert.h"
#include "noise.h"
#include "../config.h"
#include "../item.h"

void generateDesert(int dx, int dz, int x, int z, int start_h, int h, int flag, world_func func, void *arg) {
    for (int y = start_h; y < h; y++) {
        func(x, y, z, 2 * flag, arg);
    }

    if (simplex2(-x * 0.1, z * 0.1, 4, 0.8, 2) > 0.8) {
        func(x, h, z, 18 * flag, arg);
    }

    int ok = 1;
    if (dx - 4 < 0 || dz - 4 < 0 ||
        dx + 4 >= CHUNK_SIZE || dz + 4 >= CHUNK_SIZE)
    {
        ok = 0;
    }
    if (ok && simplex2(x * 4, z * 4, 6, 0.45, 2) > 0.88) {
        int height = simplex2(x, z, 6, 0.5, 2) * 16;
        for (int y = h; y < h + height; y++) {
            func(x, y, z, Item_CACTUS, arg);
        }

        int oz = z % 3;

        if(x % 2) {
            func(x + 1, h + height - 4 - oz, z, Item_CACTUS, arg);
            func(x - 1, h + height - 4 + oz, z, Item_CACTUS, arg);

            func(x + 2, h + height - 4 - oz, z, Item_CACTUS, arg);
            func(x - 2, h + height - 4 + oz, z, Item_CACTUS, arg);

            func(x + 2, h + height - 3 - oz, z, Item_CACTUS, arg);
            func(x - 2, h + height - 3 + oz, z, Item_CACTUS, arg);
        } else {
            func(x, h + height - 4 - oz, z + 1, Item_CACTUS, arg);
            func(x, h + height - 4 + oz, z - 1, Item_CACTUS, arg);

            func(x, h + height - 4 - oz, z + 2, Item_CACTUS, arg);
            func(x, h + height - 4 + oz, z - 2, Item_CACTUS, arg);

            func(x, h + height - 3 - oz, z + 2, Item_CACTUS, arg);
            func(x, h + height - 3 + oz, z - 2, Item_CACTUS, arg);
        }
    }
}
