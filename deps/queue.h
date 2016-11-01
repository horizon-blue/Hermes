#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

/* implement private struct */
struct QueuePrivate;
struct QueueNode;

enum queue_status { QUEUE_TRUE, QUEUE_FALSE };

typedef struct Queue
{
    struct QueuePrivate * p;

    /* function pointer */
    int ( *push ) ( struct Queue * self, void * element );
    void * ( *peek ) ( struct Queue * self );
    void * ( *pull ) ( struct Queue * self );
    ssize_t ( *size ) ( struct Queue * self );
    int ( *empty ) ( struct Queue * self );
    struct QueueNode * ( *search ) ( struct Queue * self, void * value, struct QueueNode * start );
    struct QueueNode * ( *get_next ) ( struct Queue * self, struct QueueNode * current );
} Queue;

Queue * initialize_queue ( Queue * ptr );

#endif
