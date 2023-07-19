#include "config.h"
#include "noise.h"
#include "world.h"

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
            int mh = g * 32 + 16; 
            int h = f * mh;
            int w = 1;
            int t = 12;  //related to block dimensions 
            if (h <= t) {
                h = t;
                w = 2;
            }

            /** 
             * I think this is what is determining where sand and grass blocks are placed up to a certain height
             * by making the h value higher there will be more sand and grass blocks making the world feel 'deeper' 
             * potentially adding another layer of stone under the grass/sand could also create that feeling
             * from the above code 't' appears to be the minimum possible for h, I will try making it much larger to see what 
             * happens and then reduce it to a more reasonable number
             * 
             * t is not the number of block layers.... appears to have altered the block size and i was stuck in a single block 
             * much larger than the player when changing the value to 50
             * 
             * 
             * */ 

            // cobblestone terrain
            /**
             * @brief 
             * make bottom layer of cobblestone under the grass/sand layer
             * cobble stone is block 11 in item array
             * making it half the depth of the original grass/sand layer
             */
            for (int y = 0; y < h/2; y++) {
                func(x, y, z, 11 * flag, arg);
            }
            // sand and grass terrain
            // modifying parameters to start after the cobblestone level
            for (int y = h/2; y < 1.5 * h; y++) {
                func(x, y, z, w * flag, arg);
            }
            if (w == 1) {
                if (SHOW_PLANTS) {
                    // grass
                    if (simplex2(-x * 0.1, z * 0.1, 4, 0.8, 2) > 0.6) {
                        func(x, 1.5*h, z, 17 * flag, arg);
                    }
                    // flowers
                    if (simplex2(x * 0.05, -z * 0.05, 4, 0.8, 2) > 0.7) {
                        int w = 18 + simplex2(x * 0.1, z * 0.1, 4, 0.8, 2) * 7;
                        func(x, 1.5*h, z, w * flag, arg);
                    }
                }
                // trees
                int ok = SHOW_TREES;
                if (dx - 4 < 0 || dz - 4 < 0 ||
                    dx + 4 >= CHUNK_SIZE || dz + 4 >= CHUNK_SIZE)
                {
                    ok = 0;
                }
                if (ok && simplex2(x, z, 6, 0.5, 2) > 0.84) {
                    for (int y = 1.5*h + 3; y < 1.5*h + 8; y++) {
                        for (int ox = -3; ox <= 3; ox++) {
                            for (int oz = -3; oz <= 3; oz++) {
                                int d = (ox * ox) + (oz * oz) +
                                    (y - (1.5*h + 4)) * (y - (1.5*h + 4));
                                if (d < 11) {
                                    func(x + ox, y, z + oz, 15, arg);
                                }
                            }
                        }
                    }
                    for (int y = 1.5*h; y < 1.5*h + 7; y++) {
                        func(x, y, z, 5, arg);
                    }
                }
            }
            // clouds
            if (SHOW_CLOUDS) {
                for (int y = 64; y < 72; y++) {
                    if (simplex3(
                        x * 0.01, y * 0.1, z * 0.01, 8, 0.5, 2) > 0.75)
                    {
                        func(x, y, z, 16 * flag, arg);
                    }
                }
            }
        }
    }
}
