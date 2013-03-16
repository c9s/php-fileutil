#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_fileutil.h"
#include "path.h"

#include <Zend/zend.h>
#include <ext/standard/php_standard.h>
#include <ext/standard/php_filestat.h>


ZEND_BEGIN_ARG_INFO_EX(arginfo_futil_scanpath, 0, 0, 1)
    ZEND_ARG_INFO(0, dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_futil_scanpath_dir, 0, 0, 1)
    ZEND_ARG_INFO(0, dir)
ZEND_END_ARG_INFO()



static const zend_function_entry fileutil_functions[] = {
    PHP_FE(futil_scanpath, arginfo_futil_scanpath)
    PHP_FE(futil_scanpath_dir, arginfo_futil_scanpath_dir)
    PHP_FE(futil_pathjoin, NULL)
    PHP_FE(futil_lastmtime, NULL)
    PHP_FE(futil_lastctime, NULL)
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


char * path_concat_from_zargs( int num_varargs , zval ***varargs TSRMLS_DC)
{
    char *dst;
    char *newpath;
    int i;
    int len;
    zval **arg;

    len = num_varargs + 1;
    for (i = 0; i < num_varargs; i++) {
        arg = varargs[i];
        len += Z_STRLEN_PP(arg);
    }

    newpath = emalloc( sizeof(char) * len );

    dst = newpath;
    for (i = 0; i < num_varargs; i++ ) {
        arg = varargs[i];
        char *subpath = Z_STRVAL_PP(arg);
        int  subpath_len = Z_STRLEN_PP(arg);

        if( subpath_len == 0 ) {
            continue;
        }

        dst = path_concat_fill(dst, subpath, subpath_len, i > 0 TSRMLS_CC);

        // concat slash to the end
        if ( *(dst-1) != DEFAULT_SLASH && i < (num_varargs - 1) ) {
            *dst = DEFAULT_SLASH;
            dst++;
        }
    }
    *dst = '\0';
    return newpath;
}


char * dir_dir_entry_handler(
        char* dirname, 
        int dirname_len,
        php_stream_dirent * entry TSRMLS_DC)
{
    char * path = path_concat(dirname, dirname_len, entry->d_name, strlen(entry->d_name) TSRMLS_CC);
    int    path_len = dirname_len + strlen(entry->d_name) + 1;
    if ( ! futil_is_dir(path, path_len TSRMLS_CC) ) {
        efree(path);
        return NULL;
    }
    return path;
}

char* dir_entry_handler(
        char* dirname, 
        int dirname_len, 
        php_stream_dirent * entry TSRMLS_DC)
{
    return path_concat(dirname, dirname_len, entry->d_name, strlen(entry->d_name) TSRMLS_CC);
}


void phpdir_scandir_with_handler(
        zval * z_list,
        php_stream * stream, 
        char* dirname, 
        int dirname_len,
        char* (*handler)(char*, int, php_stream_dirent*) TSRMLS_DC) 
{
    php_stream_dirent entry;
    while (php_stream_readdir(stream, &entry)) {
        if (strcmp(entry.d_name, "..") == 0 || strcmp(entry.d_name, ".") == 0) {
            continue;
        }
        char * newpath = (*handler)(dirname, dirname_len, &entry TSRMLS_CC);
        if ( newpath != NULL) {
            add_next_index_string(z_list, newpath, strlen(newpath) );
        }
    }
}




bool futil_stream_is_dir(php_stream *stream TSRMLS_DC)
{
    return (stream->flags & PHP_STREAM_FLAG_IS_DIR);
}

bool futil_is_dir(char* dirname, int dirname_len TSRMLS_DC)
{
    zval tmp;
    bool ret;
    php_stat(dirname, dirname_len, FS_IS_DIR, &tmp TSRMLS_CC);
    ret = Z_LVAL(tmp) ? true : false;
    zval_dtor( &tmp );
    return ret;
}




PHP_FUNCTION(futil_scanpath_dir)
{
    char *dirname;
    int dirname_len;
    zval * z_list;
    php_stream *stream;

    // parse parameters
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                    &dirname, &dirname_len ) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Path argument is required.");
        RETURN_FALSE;
    }

    // run is_dir
    if ( ! futil_is_dir(dirname, dirname_len TSRMLS_CC) ) {
        RETURN_FALSE;
    }

    stream = php_stream_opendir(dirname, REPORT_ERRORS, NULL );
    if ( ! stream ) {
        RETURN_FALSE;
    }
    // it's not fclose-able
    stream->flags |= PHP_STREAM_FLAG_NO_FCLOSE;

    MAKE_STD_ZVAL(z_list);
    array_init(z_list);

    phpdir_scandir_with_handler(
            z_list,
            stream, 
            dirname, dirname_len, 
            dir_dir_entry_handler TSRMLS_CC);

    php_stream_close(stream);

    // add reference count
    *return_value = *z_list;
    zval_copy_ctor(return_value);
}

