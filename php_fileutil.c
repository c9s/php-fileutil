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

    // add reference count
    zval_copy_ctor(z_list);
    *return_value = *z_list;

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

    if ( num_varargs == 1 && Z_TYPE_PP(varargs[0]) == IS_ARRAY ) {
        newpath = path_concat_from_zarray(varargs[0]);
    } else if ( num_varargs > 1  && Z_TYPE_PP(varargs[0]) == IS_STRING ) {
        newpath = path_concat_from_zargs( num_varargs , varargs );
    } else {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong parameters.");
    }

    if (varargs) {
        efree(varargs);
    }

    RETVAL_STRING(newpath,1);
}

