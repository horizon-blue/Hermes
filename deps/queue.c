#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "util.h"

#include "queue.h"

typedef struct handler_t {
    uint8_t event;
    void ( *handler )( struct Queue* self );
} handler_t;

struct QueuePrivate {
    struct QueueNode *head, *tail;
    ssize_t           size;
    pthread_mutex_t   m;

    handler_t* handlers;
    size_t     handler_num;
};

struct QueueNode {
    struct QueueNode *   prev, *next;
    void*                value;
    struct QueuePrivate* parent;
};

typedef struct QueuePrivate QueuePrivate;
typedef struct QueueNode    QueueNode;

int Queue_push( struct Queue* self, void* element );
void* Queue_peek( struct Queue* self );
void* Queue_pull( struct Queue* self );
void* Queue_bottom( struct Queue* self );
ssize_t Queue_size( struct Queue* self );
int Queue_empty( struct Queue* self );
QueueNode* Queue_search( struct Queue* self, void* value, QueueNode* start );
QueueNode* Queue_get_head_node( struct Queue* self );
QueueNode* Queue_get_next( struct Queue* self, QueueNode* current );
void* Queue_get_value( struct Queue* self, QueueNode* current );
void Queue_live( struct Queue* self, uint8_t event,
                 void ( *handler )( struct Queue* self ) );

void Queue_call( struct Queue* self, uint8_t event );

Queue* initialize_queue( Queue* ptr ) {
    if ( !ptr ) {
        ptr = (Queue*)malloc( sizeof( Queue ) );
    }

    memset( ptr, 0, sizeof( Queue ) );

    if ( ptr->p == NULL ) {
        ptr->p = (QueuePrivate*)malloc( sizeof( QueuePrivate ) );
    }

    ptr->p->head = ptr->p->tail = NULL;
    ptr->p->size                = 0;
    ptr->p->handlers            = NULL;
    ptr->p->handler_num         = 0;

    if ( pthread_mutex_init( &( ptr->p->m ), NULL ) != 0 ) {
        exit_with_error( __func__, "pthread_mutex_init() failed" );
    }

    ptr->push          = Queue_push;
    ptr->peek          = Queue_peek;
    ptr->pull          = Queue_pull;
    ptr->bottom        = Queue_bottom;
    ptr->size          = Queue_size;
    ptr->empty         = Queue_empty;
    ptr->search        = Queue_search;
    ptr->get_head_node = Queue_get_head_node;
    ptr->get_next      = Queue_get_next;
    ptr->get_value     = Queue_get_value;
    ptr->live          = Queue_live;

    Queue_call( ptr, QUEUE_INIT );

    return ptr;
}

int Queue_push( struct Queue* self, void* element ) {
    if ( self == NULL || self->p == NULL ) {
        return QUEUE_FALSE;
    }

    pthread_mutex_lock( &( self->p->m ) );

    QueuePrivate* p = self->p;
    QueueNode*    n = (QueueNode*)malloc( sizeof( QueueNode ) );
    n->next = n->prev = NULL;
    n->value          = element;
    n->parent         = p;

#ifdef DEBUG
    printf( "[%s] saved element %p\n", __func__, n->value );
#endif

    if ( p->head == NULL ) {
        p->head = p->tail = n;
    } else {
        p->tail->next = n;
        n->prev       = p->tail;
        p->tail       = n;
    }

    p->size++;

    pthread_mutex_unlock( &( self->p->m ) );

    Queue_call( self, QUEUE_PUSH | QUEUE_UPDATE );

    return QUEUE_TRUE;
}

void* Queue_peek( struct Queue* self ) {
    if ( self == NULL || self->p == NULL ) {
        return NULL;
    }

    pthread_mutex_lock( &( self->p->m ) );

    void* result = self->p->size > 0 ? self->p->head->value : NULL;

    pthread_mutex_unlock( &( self->p->m ) );

    return result;
}

