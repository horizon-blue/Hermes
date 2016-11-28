#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

extern int errno;

void exit_with_error( const char* func, char* message ) {
    fprintf( stderr, "[%s] %s : %s\n", func, message, strerror( errno ) );
    exit( 1 );
}
