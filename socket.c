#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "error.h"

#include "socket.h"

int Socket_create( Socket* self, const char* ip, unsigned short port );
int Socket_connect( Socket* self );
int Socket_send( Socket* self, const char* buffer, size_t length );
int Socket_receive( Socket* self );
int Socket_bind();
int Socket_listen();
int Socket_accept();
int Socket_close( Socket* self );
int Socket_destroy( Socket* self );

Socket* initialize_socket( Socket* ptr ) {
    if ( !ptr ) ptr = (Socket*)malloc( sizeof( Socket ) );
    memset( ptr, 0, sizeof( Socket ) );

    ptr->create  = Socket_create;
    ptr->connect = Socket_connect;
    ptr->send    = Socket_send;
    ptr->receive = Socket_receive;
    ptr->bind    = Socket_bind;
    ptr->listen  = Socket_listen;
    ptr->accept  = Socket_accept;
    ptr->close   = Socket_close;
    ptr->destroy = Socket_destroy;

    return ptr;
}

/* create a stream socket */
int Socket_create( Socket* self, const char* ip, unsigned short port ) {
    /* Create a reliable, stream socket using TCP */
    if ( ( self->sock = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP ) ) < 0 )
        exit_with_error( __func__, "socket() failed" );

    /* Construct the server address structure */
    memset( &self->server_addr, 0,
            sizeof( self->server_addr ) );  /* Zero out structure */
    self->server_addr.sin_family = AF_INET; /* Internet address family */
    self->server_addr.sin_addr.s_addr =
        ip == NULL ? htonl( INADDR_ANY )
                   : inet_addr( ip );           /* Server IP address */
    self->server_addr.sin_port = htons( port ); /* Server port */

    return 0;
}

/* Establish the connection to the server */
int Socket_connect( Socket* self ) {
    if ( connect( self->sock, (struct sockaddr*)&( self->server_addr ),
                  sizeof( self->server_addr ) ) < 0 )
        exit_with_error( __func__, "connect() failed" );
    return 0;
}

/* send data */
int Socket_send( Socket* self, const char* buffer, size_t length ) {
    char* ptr = (char*)buffer;
    while ( length > 0 ) {
        int i = 0;
#ifdef DEBUG
        fprintf( stderr, "[%s] send remain: %s\n", __func__, ptr );
#endif
        if ( ( i = send( self->sock, ptr, length, 0 ) ) < 0 )
            exit_with_error( __func__, "send() failed" );
        if ( i < 1 ) return 1;
        ptr += i;
        length -= i;
    }
    return 0;
}

/* receive data */
#define RCVBUFSIZE 32
int Socket_receive( Socket* self ) {
    ssize_t bytes_received;
    char    buffer[RCVBUFSIZE];
    if ( ( bytes_received = recv( self->sock, buffer, RCVBUFSIZE - 1, 0 ) ) <=
         0 )
        exit_with_error( __func__,
                         "recv() failed or connection closed prematurely" );
#ifdef DEBUG
    buffer[bytes_received - 1] = '\0';
    fprintf( stderr, "[%s] received: %s\n", __func__, buffer );
#endif
    return 0;
}
int Socket_bind() {
    return 0;
}
int Socket_listen() {
    return 0;
}
int Socket_accept() {
    return 0;
}
int Socket_close( Socket* self ) {
    close( self->sock );
    return 0;
}
int Socket_destroy( Socket* self ) {
    free( self );
    return 0;
}
