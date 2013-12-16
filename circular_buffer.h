#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

typedef struct {
    int         size;   /* maximum number of elements           */
    int         start;  /* index of oldest element              */
    int         end;    /* index at which to write new element  */
    int         length; /* number of elements */
    void        **elems;  /* vector of elements                   */
} CircularBuffer;

int cb_init(CircularBuffer **cb, int size);
 
void cb_free(CircularBuffer **cb);
 
int cb_push(CircularBuffer *cb, void *elem);

void *cb_shift(CircularBuffer *cb);

#endif
