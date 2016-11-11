#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "api.h"

typedef struct pair_t
{
    char * key;
    char * value;
} pair_t;

static pair_t * items;
static ssize_t item_num;

#define ITEM_SEPERATOR '@'
#define START_END_INDICATOR '*'
#define KEY_VALUE_SEPERATOR '='

void api_initialize()
{
    items = NULL;
    item_num = 0;
}

int api_clear()
{
    /* TODO: clear storage */
    items = NULL;
    item_num = 0;

    return 0;
}

char * api_get_key ( char * key )
{
    return NULL;
}

int api_set_key ( const char * key, const char * value )
{
    item_num++;
    items = ( pair_t * ) realloc ( items, item_num * sizeof ( pair_t ) );

    items[item_num - 1].key = malloc ( strlen ( key ) + 1 );
    strcpy ( items[item_num - 1].key, key );

    items[item_num - 1].value = malloc ( strlen ( value ) + 1 );
    strcpy ( items[item_num - 1].value, value );

    return 0;
}

int api_generator ( char ** buffer, ssize_t * length, char api )
{
    ssize_t len = 2;
    *buffer = realloc ( *buffer, len );
    ( *buffer ) [0] = START_END_INDICATOR;
    ( *buffer ) [1] = api;

    for ( ssize_t i = 0; i < item_num; i++ )
    {
        /* TODO: Encrypt data */
        char * key = items[i].key;
        char * value = items[i].value;
        *buffer = realloc ( *buffer, len + strlen ( key ) + strlen ( value ) + 2 );
        ( *buffer ) [len++] = ITEM_SEPERATOR;
        memcpy ( *buffer + len, key, strlen ( key ) );
        len += strlen ( key );
        ( *buffer ) [len++] = KEY_VALUE_SEPERATOR;
        memcpy ( *buffer + len, value, strlen ( value ) );
        len += strlen ( value );
    }

    *buffer = realloc ( *buffer, len + 2 );
    ( *buffer ) [len++] = START_END_INDICATOR;
    ( *buffer ) [len] = '\0';

    *length = len;
    return 0;
}

char api_parser ( char * buffer, ssize_t length )
{
	length -= 1;
	#ifdef DEBUG
	printf("\t[%s] buffer[0] = %c, len = %d, buffer[len] = %c\n", 
		__func__, buffer[0], (int)length, buffer[length-1]);
	#endif
    if ( buffer == NULL || buffer[0] != START_END_INDICATOR
            || buffer[length-1] != START_END_INDICATOR )
    {
        return API_NOP;
    }

    api_clear();

    return buffer[1];
}
