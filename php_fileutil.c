#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_fileutil.h"

#include <Zend/zend.h>
#include <ext/standard/php_standard.h>
#include <ext/standard/php_filestat.h>

ZEND_BEGIN_ARG_INFO_EX(arginfo_fileutil_readdir, 0, 0, 1)
    ZEND_ARG_INFO(0, dir)
ZEND_END_ARG_INFO()

static const zend_function_entry fileutil_functions[] = {
    PHP_FE(fileutil_readdir, arginfo_fileutil_readdir)
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

PHP_FUNCTION(fileutil_readdir)
{
    zval *z_list;

    char *dirname;
    int dirname_len;

    ALLOC_INIT_ZVAL( z_list );
    array_init(z_list);

    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                    &dirname, &dirname_len ) == FAILURE) {
        RETURN_FALSE;
    }


    zval tmp;
    php_stat(dirname, dirname_len, FS_IS_DIR, &tmp TSRMLS_CC);
    if (Z_LVAL(tmp)) {
        RETURN_FALSE;
    }


    zval *z_handle;
    php_stream_context *context = NULL;
    php_stream *dirp;
    zval *zcontext = NULL;


    context = php_stream_context_from_zval(zcontext, 0);
    
    // let's open it
    dirp = php_stream_opendir(dirname, REPORT_ERRORS, context);

    if (dirp == NULL) {
        RETURN_FALSE;
    }


    dirp->flags |= PHP_STREAM_FLAG_NO_FCLOSE;
        
    php_set_default_dir(dirp->rsrc_id TSRMLS_CC);

    if (!(dirp->flags & PHP_STREAM_FLAG_IS_DIR)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "%d is not a valid Directory resource", dirp->rsrc_id);
        RETURN_FALSE;
    }
    
    php_stream_dirent entry;
    while (php_stream_readdir(dirp, &entry)) {
        add_next_index_string(z_list, entry.d_name, strlen(entry.d_name)  );
    }

    // closedir
    // rsrc_id = dirp->rsrc_id;
    zend_list_delete(dirp->rsrc_id);
    return_value = z_list;
    zval_copy_ctor(return_value);
}

