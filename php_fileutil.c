#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_fileutil.h"

#include <Zend/zend.h>
#include <ext/standard/php_standard.h>
#include <ext/standard/php_filestat.h>

ZEND_BEGIN_ARG_INFO_EX(arginfo_futil_readdir, 0, 0, 1)
    ZEND_ARG_INFO(0, dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_futil_readdir_for_dir, 0, 0, 1)
    ZEND_ARG_INFO(0, dir)
ZEND_END_ARG_INFO()

static const zend_function_entry fileutil_functions[] = {
    PHP_FE(futil_readdir, arginfo_futil_readdir)
    PHP_FE(futil_readdir_for_dir, arginfo_futil_readdir_for_dir)
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




bool _futil_stream_is_dir(php_stream *stream)
{
    return (stream->flags & PHP_STREAM_FLAG_IS_DIR);
}

bool _futil_is_dir(char* dirname, int dirname_len)
{
    zval tmp;
    php_stat(dirname, dirname_len, FS_IS_DIR, &tmp TSRMLS_CC);
    return Z_LVAL(tmp) ? true : false;
}

PHP_FUNCTION(futil_readdir_for_dir)
{
}

PHP_FUNCTION(futil_readdir)
{
    zval *z_list;

    char *dirname;
    int dirname_len;

    ALLOC_INIT_ZVAL( z_list );
    array_init(z_list);

    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                    &dirname, &dirname_len ) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong parameters.");
        RETURN_FALSE;
    }


    // run is_dir
    if( ! _futil_is_dir(dirname, dirname_len) ) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "The path is not a directory.");
        RETURN_FALSE;
    }


    zval *z_handle;
    php_stream_context *context = NULL;
    php_stream *dirp;
    zval *zcontext = NULL;
    context = php_stream_context_from_zval(zcontext, 0);

    // opendir
    dirp = php_stream_opendir(dirname, REPORT_ERRORS, context);
    if (dirp == NULL) {
        RETURN_FALSE;
    }
    // it's not fclose-able
    dirp->flags |= PHP_STREAM_FLAG_NO_FCLOSE;
        
    php_stream_dirent entry;
    while (php_stream_readdir(dirp, &entry)) {
        if (strcmp(entry.d_name, "..") == 0 || strcmp(entry.d_name, ".") == 0)
            continue;
        char *newpath = (char *) emalloc(512);
        int newpath_len = dirname_len + 1 + strlen(entry.d_name);
        sprintf(newpath,"%s%c%s", dirname, DEFAULT_SLASH, entry.d_name);
        add_next_index_string(z_list, newpath ,  newpath_len );
    }

    // closedir
    // rsrc_id = dirp->rsrc_id;
    zend_list_delete(dirp->rsrc_id);
    *return_value = *z_list;
    zval_copy_ctor(return_value);
}

