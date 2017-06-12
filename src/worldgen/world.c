#include "../config.h"
#include "../item.h"
#include "noise.h"
#include "../util.h"
#include "world.h"
#include "base_terrain.h"
#include "biome_temperate.h"
#include "biome_desert.h"
#include "biome_rainforest.h"
#include "biome_taiga.h"
#include <math.h>

void create_world(int p, int q, world_func func, void *arg) {
    int pad = 1;
    for (int dx = -pad; dx < CHUNK_SIZE + pad; dx++) {
        for (int dz = -pad; dz < CHUNK_SIZE + pad; dz++) {
            int flag = 1;
            if (dx < 0 || dz < 0 || dx >= CHUNK_SIZE || dz >= CHUNK_SIZE) {
                flag = -1;
            }
            int x = p * CHUNK_SIZE + dx;
            int z = q * CHUNK_SIZE + dz;
            float f = simplex2(x * 0.01, z * 0.01, 4, 0.5, 2);
            float g = simplex2(-x * 0.01, -z * 0.01, 2, 0.9, 2);

            int mh = g * simplex2(-x * 0.001, -z * 0.001, 2, 16.0, 2);

            mh += simplex2(-x * 0.0005, -z * 0.0005, 2, 16.0, 2) * simplex2(-x * 0.005, -z * 0.005, 2, 16.0, 2) * 100;

            mh += 16;
            int h = f * mh;

            generateBaseTerrain(dx, dz, x, z, 0, 32, flag, func, arg);
            //generateBaseTerrain(dx, dz, x, z, 32, 32 + h, flag, func, arg);

            Biome biome = biome_at_pos(q, x, z);

            if (biome == Biome_TEMPERATE) {
                generateTemperate(dx, dz, x, z, 32, 32 + h, flag, func, arg);
            } else if(biome == Biome_DESERT) {
                generateDesert(dx, dz, x, z, 32, 32 + h, flag, func, arg);
            } else if(biome == Biome_RAINFOREST) {
                generateRainforest(dx, dz, x, z, 32, 32 + h, flag, func, arg);
            } else if(biome == Biome_TAIGA) {
                generateTaiga(dx, dz, x, z, 32, 32 + h, flag, func, arg);
            }

            // clouds
            for (int y = 128; y < 132; y++) {
                if (simplex3(
                    x * 0.01, y * 0.1, z * 0.01, 8, 0.5, 2) > 0.75)
                {
                    func(x, y, z, 16 * flag, arg);
                }
            }
        }
    }
}

Biome biome_at_pos(int q, int x, int z) {
    float biomen = simplex3(-x * 0.0005 * (1 / BIOME_SIZE), -z * 0.0005 * (1 / BIOME_SIZE), q * 0.001, 2, 16.0, 1);
    Biome biome = Biome_max;

    if(biomen > (1.0f/4.0f)) {
        biome = Biome_TEMPERATE;
    }
    if(biomen > (2.0f/4.0f)) {
        biome = Biome_DESERT;
    }
    if(biomen > (3.0f/4.0f)) {
        biome = Biome_RAINFOREST;
    }
    if(biome == Biome_max) {
        biome = Biome_TAIGA;
    }

    return biome;
}
