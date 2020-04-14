#ifndef _map_h_
#define _map_h_

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


//Water
#define EMPTY_ENTRY_W(e) (!(e)->x && !(e)->y && !(e)->z && !(e)->w)

#define MAP_FOR_EACH_W(map, entry) \
    for (unsigned int i = 0; i <= map->mask; i++) { \
        MapEntryW *entry = map->data + i; \
        if (EMPTY_ENTRY_W(entry)) { \
            continue; \
        }

#define END_MAP_FOR_EACH_W }

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

typedef struct {
    int x;
    int y;
    int z;
    int w;
} MapEntryW;

typedef struct {
    unsigned int mask;
    unsigned int size;
    MapEntryW *data;
} MapW;

void map_alloc(Map *map, int dx, int dy, int dz, int mask);
void map_copy(Map *dst, Map *src);
int map_set(Map *map, int x, int y, int z, int w);
int map_get(Map *map, int x, int y, int z);
void map_grow(Map *map);
void map_free(Map *map);


//water functions
void map_alloc_w(MapW *map);
void map_free_w(MapW *map);
void map_grow_w(MapW *map);
int map_set_w(MapW *map, int x, int y, int z, int w);
int map_get_w(MapW *map, int x, int y, int z);

#endif
