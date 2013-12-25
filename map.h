#ifndef _map_h_
#define _map_h_

#define EMPTY_ENTRY(e) (!(e)->x && !(e)->y && !(e)->z && !(e)->w)

#define MAP_FOR_EACH(map, entry) \
    for (unsigned int i = 0; i <= map->mask; i++) { \
        MapEntry *entry = map->data + i; \
        if (EMPTY_ENTRY(entry)) { \
            continue; \
        }

#define END_MAP_FOR_EACH }

typedef struct {
    int x;
    int y;
    int z;
    int w;
} MapEntry;

typedef struct {
    unsigned int mask;
    unsigned int size;
    MapEntry *data;
} Map;

void map_alloc(Map *map);
void map_free(Map *map);
void map_grow(Map *map);
void map_set(Map *map, int x, int y, int z, int w);
int map_get(Map *map, int x, int y, int z);

#endif
