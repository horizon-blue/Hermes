#include <stdio.h>

#include "socket.h"
#include "util.h"

#include "server.h"

bool echo( Socket* self, Socket* client ) {
#ifdef DEBUG
    fprintf(stderr, "tp");
#endif
    client->receive( client );
    return false;
}

int main( int argc, char* argv[] ) {
    fprintf( stderr, "%llu: starting server...\n", get_timestamp() );

    Socket _s;
    initialize_socket( &_s );

    _s.create( &_s, (char*)NULL, argc > 1 ? atoi( argv[1] ) : 9999, 10 );

    _s.bind( &_s );
    _s.listen( &_s );

    _s.handle( &_s, (void*)echo );

    for ( ;; )
        ;

    return 0;
}
