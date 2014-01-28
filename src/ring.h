#ifndef _ring_h_
#define _ring_h_

typedef enum {
    BLOCK,
    LIGHT,
    KEY,
    COMMIT,
    EXIT
} RingEntryType;

typedef struct {
    RingEntryType type;
    int p;
    int q;
    int x;
    int y;
    int z;
    int w;
    int key;
} RingEntry;

typedef struct {
    unsigned int capacity;
    unsigned int start;
    unsigned int end;
    RingEntry *data;
} Ring;

void ring_alloc(Ring *ring, int capacity);
void ring_free(Ring *ring);
int ring_empty(Ring *ring);
int ring_full(Ring *ring);
int ring_size(Ring *ring);
void ring_grow(Ring *ring);
void ring_put(Ring *ring, RingEntry *entry);
void ring_put_block(Ring *ring, int p, int q, int x, int y, int z, int w);
void ring_put_light(Ring *ring, int p, int q, int x, int y, int z, int w);
void ring_put_key(Ring *ring, int p, int q, int key);
void ring_put_commit(Ring *ring);
void ring_put_exit(Ring *ring);
int ring_get(Ring *ring, RingEntry *entry);

#endif
