#include "config.h"
#include "noise.h"
#include "world.h"

// Main terrain generation function
// Parameters:
// - p: chunk p location
// - q: chunk q location
// - func: function callback to modify blocks in the world (see world.h)
// - arg: last argument to be used for the function callback
void create_world(int p, int q, world_func func, void *arg) {
	// GUESS: the inclusion of the extra pad locations is for tree generation across chunk borders.
    int pad = 1;
	// Loop for each (x, z) location in chunk (p, q):
    for (int dx = -pad; dx < CHUNK_SIZE + pad; dx++) {
        for (int dz = -pad; dz < CHUNK_SIZE + pad; dz++) {
			// Why is flag either +1 or -1?
            int flag = 1;
            if (dx < 0 || dz < 0 || dx >= CHUNK_SIZE || dz >= CHUNK_SIZE) {
                flag = -1;
            }
            int x = p * CHUNK_SIZE + dx; // convert p (chunk x) and dx to world x
            int z = q * CHUNK_SIZE + dz; // convert q (chunk z) and dz to world z
            float f = simplex2(x * 0.01, z * 0.01, 4, 0.5, 2);
            float g = simplex2(-x * 0.01, -z * 0.01, 2, 0.9, 2);
            int mh = g * 32 + 16;
            int h = f * mh;
			// w = block id
            int w = 1; // grass
            int t = 12;
            if (h <= t) {
                h = t;
                w = 2; // sand
            }
            // sand and grass terrain
            for (int y = 0; y < h; y++) {
                func(x, y, z, w * flag, arg);
            }
			// Place plants on grass (block id 1).
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
            // clouds
            if (SHOW_CLOUDS) {
                for (int y = 64; y < 72; y++) {
                    if (simplex3(
                        x * 0.01, y * 0.1, z * 0.01, 8, 0.5, 2) > 0.75)
                    {
						// Block id 16 is cloud
                        func(x, y, z, 16 * flag, arg);
                    }
                }
            }
        }
    }
}
