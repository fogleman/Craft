#include "config.h"
#include "noise.h"
#include "world.h"

void create_world(Map *map, int p, int q) {
    int pad = 1;
    for (int dx = -pad; dx < CHUNK_SIZE + pad; dx++) {
        for (int dz = -pad; dz < CHUNK_SIZE + pad; dz++) {
            int x = p * CHUNK_SIZE + dx;
            int z = q * CHUNK_SIZE + dz;
            float f = simplex2(x * 0.01, z * 0.01, 4, 0.5, 2);
            float g = simplex2(-x * 0.01, -z * 0.01, 2, 0.9, 2);
            int mh = g * 32 + 16;
            int h = f * mh;
            int w = Grass;
            int t = 12;
            int l = 4;
            if (h <= t) {
                h = t;
                w = Sand;
            }
            if (dx < 0 || dz < 0 || dx >= CHUNK_SIZE || dz >= CHUNK_SIZE) {
                w = -1;
            }
            // sand and grass terrain
            for (int y = 0; y < h; y++) {
                if (y > h - l) {
                    if (w == Grass && y != h - 1)
                        map_set(map, x, y, z, Dirt);
                    else {
                        map_set(map, x, y, z, w);
                        if (w == Grass && map_get(map, x, y - 1, z) == Grass)
                            map_set(map, x, y - 1, z, Dirt);
                    }
                } else
                    map_set(map, x, y, z, Stone);
            }
            // TODO: w = -1 if outside of chunk
            if (w == Grass) {
                // grass
                if (simplex2(-x * 0.1, z * 0.1, 4, 0.8, 2) > 0.6) {
                    map_set(map, x, h, z, Grass);
                }
                // flowers
                if (simplex2(x * 0.05, -z * 0.05, 4, 0.8, 2) > 0.7) {
                    int w = Flower1 + simplex2(x * 0.1, z * 0.1, 4, 0.8, 2) * 7;
                    map_set(map, x, h, z, w);
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
                                    map_set(map, x + ox, y, z + oz, Leaf);
                                }
                            }
                        }
                    }
                    for (int y = h; y < h + 7; y++) {
                        map_set(map, x, y, z, Wood);
                    }
                }
            }
            // clouds
            for (int y = 64; y < 72; y++) {
                if (simplex3(x * 0.01, y * 0.1, z * 0.01, 8, 0.5, 2) > 0.75) {
                    map_set(map, x, y, z, Cloud);
                }
            }
        }
    }
}
