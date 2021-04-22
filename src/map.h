#ifndef _map_h_
#define _map_h_

#ifdef __cplusplus
extern "C" {
#endif
    #include <GL/glew.h>
    #include <GLFW/glfw3.h>
    #include <curl/curl.h>
    #include <math.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <time.h>
    #include "auth.h"
    #include "client.h"
    #include "config.h"
    #include "cube.h"
    // #include "db.h"
    #include "item.h"
    #include "matrix.h"
    #include "noise.h"
    #include "sign.h"
    #include "tinycthread.h"
    #include "util.h"
    #include "world.h"


#define EMPTY_ENTRY(entry) ((entry)->value == 0)

#define MAP_FOR_EACH(map, ex, ey, ez, ew) \
    for (unsigned int i = 0; i <= map->mask; i++) { \
        MapEntry *entry = map->data + i; \
        if (EMPTY_ENTRY(entry)) { \
            continue; \
        } \
        int ex = entry->e.x + map->dx; \
        int ey = entry->e.y + map->dy; \
        int ez = entry->e.z + map->dz; \
        int ew = entry->e.w;

#define END_MAP_FOR_EACH }



typedef union {
    unsigned int value;
    struct {
        unsigned char x;
        unsigned char y;
        unsigned char z;
        char w;
    } e;
} MapEntry;

typedef struct {
    int dx;
    int dy;
    int dz;
    unsigned int mask;
    unsigned int size;
    MapEntry *data;
} Map;


void map_alloc(Map *map, int dx, int dy, int dz, int mask);
void map_free(Map *map);
void map_copy(Map *dst, Map *src);
void map_grow(Map *map);
int map_set(Map *map, int x, int y, int z, int w, int t); //overloaded to take time
int map_get(Map *map, int x, int y, int z);
#ifdef __cplusplus
}
#endif


#endif