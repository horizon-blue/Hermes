#define _BSD_SOURCE
#define _GNU_SOURCE

#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>

#include "queue.h"
#include "socket.h"
#include "util.h"

int main ( int argc, char ** argv )
{

    printf ( "Welcome to mkilo\n" );

    printf ( "Server ip (default localhost): " );

    char * server_ip = NULL;
    ssize_t len = getline ( &server_ip, NULL, STDIN_FILENO );
    server_ip[len-1] = '\0';

    printf("[%s] server_ip [%s]\n", __func__, server_ip);

    fprintf ( stderr, "%llu: starting client...\n", get_timestamp() );

    Socket _s;
    initialize_socket ( &_s );

    _s.create ( &_s, argc > 1 ? argv[1] : "216.58.192.174", 80, 0 );
    _s.connect ( &_s );

    char * request = "GET / HTTP/1.1\r\nHost: \r\n\r\n";
    _s.send ( &_s, request, strlen ( request ) );

    _s.receive ( &_s );

    _s.close ( &_s );

    return 0;
}