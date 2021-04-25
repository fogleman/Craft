#ifndef _world_h_
#define _world_h_

typedef void (*world_func)(int, int, int, int, void *);

void create_world(int p, int q, world_func func, void *arg);

void create_clouds(int flag, int x, int z, int w, world_func func, void *arg);
void create_fog(int flag, int x, int z, int w, world_func func, void *arg);

#endif
