#ifndef __UTIL_H__
#define __UTIL_H__

#include <inttypes.h>
#include <stdint.h>

uint64_t get_timestamp();
void     clear_screen();

char* base64_encode( const unsigned char* data, size_t input_length,
                     size_t* output_length );
unsigned char* base64_decode( const char* data, size_t input_length,
                              size_t* output_length );

char** get_file_list( const char* base_directory );

/* merge array to string */
char* str_implode( char separator, char** array );

/* split string to array */
char** str_explode( char separator, char* string );

#endif
