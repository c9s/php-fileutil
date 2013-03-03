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

    char *path;
    int path_len;

    ALLOC_INIT_ZVAL( z_list );
    array_init(z_list);

    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                    &path, &path_len ) == FAILURE) {
        RETURN_FALSE;
    }


    zval tmp;
    php_stat(path, path_len, FS_IS_DIR, &tmp TSRMLS_CC);
    if (Z_LVAL(tmp)) {
        RETURN_FALSE;
    }


    // add_next_index_string(z_list, "", 1);
    // RETURN_STRING("Hello World", 1);
}

