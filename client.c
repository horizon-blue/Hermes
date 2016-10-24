#include <stdio.h>
#include <string.h>

#include "socket.h"
#include "util.h"

#include "client.h"

int main( int argc, char* agrv[] ) {
    fprintf( stderr, "%llu: starting client...\n", get_timestamp() );

    Socket _s;
    initialize_socket( &_s );

    _s.create( &_s, "216.58.192.174", 80 );
    _s.connect( &_s );

    char* request = "GET / HTTP/1.1\r\nHost: \r\n\r\n";
    _s.send( &_s, request, strlen( request ) );

    _s.receive( &_s );

    _s.close( &_s );

    return 0;
}
