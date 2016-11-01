#include <inttypes.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "socket.h"
#include "queue.h"
#include "util.h"

#include "server.h"

static volatile int keep_running = 1;

void int_handler( int dummy ) {
    keep_running = 0;
}

typedef union {
    uint8_t (*funcptr)(Socket* self, Socket* client);
    unsigned char *objptr;
} echo_u;

uint8_t echo( Socket* self, Socket* client ) {
    static char* buffer = NULL;
    size_t       length;
    if ( client->receive( client, &buffer, &length ) ) return 0;

#ifdef DEBUG
    fprintf( stderr, "[%s] received: %s\n", __func__, buffer );
#endif

    if ( memcmp( buffer, "exit", 4 ) == 0 ) {
        client->send( client, "\nGoodbye.\r\n", 11 );
        client->close( self, client );

        self->broadcast( self, self, "\nSomeone left.\r\n", 16 );
        return 1;
    }
    self->broadcast( self, self, buffer, length );
    return 1;
}

int main( int argc, char* argv[] ) {
    fprintf( stderr, "%llu: starting server...\n", (long long unsigned int)get_timestamp() );

    signal( SIGINT, int_handler );

    Socket _s;
    initialize_socket( &_s );

    _s.create( &_s, (char*)NULL, argc > 1 ? atoi( argv[1] ) : 9999, 10 );

    _s.bind( &_s );
    _s.listen( &_s );

    echo_u _u;
    _u.funcptr = echo;
    _s.handle( &_s, _u.objptr );

    while ( keep_running )
        ;

    /* TODO destroy handler thread */

    return 0;
}
