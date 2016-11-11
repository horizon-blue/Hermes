#include <stdio.h>
#include <stdlib.h>

#include "util.h"

void test_get_timestamp( ) {
    printf("%s\n", __func__);

    for(int i = 0; i < 5; i++) {
        printf("[%s] %llu\n", __func__, (unsigned long long)get_timestamp( ));
    }

    return;
}

void test_get_file_list( ) {
    printf("%s\n", __func__);
    char** the_files = get_file_list(".");
    if(!the_files) {
        printf("\t[%s] no directory: %s\n", __func__, ".");
        return;
    }
    for(int i = 0; the_files[i]; ++i) {
        printf("\t[%s] %s\n", __func__, the_files[i]);
        free(the_files[i]);
    }
    free(the_files);

    return;
}

void test_str_implode_explode() {
    printf("%s\n", __func__);
    char** the_files = get_file_list(".");
    char* str = str_implode('&', the_files);

    printf("\t[%s] str = %s\n", __func__, str);

    char** array = str_explode('&', str);

    for(int i = 0; array[i]; ++i) {
        printf("\t[%s] %s\n", __func__, array[i]);
        free(array[i]);
    }
    free(array);

    for(int i = 0; the_files[i]; ++i) {
        free(the_files[i]);
    }
    free(the_files);
    free(str);

    return;
}

int main(int argc, char* argv[]) {
    test_get_timestamp( );
    test_get_file_list( );
    test_str_implode_explode();

    printf("[%s] done\n", __func__);
    return 0;
}
