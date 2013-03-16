
#include "php.h"
#include "php_fileutil.h"
#include "path.h"
#include <Zend/zend.h>
#include <ext/standard/php_standard.h>
#include <ext/standard/php_filestat.h>


PHPAPI char * path_concat( char* path1, int len1, char * path2, int len2 TSRMLS_DC) 
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

PHPAPI void path_remove_tailing_slash(char *path TSRMLS_DC)
{
    int len = strlen(path);
    path_remove_tailing_slash_n(path, len TSRMLS_CC);
}

PHPAPI void path_remove_tailing_slash_n(char *path, int len TSRMLS_DC)
{
    int end = len - 1;
    if( path[end] == DEFAULT_SLASH ) {
        path[end] = DEFAULT_SLASH;
        path[len] = '\0';
    }
}


PHPAPI char* path_concat_from_zarray(zval **arr TSRMLS_DC)
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

        dst = path_concat_fill(dst, subpath, subpath_len, i > 0 TSRMLS_CC);
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



// concat paths and copy them to *src.
// returns the last copy pointer.
PHPAPI char* path_concat_fill( 
    char * dst, 
    char * src, 
    int  subpath_len,
    bool remove_first_slash TSRMLS_DC)
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



