#ifndef DIRP_H
#define DIRP_H

// dirp
typedef struct { 
    php_stream_context *context;
    php_stream *stream;
    zval *zcontext;
} dirp;

dirp* dirp_open(char * dirname);
zval* dirp_scandir_with_func( dirp * dirp, char* dirname, int dirname_len, char* (*func)(char*, int, php_stream_dirent*) );
void dirp_close( dirp * dirp );

char* dirp_scandir_entry_handler( char* dirname, int dirname_len, php_stream_dirent * entry);

#endif
