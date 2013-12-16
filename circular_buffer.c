#include <stdio.h>
#include <stdlib.h>
#include "circular_buffer.h"
 
int cb_init(CircularBuffer **cb, int size) {
    *cb = (CircularBuffer *) malloc(sizeof(CircularBuffer));
    (*cb)->size  = size;
    (*cb)->start = 0;
    (*cb)->end   = 0;
    (*cb)->length = 0;
    (*cb)->elems = (void *) malloc(sizeof(void *) * size);
    return 0;
}
 
void cb_free(CircularBuffer **cb) {
    free((*cb)->elems);
    free(*cb);
}
 
int cb_push(CircularBuffer *cb, void *elem) {
    if (cb->length == cb->size) {
        void **tmp = (void *) malloc(sizeof(void*) * cb->size * 2);
        int i = 0;
        for (; i < cb->size; i++){
            tmp[i] = cb->elems[(cb->start+i) % cb->size];
        }
        cb->size = cb->size * 2;
        cb->start = 0;
        cb->end = i;
        free(cb->elems);
        cb->elems = tmp;
    }
    cb->length++;
    cb->elems[cb->end] = elem;
    cb->end = (cb->end + 1) % cb->size;
}

void *cb_shift(CircularBuffer *cb) {
    if (!cb->length) {
        return NULL;
    }

    void *ret = cb->elems[cb->start];

    cb->start = (cb->start + 1) % cb->size;
    cb->length--;

    return ret;
}
