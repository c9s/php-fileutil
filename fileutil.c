#ifdef HAVE_CONFIG_H
#include "config.h"
#endif#include "php.h"


#include "php_fileutil.h"

static function_entry fileutil_functions[] = {
    PHP_FE(fileutil, NULL)
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

PHP_FUNCTION(fileutil_world)
{
    RETURN_STRING("", 1);
}


