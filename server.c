#include <inttypes.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "api.h"
#include "queue.h"
#include "socket.h"
#include "util.h"

#include "server.h"

static volatile int keep_running = 1;

void int_handler( int dummy ) {
    keep_running = 0;
}

static Queue _q;

typedef struct info_t {
    uint64_t       timestamp;
    char*          buffer;
    size_t         length;
    struct in_addr addr;
} info_t;

typedef union {
    uint8_t ( *funcptr )( Socket* self, Socket* client );
    unsigned char* objptr;
} echo_u;

uint8_t echo( Socket* self, Socket* client ) {
    static char* buffer = NULL;
    size_t       length;

    if ( client->receive( client, &buffer, &length ) ) {
        return 0;
    }

#ifdef DEBUG
    fprintf( stderr, "[%s] received: %s\n", __func__, buffer );
#endif

    if ( !buffer ) {
        return 1;
    }

    if ( memcmp( buffer, "exit", 4 ) == 0 ) {
        client->send( client, "\nGoodbye.\r\n", 11 );
        client->close( self, client );

        self->broadcast( self, self, "\nSomeone left.\r\n", 16 );
        return 1;
    }

    info_t* _i    = (info_t*)malloc( sizeof( info_t ) );
    _i->timestamp = get_timestamp();
    _i->buffer    = (char*)malloc( ( length ) * sizeof( char ) );
    memcpy( _i->buffer, buffer, length );
    _i->length = length;
    memcpy( &( _i->addr ), &( client->server_addr.sin_addr ),
            sizeof( struct in_addr ) );

    self->broadcast( self, self, buffer, length );

    _q.push( &_q, (void*)_i );

    return 1;
}

void monitor( Queue* q ) {
    // clear_screen();
    printf( "[%s] Queue size = %lu\n", __func__, q->size( q ) );
    printf( "----Queue----\n" );
    void* head = q->get_head_node( q );

    for ( ; head != NULL; head = q->get_next( q, head ) ) {
        if ( head ) {
            info_t* msg = (info_t*)q->get_value( q, head );
            fflush( stdout );

            if ( msg )
                printf( "%s \t %llu: %s\n", inet_ntoa( msg->addr ),
                        (long long unsigned int)msg->timestamp, msg->buffer );
        }
    }

    printf( "-------------\n" );
    return;
}

void processor( Queue* q ) {
#ifdef DEBUG
    printf( "[%s] resume\n", __func__ );
#endif
    info_t* command = (info_t*)q->bottom( q );

    switch ( api_parser( command->buffer, command->length ) ) {
        case API_GET_REMOTE_FILE_LIST:
            printf( "[%s] fetching file list from %s\n", __func__,
                    api_get_key( "BASE" ) );
            break;
    }
#ifdef DEBUG
    printf( "[%s] halt\n", __func__ );
#endif
}

int main( int argc, char* argv[] ) {
    fprintf( stderr, "%llu: starting server...\n",
             (long long unsigned int)get_timestamp() );

    signal( SIGINT, int_handler );

    initialize_queue( &_q );
    _q.live( &_q, QUEUE_PUSH, monitor );
    _q.live( &_q, QUEUE_PUSH, processor );

    Socket _s;
    initialize_socket( &_s );

    _s.create( &_s, (char*)NULL, argc > 1 ? atoi( argv[1] ) : 9999, 10 );

    _s.bind( &_s );
    _s.listen( &_s );

    echo_u _u;
    _u.funcptr = echo;
    _s.handle( &_s, _u.objptr );

    while ( keep_running ) {
        sleep( 1 );
    }

    /* TODO destroy handler thread */

    return 0;
}
