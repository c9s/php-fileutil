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

    int createobject = 0;
    if (createobject) {
        // object_init_ex(return_value, dir_class_entry_ptr);
        // add_property_stringl(return_value, "path", dirname, dirname_len, 1);
        // add_property_resource(return_value, "handle", dirp->rsrc_id);
        // php_stream_auto_cleanup(dirp); /* so we don't get warnings under debug */
    } else {
        // convert php stream to zval
        php_stream_to_zval(dirp, z_handle);
    }


    // add_next_index_string(z_list, "", 1);
    // RETURN_STRING("Hello World", 1);
}

