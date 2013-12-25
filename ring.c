#include <stdlib.h>
#include "ring.h"

void ring_alloc(Ring *ring, int capacity) {
    ring->capacity = capacity;
    ring->start = 0;
    ring->end = 0;
    ring->data = (RingEntry *)calloc(capacity, sizeof(RingEntry));
}

void ring_free(Ring *ring) {
    free(ring->data);
}

int ring_empty(Ring *ring) {
    return ring->start == ring->end;
}

int ring_full(Ring *ring) {
    return ring->start == (ring->end + 1) % ring->capacity;
}

int ring_size(Ring *ring) {
    if (ring->end >= ring->start) {
        return ring->end - ring->start;
    }
    else {
        return ring->capacity - (ring->start - ring->end);
    }
}

void ring_grow(Ring *ring) {
    Ring new_ring;
    ring_alloc(&new_ring, ring->capacity * 2);
    int p, q, x, y, z, w, key;
    while (ring_get(ring, &p, &q, &x, &y, &z, &w, &key)) {
        ring_put(&new_ring, p, q, x, y, z, w, key);
    }
    free(ring->data);
    ring->capacity = new_ring.capacity;
    ring->start = new_ring.start;
    ring->end = new_ring.end;
    ring->data = new_ring.data;
}

void ring_put(
    Ring *ring, int p, int q, int x, int y, int z, int w, int key)
{
    if (ring_full(ring)) {
        ring_grow(ring);
    }
    RingEntry *entry = ring->data + ring->end;
    entry->p = p;
    entry->q = q;
    entry->x = x;
    entry->y = y;
    entry->z = z;
    entry->w = w;
    entry->key = key;
    ring->end = (ring->end + 1) % ring->capacity;
}

int ring_get(
    Ring *ring, int *p, int *q, int *x, int *y, int *z, int *w, int *key)
{
    if (ring_empty(ring)) {
        return 0;
    }
    RingEntry *entry = ring->data + ring->start;
    *p = entry->p;
    *q = entry->q;
    *x = entry->x;
    *y = entry->y;
    *z = entry->z;
    *w = entry->w;
    *key = entry->key;
    ring->start = (ring->start + 1) % ring->capacity;
    return 1;
}
