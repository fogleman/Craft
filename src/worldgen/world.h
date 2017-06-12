#ifndef _world_h_
#define _world_h_

typedef enum {
    Biome_TEMPERATE,
    Biome_DESERT,
    Biome_RAINFOREST,
    Biome_TAIGA,
    Biome_max
} Biome;

typedef void (*world_func)(int, int, int, int, void *);

void create_world(int p, int q, world_func func, void *arg);
Biome biome_at_pos(int q, int x, int z);

#endif // _world_h_
