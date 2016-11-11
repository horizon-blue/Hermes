#ifndef __API_H__
#define __API_H__

#include <sys/types.h>

enum API_LIST {
    API_NOP                      = 0,
    API_GET_REMOTE_FILE_LIST     = 65,
    API_RESPOSE_REMOTE_FILE_LIST = 66,
};

void  api_initialize();
int   api_clear();
char* api_get_key( char* key );
int api_set_key( const char* key, const char* value );
int api_generator( char** buffer, ssize_t* length, char api );
char api_parser( char* buffer, ssize_t length );

#endif
