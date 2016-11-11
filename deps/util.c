/* Util.c
 * mKilo
 * Rijn
 */

/* Assert macro */
#if !defined( _POSIX_C_SOURCE ) && _POSIX_C_SOURCE < 199309L
#define _POSIX_C_SOURCE 199309L
#endif
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#include <assert.h>
#ifndef __MACH__
#include <features.h>
#endif
#define _GNU_SOURCE /* See feature_test_macros(7) */
#include <dirent.h>
#include <fcntl.h> /* Obtain O_* constant definitions */
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dirent.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

/* fix DT_DIR in osx */
#if !defined( DT_DIR )
#define DT_DIR 4
#endif

/* missing strdup in osx */
#ifdef __MACH__
char *my_strdup( const char *s ) {
    char *p = (char *)malloc( strlen( s ) + 1 );
    if ( p ) {
        memcpy( p, s, strlen( s ) + 1 );
    }
    return p;
}

#define strdup( x ) my_strdup( x )
#endif

#include "error.h"

#include "util.h"

uint64_t get_timestamp() {
    struct timespec ts;

/*
 *https://gist.github.com/jbenet/1087739
 */
#ifdef __MACH__  // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t    cclock;
    mach_timespec_t mts;
    host_get_clock_service( mach_host_self(), CALENDAR_CLOCK, &cclock );
    clock_get_time( cclock, &mts );
    mach_port_deallocate( mach_task_self(), cclock );
    ts.tv_sec  = mts.tv_sec;
    ts.tv_nsec = mts.tv_nsec;
#else
    assert( _POSIX_C_SOURCE >= 199309L );
    clock_gettime( CLOCK_REALTIME, &ts );
#endif

    return 1000000000L * ts.tv_sec + ts.tv_nsec;
}

void clear_screen() {
    const char *CLEAR_SCREE_ANSI = "\033[1;1H\033[2J";
    write( STDOUT_FILENO, CLEAR_SCREE_ANSI, 12 );
}

static char encoding_table[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};
static char *decoding_table = NULL;
static int   mod_table[]    = {0, 2, 1};

void build_decoding_table() {
    decoding_table = malloc( 256 );

    for ( int i                                          = 0; i < 64; i++ )
        decoding_table[(unsigned char)encoding_table[i]] = i;
}

void base64_cleanup() {
    free( decoding_table );
}

char *base64_encode( const unsigned char *data, size_t input_length,
                     size_t *output_length ) {
    *output_length = 4 * ( ( input_length + 2 ) / 3 );

    char *encoded_data = malloc( *output_length );
    if ( encoded_data == NULL ) return NULL;

    for ( size_t i = 0, j = 0; i < input_length; ) {
        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = ( octet_a << 0x10 ) + ( octet_b << 0x08 ) + octet_c;

        encoded_data[j++] = encoding_table[( triple >> 3 * 6 ) & 0x3F];
        encoded_data[j++] = encoding_table[( triple >> 2 * 6 ) & 0x3F];
        encoded_data[j++] = encoding_table[( triple >> 1 * 6 ) & 0x3F];
        encoded_data[j++] = encoding_table[( triple >> 0 * 6 ) & 0x3F];
    }

    for ( int i = 0; i < mod_table[input_length % 3]; i++ )
        encoded_data[*output_length - 1 - i] = '=';

    return encoded_data;
}

