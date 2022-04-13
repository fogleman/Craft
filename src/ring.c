#include <stdlib.h>
#include <string.h>
#include "ring.h"

// Ring data structure for setting up a sequence of tasks for workers to do to
// the world database.

// Allocate ring with a set initial capacity (which can grow)
// Arguments:
// - ring: pointer to ring structure to modify
// - capacity: initial capacity for ring entries
// Returns:
// - modifies the structure that ring points to
void ring_alloc(Ring *ring, int capacity) {
    ring->capacity = capacity;
    ring->start = 0;
    ring->end = 0;
    ring->data = (RingEntry *)calloc(capacity, sizeof(RingEntry));
}

// Free the ring's data (but does not free the given ring pointer).
// Arguments:
// - ring: pointer to ring structure
// Returns: none
void ring_free(Ring *ring) {
    free(ring->data);
}

// Predicate function for if the ring is empty
// Arguments:
// - ring: pointer to ring structure
// Returns:
// - returns non-zero if ring is empty
int ring_empty(Ring *ring) {
    return ring->start == ring->end;
}

// Predicate function for if the ring is full
// Arguments:
// - ring: pointer to ring structure
// Returns:
// - returns non-zero if ring is full
int ring_full(Ring *ring) {
    return ring->start == (ring->end + 1) % ring->capacity;
}

// Get the ring's number of current entries
// Arguments:
// - ring: pointer to ring structure
// Returns:
// - returns the number of entries
int ring_size(Ring *ring) {
    if (ring->end >= ring->start) {
        return ring->end - ring->start;
    }
    else {
        return ring->capacity - (ring->start - ring->end);
    }
}

// Grow the ring's memory space for ring entries.
// Note: this function is mutually recursive with ring_put
// Arguments:
// - ring: pointer to ring structure to modify
// Returns:
// - modifies the structure that ring points to
void ring_grow(Ring *ring) {
    Ring new_ring;
    RingEntry entry;
    // Create a new ring with double the capacity
    ring_alloc(&new_ring, ring->capacity * 2);
    // Copy the entries from the old ring to the new
    while (ring_get(ring, &entry)) {
        ring_put(&new_ring, &entry);
    }
    // Free the old ring's data
    free(ring->data);
    // Make the old ring have the new ring's values
    ring->capacity = new_ring.capacity;
    ring->start = new_ring.start;
    ring->end = new_ring.end;
    ring->data = new_ring.data;
}

// Put an entry into the ring.
// Note: this function is mutually recursive with ring_grow
// Arguments:
// - ring: pointer to ring structure to modify
// Returns:
// - modifies the structure that ring points to
void ring_put(Ring *ring, RingEntry *entry) {
    // Ensure there is room for a new entry
    if (ring_full(ring)) {
        ring_grow(ring);
    }
    // Copy the new entry to the end of the ring
    RingEntry *e = ring->data + ring->end;
    memcpy(e, entry, sizeof(RingEntry));
    // Increment the end pointer
    ring->end = (ring->end + 1) % ring->capacity;
}

// Put a block entry into the ring.
// Arguments:
// - ring: pointer to ring structure to modify
// - p: chunk x position
// - q: chunk z position
// - x: block x position
// - y: block y position
// - z: block z position
// - w: block type to set
// Returns:
// - modifies the structure that ring points to
void ring_put_block(Ring *ring, int p, int q, int x, int y, int z, int w) {
    RingEntry entry;
    entry.type = BLOCK;
    entry.p = p;
    entry.q = q;
    entry.x = x;
    entry.y = y;
    entry.z = z;
    entry.w = w;
    ring_put(ring, &entry);
}

// Put a light entry into the ring.
// Arguments:
// - ring: pointer to ring structure to modify
// - p: chunk x position
// - q: chunk z position
// - x: light x position
// - y: light y position
// - z: light z position
// - w: light value to set
// Returns:
// - modifies the structure that ring points to
void ring_put_light(Ring *ring, int p, int q, int x, int y, int z, int w) {
    RingEntry entry;
    entry.type = LIGHT;
    entry.p = p;
    entry.q = q;
    entry.x = x;
    entry.y = y;
    entry.z = z;
    entry.w = w;
    ring_put(ring, &entry);
}

// Put a key entry into the ring.
// Arguments:
// - ring: pointer to ring structure to modify
// - p: chunk x position
// - q: chunk z position
// - key: key value to set
// Returns:
// - modifies the structure that ring points to
void ring_put_key(Ring *ring, int p, int q, int key) {
    RingEntry entry;
    entry.type = KEY;
    entry.p = p;
    entry.q = q;
    entry.key = key;
    ring_put(ring, &entry);
}

// Put a commit entry into the ring.
// Arguments:
// - ring: pointer to ring structure to modify
// Returns:
// - modifies the structure that ring points to
void ring_put_commit(Ring *ring) {
    RingEntry entry;
    entry.type = COMMIT;
    ring_put(ring, &entry);
}

// Put a exit entry into the ring.
// Arguments:
// - ring: pointer to ring structure to modify
// Returns:
// - modifies the structure that ring points to
void ring_put_exit(Ring *ring) {
    RingEntry entry;
    entry.type = EXIT;
    ring_put(ring, &entry);
}

// Retrieves and removes the next RingEntry from the ring and copies it to the
// entry argument.
// Arguments:
// - ring: pointer to ring structure to modify
// Returns:
// - returns 0 if an entry was not retrieved
// - returns 1 if an entry was retrieved
// - modifies the structure that ring points to
int ring_get(Ring *ring, RingEntry *entry) {
    if (ring_empty(ring)) {
        return 0;
    }
    RingEntry *e = ring->data + ring->start;
    memcpy(entry, e, sizeof(RingEntry));
    ring->start = (ring->start + 1) % ring->capacity;
    return 1;
}

