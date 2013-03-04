#ifndef PHP_FILEUTIL_PATH_H
#define PHP_FILEUTIL_PATH_H

char * path_concat( 
        char* path1, 
        int len1, 
        char * path2 , int len2 );

char* path_concat_fill( 
        char * dst, 
        char * src, 
        int  subpath_len,
        bool remove_first_slash );

char* path_concat_from_zargs( int num_varargs , zval ***varargs );

void path_remove_tailing_slash(char *path);

void path_remove_tailing_slash_a2(char *path, int len);

#endif
