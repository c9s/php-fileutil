#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_fileutil.h"
#include "dirp.h"
#include "path.h"

#include <Zend/zend.h>
#include <ext/standard/php_standard.h>
#include <ext/standard/php_filestat.h>

ZEND_BEGIN_ARG_INFO_EX(arginfo_futil_scandir, 0, 0, 1)
    ZEND_ARG_INFO(0, dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_futil_scandir_dir, 0, 0, 1)
    ZEND_ARG_INFO(0, dir)
ZEND_END_ARG_INFO()

static const zend_function_entry fileutil_functions[] = {
    PHP_FE(futil_scandir, arginfo_futil_scandir)
    PHP_FE(futil_scandir_dir, arginfo_futil_scandir_dir)
    PHP_FE(futil_join, NULL)
    {NULL, NULL, NULL}
};


zend_module_entry fileutil_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_FILEUTIL_EXTNAME,
    fileutil_functions,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#if ZEND_MODULE_API_NO >= 20010901
    PHP_FILEUTIL_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_FILEUTIL
ZEND_GET_MODULE(fileutil)
#endif


bool futil_stream_is_dir(php_stream *stream)
{
    return (stream->flags & PHP_STREAM_FLAG_IS_DIR);
}

bool futil_is_dir(char* dirname, int dirname_len)
{
    zval tmp;
    php_stat(dirname, dirname_len, FS_IS_DIR, &tmp TSRMLS_CC);
    return Z_LVAL(tmp) ? true : false;
}

/*char* path_concat_from_zarray() 
{
    return '';
}*/



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



PHP_FUNCTION(futil_scandir_dir)
{
    dirp *dirp;
    zval *z_list;
    char *dirname;
    int dirname_len;

    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                    &dirname, &dirname_len ) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Path argument is required.");
        RETURN_FALSE;
    }

    // run is_dir
    if( ! futil_is_dir(dirname, dirname_len) ) {
        RETURN_FALSE;
    }

    dirp = dirp_open(dirname);
    if( dirp == NULL ) {
        RETURN_FALSE;
    }

    z_list = dirp_scandir_with_handler(dirp, 
            dirname, dirname_len, 
            dirp_dir_entry_handler );

    *return_value = *z_list;
    // add reference count
    zval_copy_ctor(return_value);

    // closedir
    // rsrc_id = dirp->rsrc_id;
    dirp_close(dirp);
}

PHP_FUNCTION(futil_scandir)
{
    dirp *dirp;
    zval *z_list;
    char *dirname;
    int dirname_len;


    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                    &dirname, &dirname_len ) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong parameters.");
        RETURN_FALSE;
    }

    // run is_dir
    if( ! futil_is_dir(dirname, dirname_len) ) {
        RETURN_FALSE;
    }

    dirp = dirp_open(dirname);
    if( dirp == NULL ) {
        RETURN_FALSE;
    }

    z_list = dirp_scandir_with_handler(dirp, dirname, dirname_len, dirp_entry_handler );

    *return_value = *z_list;
    // add reference count
    zval_copy_ctor(return_value);

    // closedir
    // rsrc_id = dirp->rsrc_id;
    dirp_close(dirp);
}


PHP_FUNCTION(futil_join)
{
    int num_varargs;
    zval ***varargs = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "*",
                    &varargs, &num_varargs
                    ) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong parameters.");
        RETURN_FALSE;
    }

    char *newpath;

    if ( num_varargs == 1 && Z_TYPE_PP(varargs[0]) == IS_ARRAY ) 
    {

        int total_len = 0;
        char **paths;
        int  *lens;
        char *dst;
        
        zval **arr = varargs[0];
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
            if ( *(dst-1) != DEFAULT_SLASH && i < (num_varargs - 1) ) {
                *dst = DEFAULT_SLASH;
                dst++;
            }
        }
        *dst = '\0';
        efree(paths);
        efree(lens);

    } 
    else if ( num_varargs > 1  && Z_TYPE_PP(varargs[0]) == IS_STRING ) 
    {
        newpath = path_concat_from_zargs( num_varargs , varargs );
    } 
    else 
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong parameters.");
    }

    if (varargs) {
        efree(varargs);
    }

    RETVAL_STRING(newpath,1);
}

