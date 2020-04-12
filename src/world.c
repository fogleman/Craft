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

            //use a random number generator to determine if a moutain will
            //be made or not set to 90 so moutains are rare
            int num = (rand() % (100 - 1 + 1)) + 1;
            int h = f * mh *2;

            int w = 1;
            int t = 12;
            //player height is .75 so brick player is standing on is height -.75
            //sand starts at brick 12 which is the value of t
            //int oldh = h;

            /**
                Grass is 1
                Sand  is 2
                Stone is 6
            */
            //if the height is less than ground level (12) then use sand brick
            if (h <= t+1/*t is a hardcoded ground level*/) {
                w = 2;
            }


            //if height is 2 less than ground level then use stone brick
            //if the level is over 28 then use the stone block. This means
            //a moutain is being formed
            if (h <= t-2 || h>= 32) {
                //h = t;
                w = 6;

                if (num >=95)
                {
                	w = 1;
                }

                if (h>=55)
                {
                	//white wool looks like snow brick
                	//use that brick if the height is moutain high
                	w=61;
                }
            }
            //terrain
            for (int y = 0; y < h; y++) {
                // this is what generates the map

                func(x, y, z, w * flag, arg);
            }
            // /*
            if (w == 1) {
                if (SHOW_PLANTS) {
                    // grass
                    if (simplex2(-x * 0.1, z * 0.1, 4, 0.8, 2) > 0.6) {
                        func(x, h, z, 17 * flag, arg);
                    }
                    // flowers
                    if (simplex2(x * 0.05, -z * 0.05, 4, 0.8, 2) > 0.7) {
                        int w = 18 + simplex2(x * 0.1, z * 0.1, 4, 0.8, 2) * 7;
                        func(x, h, z, w * flag, arg);
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
             //clouds
            if (SHOW_CLOUDS) {
                for (int y = 64; y < 72; y++) {
                    if (simplex3(
                       x * 0.01, y * 0.1, z * 0.01, 8, 0.5, 2) > 0.75)
                    {
                      func(x, y, z, 16 * flag, arg);
                   }
               }
            }//*/

        }
    }
}
