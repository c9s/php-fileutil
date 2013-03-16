#ifndef PHP_FILEUTIL_H
#define PHP_FILEUTIL_H 1
#define PHP_FILEUTIL_VERSION "1.0"
#define PHP_FILEUTIL_EXTNAME "fileutil"

PHP_FUNCTION(futil_scanpath);
PHP_FUNCTION(futil_scanpath_dir);
PHP_FUNCTION(futil_pathjoin);
PHP_FUNCTION(futil_pathsplit);
PHP_FUNCTION(futil_lastmtime);
PHP_FUNCTION(futil_lastctime);

bool futil_stream_is_dir(php_stream *stream TSRMLS_DC);
bool futil_is_dir(char* dirname, int dirname_len TSRMLS_DC);
char * path_concat_from_zargs( int num_varargs , zval ***varargs TSRMLS_DC);

extern zend_module_entry fileutil_module_entry;
#define phpext_fileutil_ptr &fileutil_module_entry

#endif
