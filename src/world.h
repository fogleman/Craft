#ifndef _world_h_
#define _world_h_

// World function callback (apparently used to modify a world's blocks)
typedef void (*world_func)(int x, int y, int z, int w, void *arg);

void create_world(int p, int q, world_func func, void *arg);

#endif
