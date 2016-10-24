#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "error.h"

#include "socket.h"

typedef uint8_t bool;

int Socket_create( Socket* self, const char* ip, unsigned short port,
                   int max_connection );
int Socket_connect( Socket* self );
int Socket_send( Socket* self, const char* buffer, size_t length );
int Socket_receive( Socket* self );
int Socket_bind( Socket* self );
int Socket_listen( Socket* self );
int Socket_accept( Socket* self );
int Socket_handle( Socket* self, void* handler );
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
    ptr->handle  = Socket_handle;
    ptr->close   = Socket_close;
    ptr->destroy = Socket_destroy;

    return ptr;
}

/* create a stream socket */
int Socket_create( Socket* self, const char* ip, unsigned short port,
                   int max_connection ) {
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

/* Bind to the local address */
int Socket_bind( Socket* self ) {
    if ( bind( self->sock, (struct sockaddr*)&( self->server_addr ),
               sizeof( self->server_addr ) ) < 0 )
        exit_with_error( __func__, "bind() failed" );
    return 0;
}

/* Mark the socket so it will listen for incoming connections */
int Socket_listen( Socket* self ) {
    if ( listen( self->sock, self->max_connection ) < 0 )
        exit_with_error( __func__, "listen() failed" );
    return 0;
}

int Socket_accept( Socket* self ) {
    return 0;
}

typedef struct Socket_response_thread_t {
    pthread_t pid;
    Socket* self;
    Socket* client;
    bool ( *handler )( Socket* self, Socket* client );
} Socket_response_thread_t;

void* Socket_response_thread( void* info ) {
    Socket_response_thread_t* ptr = (Socket_response_thread_t*)info;
    while ( ( ptr->handler )( ptr->self, ptr->client ) )
        ;
    return NULL;
}

typedef struct Socket_handle_thread_t {
    Socket* self;
    void*   handler;
} Socket_handle_thread_t;

void* Socket_handle_thread( void* info ) {
    Socket_handle_thread_t* ptr = (Socket_handle_thread_t*)info;
    size_t                  clntLen;
    Socket                  clientSocket;
    initialize_socket( &clientSocket );

    Socket_response_thread_t* clients = NULL;
    size_t client_number = 0;

    for ( ;; ) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        clntLen = sizeof( clientSocket.server_addr );

        /* Wait for a client to connect */
        if ( ( clientSocket.sock =
                   accept( ptr->self->sock,
                           (struct sockaddr*)&( ptr->self->server_addr ),
                           (socklen_t*)&clntLen ) ) < 0 )
            exit_with_error( __func__, "accept() failed" );

        /* clntSock is connected to a client! */

        printf( "Handling client %s\n",
                inet_ntoa( clientSocket.server_addr.sin_addr ) );

        client_number++;
        //clients = (Socket_response_thread_t*);

        //( ptr->handler )( ptr->self, &clientSocket );
        //info->handler = (bool ( * )( Socket * self, Socket * client ))handler;
    }
    return NULL;
}

int Socket_handle( Socket* self, void* handler ) {
    Socket_handle_thread_t* info =
        (Socket_handle_thread_t*)malloc( sizeof( Socket_handle_thread_t ) );
    memset( info, 0, sizeof( Socket_handle_thread_t ) );
    info->self    = self;
    info->handler = handler;

    pthread_t pid;
    return pthread_create( &pid, 0, Socket_handle_thread, info );
}

int Socket_close( Socket* self ) {
    close( self->sock );
    return 0;
}

int Socket_destroy( Socket* self ) {
    free( self );
    return 0;
}
