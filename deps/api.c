#if !defined( _POSIX_C_SOURCE ) && _POSIX_C_SOURCE < 200809L
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "util.h"

#include "api.h"

typedef struct pair_t {
    char* key;
    char* value;
} pair_t;

static pair_t* items;
static ssize_t item_num;

#define ITEM_SEPERATOR '@'
#define START_END_INDICATOR '*'
#define KEY_VALUE_SEPERATOR '`'

void api_initialize() {
    items    = NULL;
    item_num = 0;
}

int api_clear() {
    for ( ssize_t i = 0; i < item_num; i++ ) {
        free( items[i].key );
        free( items[i].value );
    }

    free( items );
    items    = NULL;
    item_num = 0;

    return 0;
}

char* api_get_key( char* key ) {
    for ( ssize_t i = 0; i < item_num; i++ ) {
        if ( strcmp( key, items[i].key ) == 0 ) {
            return items[i].value;
        }
    }

    return NULL;
}

int api_set_key( const char* key, const char* value ) {
    item_num++;
    items = (pair_t*)realloc( items, item_num * sizeof( pair_t ) );

    items[item_num - 1].key = malloc( strlen( key ) + 1 );
    strcpy( items[item_num - 1].key, key );

    items[item_num - 1].value = malloc( strlen( value ) + 1 );
    strcpy( items[item_num - 1].value, value );

    return 0;
}

int api_generator( char** buffer, ssize_t* length, char api ) {
    ssize_t len    = 2;
    *buffer        = realloc( *buffer, len );
    ( *buffer )[0] = START_END_INDICATOR;
    ( *buffer )[1] = api;

    for ( ssize_t i = 0; i < item_num; i++ ) {
        /* TODO: Encrypt data */
        size_t data_len = 0;
        char*  data     = NULL;

        *buffer            = realloc( *buffer, len + 1 );
        ( *buffer )[len++] = ITEM_SEPERATOR;

        data = base64_encode( (unsigned char*)items[i].key,
                              strlen( items[i].key ), &data_len );
        *buffer = realloc( *buffer, len + data_len + 1 );
        memcpy( *buffer + len, data, data_len );
        len += data_len;
        free( data );

        *buffer            = realloc( *buffer, len + 1 );
        ( *buffer )[len++] = KEY_VALUE_SEPERATOR;

        data = base64_encode( (unsigned char*)items[i].value,
                              strlen( items[i].value ), &data_len );
        *buffer = realloc( *buffer, len + data_len + 1 );
        memcpy( *buffer + len, data, data_len );
        len += data_len;
        free( data );
    }

    *buffer            = realloc( *buffer, len + 2 );
    ( *buffer )[len++] = START_END_INDICATOR;
    ( *buffer )[len]   = '\0';

    *length = len;
    return 0;
}

char api_parser( char* buffer, ssize_t length ) {
    length -= 1;
#ifdef DEBUG
    printf( "\t[%s] buffer[0] = %c, len = %d, buffer[len] = %c\n", __func__,
            buffer[0], (int)length, buffer[length - 1] );
#endif

    if ( buffer == NULL || buffer[0] != START_END_INDICATOR ||
         buffer[length - 1] != START_END_INDICATOR ) {
        return API_NOP;
    }

    api_clear();

    ssize_t ptr = 2, key_start = 0, value_start = 0;

    for ( ;; ) {
        if ( buffer[ptr] == ITEM_SEPERATOR ||
             buffer[ptr] == START_END_INDICATOR ) {
            if ( key_start != 0 ) {
                /* process last item */
                char* data = NULL;

                data = malloc( value_start - key_start );
                memcpy( data, buffer + key_start + 1,
                        value_start - key_start - 1 );
                data[value_start - key_start - 1] = '\0';

                char* key = (char*)base64_decode(
                    data, value_start - key_start - 1, (size_t*)NULL );

                free( data );

                data = malloc( ptr - value_start );
                memcpy( data, buffer + value_start + 1, ptr - value_start - 1 );
                data[ptr - value_start - 1] = '\0';

                char* value = (char*)base64_decode( data, ptr - value_start - 1,
                                                    (size_t*)NULL );

                free( data );

                api_set_key( key, value );

                free( key );
                free( value );
            }

            key_start = ptr;
        }

        if ( buffer[ptr] == KEY_VALUE_SEPERATOR ) {
            value_start = ptr;
        }

        if ( buffer[ptr] == START_END_INDICATOR ) {
            break;
        }

        ptr++;
    }

    return buffer[1];
}

char* api_divider( char** buffer, ssize_t* length ) {
    if ( buffer == NULL || *buffer == NULL ) return NULL;

    char* split = strchr( *buffer + 1, START_END_INDICATOR );
    if ( split == NULL ) return NULL;

#ifdef DEBUG
    fprintf( stderr, "[%s] old buffer = %s, len = %lu\n", __func__, *buffer,
             *length );
#endif

    ssize_t api_length = (uint64_t)split - (uint64_t)(*buffer) + 2;
    *length -= api_length;
    char* divide_api = strndup( *buffer, api_length );
    char* old   = *buffer;
    *buffer     = length == 0 ? NULL : strdup( *buffer + api_length );
    free( old );

#ifdef DEBUG
    fprintf( stderr, "[%s] new buffer = %s, len = %lu\n", __func__, *buffer,
             *length );
#endif

    return divide_api;
}