PHP_FUNCTION(futil_scanpath)
{
    char *dirname;
    int dirname_len;
    zval * z_list;
    php_stream * stream;

    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                    &dirname, &dirname_len ) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong parameters.");
        RETURN_FALSE;
    }

    if ( dirname_len < 1 ) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Directory name cannot be empty");
        RETURN_FALSE;
    }

    // run is_dir
    if ( ! futil_is_dir(dirname, dirname_len TSRMLS_CC) ) {
        RETURN_FALSE;
    }


    stream = php_stream_opendir(dirname, REPORT_ERRORS, NULL );
    if ( ! stream ) {
        RETURN_FALSE;
    }
    // it's not fclose-able
    stream->flags |= PHP_STREAM_FLAG_NO_FCLOSE;

    MAKE_STD_ZVAL(z_list);
    array_init(z_list);

    phpdir_scandir_with_handler(
            z_list, 
            stream,
            dirname, dirname_len, 
            dir_entry_handler TSRMLS_CC);

    php_stream_close(stream);

    // add reference count
    *return_value = *z_list;
    zval_copy_ctor(return_value);
}


PHP_FUNCTION(futil_lastctime)
{
    zval *zarr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a",
                    &zarr
                    ) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong parameters.");
        RETURN_FALSE;
    }

    long lastctime = 0;

    zval **entry_data;
    HashTable *zarr_hash;
    HashPosition pointer;
    int array_count;

    char *path;
    int path_len;

    zarr_hash = Z_ARRVAL_P(zarr);
    array_count = zend_hash_num_elements(zarr_hash);

    for(zend_hash_internal_pointer_reset_ex(zarr_hash, &pointer); 
            zend_hash_get_current_data_ex(zarr_hash, (void**) &entry_data, &pointer) == SUCCESS; 
            zend_hash_move_forward_ex(zarr_hash, &pointer)) 
    {
        if ( Z_TYPE_PP(entry_data) == IS_STRING ) {
            // for string type, we try to treat it as a path
            path = Z_STRVAL_PP(entry_data);
            path_len  = Z_STRLEN_PP(entry_data);

            zval mtime;
            php_stat(path, path_len, FS_MTIME, &mtime TSRMLS_CC);
            if (mtime.value.lval > lastctime ) {
                lastctime = mtime.value.lval;
            }
        }
    }
    RETURN_LONG(lastctime);
}




PHP_FUNCTION(futil_lastmtime)
{
    zval *zarr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a",
                    &zarr
                    ) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong parameters.");
        RETURN_FALSE;
    }

    long lastmtime = 0;

    zval **entry_data;
    HashTable *zarr_hash;
    HashPosition pointer;
    int array_count;

    char *path;
    int path_len;

    zarr_hash = Z_ARRVAL_P(zarr);
    array_count = zend_hash_num_elements(zarr_hash);

    for(zend_hash_internal_pointer_reset_ex(zarr_hash, &pointer); 
            zend_hash_get_current_data_ex(zarr_hash, (void**) &entry_data, &pointer) == SUCCESS; 
            zend_hash_move_forward_ex(zarr_hash, &pointer)) 
    {
        if ( Z_TYPE_PP(entry_data) == IS_STRING ) {
            // for string type, we try to treat it as a path
            path = Z_STRVAL_PP(entry_data);
            path_len  = Z_STRLEN_PP(entry_data);

            zval mtime;
            php_stat(path, path_len, FS_MTIME, &mtime TSRMLS_CC);
            if (mtime.value.lval > lastmtime ) {
                lastmtime = mtime.value.lval;
            }
        }
    }
    RETURN_LONG(lastmtime);
}

PHP_FUNCTION(futil_pathjoin)
{
    int num_varargs;
    zval ***varargs = NULL;
    char *newpath;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "*",
                    &varargs, &num_varargs
                    ) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong parameters.");
        RETURN_FALSE;
    }

    if ( num_varargs == 1 && Z_TYPE_PP(varargs[0]) == IS_ARRAY ) {
        newpath = path_concat_from_zarray(varargs[0] TSRMLS_CC);
    } else if ( num_varargs > 1  && Z_TYPE_PP(varargs[0]) == IS_STRING ) {
        newpath = path_concat_from_zargs( num_varargs , varargs TSRMLS_CC);
    } else {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong parameters.");
    }

    if (varargs) {
        efree(varargs);
    }
    RETURN_STRING(newpath,0);
}

