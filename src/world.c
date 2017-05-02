#include "config.h"
#include "item.h"
#include "noise.h"
#include "util.h"
#include "world.h"
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
            //printf("%f, %f\n", simplex2(x * 0.01, z * 0.01, 4, 0.5, 2), simplex2(-x * 0.01, -z * 0.01, 2, 0.9, 2));
            int mh = g * simplex2(-x * 0.001, -z * 0.001, 2, 16.0, 2);

            //float mountains = simplex2(-x * 0.0005, -z * 0.0005, 2, 16.0, 2) > 0.5;
            //mh += mountains ? 32 : 128;
            mh += simplex2(-x * 0.0005, -z * 0.0005, 2, 16.0, 2) * simplex2(-x * 0.005, -z * 0.005, 2, 16.0, 2) * 100;

            mh += 16;
            int h = f * mh;
            /*
            int t = 2;
            if (h <= t) {
                h = t;
                w = 2;
            }
            */

            Biome biome = biome_at_pos(q, x, z);

			int w = 0;
			switch(biome) {
                case Biome_TEMPERATE:
                    w = 1;
                    break;
                case Biome_DESERT:
                    w = 2;
                    break;
                case Biome_RAINFOREST:
                    w = 1;
                    break;
                case Biome_TAIGA:
                    w = 9;
                    break;
                default:
                    // Should never happen!
                    biome = Biome_TEMPERATE;
                    break;
			}

            // sand and grass terrain
            for (int y = 0; y < h; y++) {
                func(x, y, z, w * flag, arg);
            }
            if (biome == Biome_TEMPERATE) {
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
            } else if(biome == Biome_DESERT) {
            	if (SHOW_PLANTS) {
                    if (simplex2(-x * 0.1, z * 0.1, 4, 0.8, 2) > 0.8) {
                        func(x, h, z, 18 * flag, arg);
                    }
                }
                // 34
                int ok = SHOW_TREES;
                if (dx - 4 < 0 || dz - 4 < 0 ||
                    dx + 4 >= CHUNK_SIZE || dz + 4 >= CHUNK_SIZE)
                {
                    ok = 0;
                }
                if (ok && simplex2(x * 4, z * 4, 6, 0.45, 2) > 0.88) {
                    /* for (int y = h + 3; y < h + 8; y++) {
                        for (int ox = -3; ox <= 3; ox++) {
                            for (int oz = -3; oz <= 3; oz++) {
                                int d = (ox * ox) + (oz * oz) +
                                    (y - (h + 4)) * (y - (h + 4));
                                if (d < 11) {
                                    func(x + ox, y, z + oz, 15, arg);
                                }
                            }
                        }
                    } */
                    int height = simplex2(x, z, 6, 0.5, 2) * 16;
                    for (int y = h; y < h + height; y++) {
                        func(x, y, z, Item_CACTUS, arg);
                    }

                    int oz = z % 3;
					//printf("oz: %d\n", oz);

					if(x % 2) {
                    	func(x + 1, h + height - 4 - oz, z, Item_CACTUS, arg);
                    	func(x - 1, h + height - 4 + oz, z, Item_CACTUS, arg);

                        func(x + 2, h + height - 4 - oz, z, Item_CACTUS, arg);
                    	func(x - 2, h + height - 4 + oz, z, Item_CACTUS, arg);

                        func(x + 2, h + height - 3 - oz, z, Item_CACTUS, arg);
                    	func(x - 2, h + height - 3 + oz, z, Item_CACTUS, arg);
                    } else {
                    	//printf("test world.c !(x % 2)\n");
                    	func(x, h + height - 4 - oz, z + 1, Item_CACTUS, arg);
                    	func(x, h + height - 4 + oz, z - 1, Item_CACTUS, arg);

                        func(x, h + height - 4 - oz, z + 2, Item_CACTUS, arg);
                    	func(x, h + height - 4 + oz, z - 2, Item_CACTUS, arg);

                        func(x, h + height - 3 - oz, z + 2, Item_CACTUS, arg);
                    	func(x, h + height - 3 + oz, z - 2, Item_CACTUS, arg);
                    }
                }
            } else if(biome == Biome_RAINFOREST) {
            	for (int y = 0; y < h; y++) {
                    func(x, y, z, w * flag, arg);
                }
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
                /** TODO **/
                // Is removing this ok?
                int ok = SHOW_TREES;
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
                /* else {
                    if(ok) {
                        //printf("simplex2: %d %d %f\n", x, z, (double) simplex2(x, z, 6, 0.5, 2));
                    }
                }
                for(int i = 0; i < (simplex2(x, z, 6, 0.5, 2) * 10); ++i) {
                    if(simplex2(x, z, 6, 0.5, 2) < 0.2) {
                        printf("\033[0;31m\033[1m");
                    }
                    printf("%c\033[0m", '#');
                }
                puts("");
                */
            } else if(biome == Biome_TAIGA) {
            	for (int y = 0; y < h; y++) {
                    func(x, y, z, w * flag, arg);
                }
                if (SHOW_PLANTS) {
                    // grass
                    if (simplex2(-x * 0.1, z * 0.1, 4, 0.8, 2) > 0.7) {
                        func(x, h, z, 17 * flag, arg);
                    }
                    // flowers: disabled by FMMC for this ecosystem.
                    /* if (simplex2(x * 0.05, -z * 0.05, 4, 0.8, 2) > 0.7) {
                        int w = 18 + simplex2(x * 0.1, z * 0.1, 4, 0.8, 2) * 7;
                        func(x, h, z, w * flag, arg);
                    } */
                }
                // trees
                int ok = SHOW_TREES;
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
