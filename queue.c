#include "queue.h"

int queue_init(Queue **queue) {
    *queue = (Queue *) malloc(sizeof(Queue));
    if (queue == NULL) return 1;

    (*queue)->head = NULL;
    (*queue)->tail = NULL;
    (*queue)->size = 0;
    return 0;
}

int queue_push(Queue *queue, void *data) {
    QueueNode *newNode = (QueueNode *) malloc(sizeof(QueueNode));
    newNode->data = data;
    newNode->next = NULL;
    if (queue->head == NULL)
    {
        queue->head = queue->tail = newNode;
    }
    else
    {
        queue->tail->next = newNode;
        queue->tail = newNode;
    }
    queue->size = queue->size + 1;
    return queue->size;
}

void *queue_shift(Queue *queue) {
    QueueNode *ret;
    void *data;
    if (queue->head == NULL) {
        return NULL;
    }

    ret = queue->head;
    if (queue->head == queue->tail) {
        queue->head = queue->tail = NULL;
    }
    else {
        queue->head = ret->next;
    }
    queue->size--;
    data = ret->data;
    free(ret);
    return data;
}
