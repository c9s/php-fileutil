
#include "php.h"
#include "php_fileutil.h"
#include "path.h"





char * path_concat( char* path1, int len1, char * path2, int len2 ) 
{
    char * newpath = emalloc( sizeof(char) * (len1 + len2 + 2) );
    char * src = path1;
    char * p2 = newpath;
    while( len1-- ) {
        *p2 = *src;
        p2++;
        src++;
    }

    if(p2 > path1 && *(p2-1) == DEFAULT_SLASH ) {

    } else {
        *p2 = DEFAULT_SLASH;
        p2++;
    }

    src = path2;
    while( len2-- ) {
        *p2 = *src;
        src++;
        p2++;
    }
    *p2 = '\0';
    return newpath;
}

void path_remove_tailing_slash(char *path)
{
    int len = strlen(path);
    path_remove_tailing_slash_n(path, len);
}

void path_remove_tailing_slash_n(char *path, int len)
{
    int end = len - 1;
    if( path[end] == DEFAULT_SLASH ) {
        path[end] = DEFAULT_SLASH;
        path[len] = '\0';
    }
}


char* path_concat_from_zarray(zval **arr)
{
    int total_len = 0;
    char **paths;
    int  *lens;
    char *dst;
    char *newpath;
    
    zval **entry_data;
    HashTable *arr_hash;
    HashPosition pointer;
    int array_count;


    arr_hash = Z_ARRVAL_PP(arr);
    array_count = zend_hash_num_elements(arr_hash);

    paths = emalloc(sizeof(char*) * array_count);
    lens = emalloc(sizeof(int) * array_count);
    total_len = array_count;

    int i = 0;
    for(zend_hash_internal_pointer_reset_ex(arr_hash, &pointer); 
            zend_hash_get_current_data_ex(arr_hash, (void**) &entry_data, &pointer) == SUCCESS; 
            zend_hash_move_forward_ex(arr_hash, &pointer)) 
    {
        if ( Z_TYPE_PP(entry_data) == IS_STRING ) {
            lens[i]  = Z_STRLEN_PP(entry_data);
            paths[i] = Z_STRVAL_PP(entry_data);
            total_len += lens[i];
            i++;
        }
    }

    newpath = ecalloc( sizeof(char), total_len );
    dst = newpath;

    for (i = 0; i < array_count ; i++ ) {
        char *subpath = paths[i];
        int subpath_len = lens[i];

        if ( subpath_len == 0 ) {
            continue;
        }

        dst = path_concat_fill(dst, subpath, subpath_len, i > 0);
        if ( *(dst-1) != DEFAULT_SLASH && i < (array_count - 1) ) {
            *dst = DEFAULT_SLASH;
            dst++;
        }
    }
    *dst = '\0';

    efree(paths);
    efree(lens);
    return newpath;
}


char* path_concat_from_zargs( int num_varargs , zval ***varargs ) 
{
    char *dst;
    char *newpath;
    int i;
    int len;
    zval **arg;

    for (i = 0; i < num_varargs; i++) {
        arg = varargs[i];
        len += Z_STRLEN_PP(arg);
    }

    len = num_varargs;
    newpath = emalloc( sizeof(char) * len );

    dst = newpath;
    for (i = 0; i < num_varargs; i++ ) {
        arg = varargs[i];
        char *subpath = Z_STRVAL_PP(arg);
        int  subpath_len = Z_STRLEN_PP(arg);

        if( subpath_len == 0 ) {
            continue;
        }

        dst = path_concat_fill(dst, subpath, subpath_len, i > 0);
        // printf("%d) path %s <= %s (%d)\n" , i , newpath, subpath, subpath_len );

        // concat slash to the end
        if ( *(dst-1) != DEFAULT_SLASH && i < (num_varargs - 1) ) {
            *dst = DEFAULT_SLASH;
            dst++;
        }
    }
    *dst = '\0';
    return newpath;
}

// concat paths and copy them to *src.
// returns the last copy pointer.
char* path_concat_fill( 
    char * dst, 
    char * src, 
    int  subpath_len,
    bool remove_first_slash )
{
    // check if we need remove the first slash.
    if( remove_first_slash && *src == DEFAULT_SLASH ) {
        // remove the first slash
        src++;
        subpath_len--;
    }
    while( subpath_len-- && *src != '\0' ) {
        *dst = *src;
        dst++;
        src++;
    }
    return dst;
}