void* Queue_pull( struct Queue* self ) {
    if ( self == NULL || self->p == NULL ) {
        return NULL;
    }

    pthread_mutex_lock( &( self->p->m ) );

    void* result = NULL;

    if ( self->p->size > 0 ) {
        QueueNode* t  = self->p->head;
        result        = t->value;
        self->p->head = t->next;

        free( t );
        t = NULL;

        if ( self->p->head != NULL ) {
            self->p->head->prev = NULL;
        } else {
            self->p->tail = NULL;
        }

        self->p->size--;
    }

    pthread_mutex_unlock( &( self->p->m ) );

    Queue_call( self, QUEUE_PULL | QUEUE_UPDATE |
                          ( self->p->size == 0 ? QUEUE_EMPTY : 0 ) );

    return result;
}

void* Queue_bottom( struct Queue* self ) {
    if ( self == NULL || self->p == NULL ) {
        return NULL;
    }

    pthread_mutex_lock( &( self->p->m ) );

    void* result = self->p->size > 0 ? self->p->tail->value : NULL;

    pthread_mutex_unlock( &( self->p->m ) );

    return result;
}

ssize_t Queue_size( struct Queue* self ) {
    if ( self == NULL || self->p == NULL ) {
        return QUEUE_FALSE;
    }

    pthread_mutex_lock( &( self->p->m ) );

    ssize_t result = self->p->size;

    pthread_mutex_unlock( &( self->p->m ) );

    return result;
}

int Queue_empty( struct Queue* self ) {
    if ( self == NULL || self->p == NULL ) {
        return QUEUE_FALSE;
    }

    pthread_mutex_lock( &( self->p->m ) );

    ssize_t result = self->p->size == 0 ? (int)QUEUE_TRUE : (int)QUEUE_FALSE;

    pthread_mutex_unlock( &( self->p->m ) );

    return result;
}

QueueNode* Queue_search( struct Queue* self, void* value, QueueNode* start ) {
    if ( self == NULL || self->p == NULL ) {
        return NULL;
    }

    pthread_mutex_lock( &( self->p->m ) );

    if ( start == NULL ) {
        start = self->p->head;
    }

    if ( start != NULL && start->parent == self->p ) {
        while ( start != NULL && start->value != value ) {
            start = start->next;
        }
    }

    pthread_mutex_unlock( &( self->p->m ) );

    return start;
}

QueueNode* Queue_get_head_node( struct Queue* self ) {
    /* TODO: check header validity */
    return self->p->head;
}

QueueNode* Queue_get_next( struct Queue* self, QueueNode* current ) {
    if ( self == NULL || self->p == NULL ) {
        return NULL;
    }

    pthread_mutex_lock( &( self->p->m ) );

    if ( current != NULL && current->parent == self->p ) {
        current = current->next;
    }

    pthread_mutex_unlock( &( self->p->m ) );

    return current;
}

void* Queue_get_value( struct Queue* self, QueueNode* current ) {
// #ifdef DEBUG
//     printf( "[%s] current addr %p\n", __func__, current );
// #endif
    return current ? current->value : NULL;
}

void Queue_live( struct Queue* self, uint8_t event,
                 void ( *handler )( struct Queue* self ) ) {
    if ( self == NULL || self->p == NULL ) {
        return;
    }

    pthread_mutex_lock( &( self->p->m ) );

    self->p->handler_num++;
    self->p->handlers = (handler_t*)realloc(
        self->p->handlers, self->p->handler_num * sizeof( handler_t ) );
    self->p->handlers[self->p->handler_num - 1].event   = event;
    self->p->handlers[self->p->handler_num - 1].handler = handler;

    pthread_mutex_unlock( &( self->p->m ) );

    return;
}

void Queue_call( struct Queue* self, uint8_t event ) {
    for ( size_t i = 0; i < self->p->handler_num; i++ ) {
        if ( self->p->handlers[i].event | event ) {
            ( self->p->handlers[i].handler )( self );
        }
    }

    return;
}
