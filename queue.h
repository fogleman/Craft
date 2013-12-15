#ifndef _queue_h_
#define _queue_h_

#include <stdio.h>
#include <stdlib.h>

typedef struct queue_node
{
    void *data;
    struct queue_node *next;
} QueueNode;

typedef struct 
{
    QueueNode *head;
    QueueNode *tail;
    int size;
} Queue;

int queue_init(Queue **queue);
int queue_push(Queue *queue, void *data);
void *queue_shift(Queue *queue);

#endif
