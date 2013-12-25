#ifndef _ring_h_
#define _ring_h_

typedef struct {
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
void ring_put(
    Ring *ring, int p, int q, int x, int y, int z, int w, int key);
int ring_get(
    Ring *ring, int *p, int *q, int *x, int *y, int *z, int *w, int *key);

#endif
