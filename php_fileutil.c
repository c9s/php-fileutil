#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_fileutil.h"
#include "phpdir.h"
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




PHP_FUNCTION(futil_scandir_dir)
{
    phpdir *phpdir;
    char *dirname;
    int dirname_len;

    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                    &dirname, &dirname_len ) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Path argument is required.");
        RETURN_FALSE;
    }

    // run is_dir
    if ( ! futil_is_dir(dirname, dirname_len TSRMLS_CC) ) {
        RETURN_FALSE;
    }

    phpdir = phpdir_open(dirname TSRMLS_CC);
    if ( ! phpdir ) {
        RETURN_FALSE;
    }

    zval * z_list;
    MAKE_STD_ZVAL(z_list);
    array_init( z_list );

    phpdir_scandir_with_handler(
            z_list,
            phpdir, 
            dirname, dirname_len, 
            phpdir_dir_entry_handler TSRMLS_CC);


    // closedir
    // rsrc_id = phpdir->rsrc_id;
    phpdir_close(phpdir TSRMLS_CC);

    // add reference count
    *return_value = *z_list;
    zval_copy_ctor(return_value);
}

PHP_FUNCTION(futil_scandir)
{
    phpdir *phpdir;
    char *dirname;
    int dirname_len;

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

    phpdir = phpdir_open(dirname TSRMLS_CC);
    if ( ! phpdir ) {
        RETURN_FALSE;
    }

    zval * z_list;
    MAKE_STD_ZVAL(z_list);
    array_init( z_list );

    phpdir_scandir_with_handler(
            z_list, 
            phpdir,
            dirname, dirname_len, 
            phpdir_entry_handler TSRMLS_CC);

    // closedir
    // rsrc_id = phpdir->rsrc_id;
    phpdir_close(phpdir TSRMLS_CC);

    // add reference count
    *return_value = *z_list;
    zval_copy_ctor(return_value);
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
    RETURN_STRING(newpath,1);
}

