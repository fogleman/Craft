#define EMPTY_ENTRY(e) (!(e)->x && !(e)->y && !(e)->z && !(e)->w)

typedef struct {
    int x;
    int y;
    int z;
    int w;
} Entry;

typedef struct {
    unsigned int mask;
    unsigned int size;
    Entry *data;
} Map;

void map_alloc(Map *map);
void map_free(Map *map);
void map_set(Map *map, int x, int y, int z, int w);
int map_get(Map *map, int x, int y, int z);
