#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

/* implement private struct */
struct QueuePrivate;
struct QueueNode;

enum queue_status { QUEUE_TRUE, QUEUE_FALSE };
enum queue_event {
    QUEUE_INIT   = 1,
    QUEUE_PUSH   = 2,
    QUEUE_PULL   = 4,
    QUEUE_UPDATE = 8,
    QUEUE_EMPTY  = 16
};

typedef struct Queue {
    struct QueuePrivate* p;

    /* function pointer */
    int ( *push )( struct Queue* self, void* element );
    void* ( *peek )( struct Queue* self );
    void* ( *pull )( struct Queue* self );
    void* ( *bottom )( struct Queue* self );
    ssize_t ( *size )( struct Queue* self );
    int ( *empty )( struct Queue* self );
    struct QueueNode* ( *search )( struct Queue* self, void* value,
                                   struct QueueNode* start );
    struct QueueNode* ( *get_head_node )( struct Queue* self );
    struct QueueNode* ( *get_next )( struct Queue*     self,
                                     struct QueueNode* current );
    void* ( *get_value )( struct Queue* self, struct QueueNode* current );

    /* attach event handler */
    void ( *live )( struct Queue* self, uint8_t event,
                    void ( *handler )( struct Queue* self ) );
} Queue;

Queue* initialize_queue( Queue* ptr );

#endif
