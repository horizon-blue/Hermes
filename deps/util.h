#ifndef __UTIL_H__
#define __UTIL_H__

#include <inttypes.h>
#include <stdint.h>

uint64_t get_timestamp();
void     clear_screen();

char * base64_encode ( const unsigned char * data,
                       size_t input_length,
                       size_t * output_length );
unsigned char * base64_decode ( const char * data,
                                size_t input_length,
                                size_t * output_length );

#endif
