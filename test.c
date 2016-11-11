#include <stdio.h>

#include "util.h"

void test_get_timestamp() {
    printf( "%s\n", __func__ );

    for ( int i = 0; i < 5; i++ ) {
        printf( "[%s] %llu\n", __func__, (unsigned long long)get_timestamp() );
    }

    return;
}

void test_get_file_list() {
    printf( "%s\n", __func__ );
    return;
}

int main( int argc, char* argv[] ) {
    test_get_timestamp();
    test_get_file_list();

    printf( "[%s] done\n", __func__ );
    return 0;
}
