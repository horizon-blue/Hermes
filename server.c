#include <stdio.h>

#include "socket.h"
#include "util.h"

#include "server.h"

int main( int argc, char* agrv[] ) {
    fprintf( stderr, "%llu: starting server...\n", get_timestamp() );
    Socket _s;
    initialize_socket( &_s );
    _s.create( &_s, "127.0.0.1", 80 );
    _s.connect( &_s );
    _s.send( &_s, "test", 5 );

    return 0;
}