unsigned char *base64_decode( const char *data, size_t input_length,
                              size_t *output_length ) {
    if ( decoding_table == NULL ) build_decoding_table();

    if ( input_length % 4 != 0 ) return NULL;

    size_t *length = malloc( sizeof( size_t ) );

    *length = input_length / 4 * 3;
    if ( data[input_length - 1] == '=' ) ( *length )--;
    if ( data[input_length - 2] == '=' ) ( *length )--;

    unsigned char *decoded_data = malloc( *length + 1 );
    if ( decoded_data == NULL ) return NULL;

    for ( size_t i = 0, j = 0; i < input_length; ) {
        uint32_t sextet_a =
            data[i] == '=' ? 0 & i++ : (unsigned)decoding_table[(int)data[i++]];
        uint32_t sextet_b =
            data[i] == '=' ? 0 & i++ : (unsigned)decoding_table[(int)data[i++]];
        uint32_t sextet_c =
            data[i] == '=' ? 0 & i++ : (unsigned)decoding_table[(int)data[i++]];
        uint32_t sextet_d =
            data[i] == '=' ? 0 & i++ : (unsigned)decoding_table[(int)data[i++]];

        uint32_t triple = ( sextet_a << 3 * 6 ) + ( sextet_b << 2 * 6 ) +
                          ( sextet_c << 1 * 6 ) + ( sextet_d << 0 * 6 );

        if ( j < *length ) decoded_data[j++] = ( triple >> 2 * 8 ) & 0xFF;
        if ( j < *length ) decoded_data[j++] = ( triple >> 1 * 8 ) & 0xFF;
        if ( j < *length ) decoded_data[j++] = ( triple >> 0 * 8 ) & 0xFF;
    }

    decoded_data[*length] = '\0';

    if ( output_length != NULL ) {
        *output_length = *length;
    }
    free( length );

    return decoded_data;
}

static int file_name_cmp( const void *f1, const void *f2 ) {
    return strcmp( *(char *const *)f1, *(char *const *)f2 );
}
// not thread safe
char **get_file_list( const char *base_directory ) {
    DIR *the_directory = opendir( base_directory );
    if ( !the_directory ) return NULL;
    // loop twice, first count the number of files,
    // then create the array to store the file names

    size_t file_counts = 0;
    while ( readdir( the_directory ) ) ++file_counts;

    rewinddir( the_directory );  // back to beginning

    struct dirent *curr_file = NULL;
    char **        name_of_files =
        (char **)malloc( sizeof( char * ) * ( file_counts + 1 ) );
    size_t index = 0;
    while ( ( curr_file = readdir( the_directory ) ) ) {
        if ( curr_file->d_type == DT_DIR ) {
            size_t name_len      = strlen( curr_file->d_name );
            name_of_files[index] = (char *)malloc( name_len + 2 );
            memcpy( name_of_files[index], curr_file->d_name, name_len );
            name_of_files[index][name_len]     = '/';  // for directory
            name_of_files[index][name_len + 1] = '\0';
        } else
            name_of_files[index] = strdup( curr_file->d_name );
        ++index;
    }
    name_of_files[file_counts] = (char *)NULL;

    qsort( name_of_files, file_counts, sizeof( char * ), file_name_cmp );
    return name_of_files;
}

/* merge array to string */
char *str_implode( char separator, char **array ) {
    char *  str = NULL;
    ssize_t len = 0;
    for ( char **ptr = array; *ptr != NULL; ptr++ ) {
        str = (char *)realloc( str, len + strlen( *ptr ) + 1 );
        memcpy( str + len, *ptr, strlen( *ptr ) );
        len += strlen( *ptr );
        str[len++] = separator;
    }
    str      = (char *)realloc( str, len + 1 );
    str[len] = '\0';
    return str;
}

/* split string to array */
char **str_explode( char separator, char *string ) {
    ssize_t index = 0;

    char **array = NULL;
    char * next  = NULL;

    while ( ( next = strchr( string, separator ) ) != NULL &&
            *( string ) != '\0' ) {
        array = (char **)realloc( array, ( index + 1 ) * sizeof( char * ) );
        uint64_t len = (uint64_t)next - (uint64_t)string;
        array[index] = (char *)malloc( len + 1 );
        memcpy( array[index], string, len );
        array[index][len] = '\0';
        string = next + 1;
        index++;
    }

    array        = (char **)realloc( array, ( index + 1 ) * sizeof( char * ) );
    array[index] = (char *)NULL;

    return array;
}
