#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <unistd.h>     /* for close() */

typedef struct Socket {
    int                sock;
    struct sockaddr_in server_addr;
    int ( *create )( struct Socket* self, const char* ip, unsigned short port );
    int ( *connect )( struct Socket* self );
    int ( *send )( struct Socket* self, const char* buffer, size_t length );
    int ( *receive )( struct Socket* self );
    int ( *bind )();
    int ( *listen )();
    int ( *accept )();
    int ( *close )( struct Socket* self );
    int ( *destroy )( struct Socket* self );
} Socket;

Socket* initialize_socket( Socket* ptr );

#endif
