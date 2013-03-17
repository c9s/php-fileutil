#ifndef PHP_FILEUTIL_H
#define PHP_FILEUTIL_H 1
#define PHP_FILEUTIL_VERSION "1.0"
#define PHP_FILEUTIL_EXTNAME "fileutil"



PHP_FUNCTION(futil_scanpath);
PHP_FUNCTION(futil_scanpath_dir);
PHP_FUNCTION(futil_pathjoin);
PHP_FUNCTION(futil_pathsplit);
PHP_FUNCTION(futil_paths_prepend);
PHP_FUNCTION(futil_paths_append);
PHP_FUNCTION(futil_lastmtime);
PHP_FUNCTION(futil_lastctime);
PHP_FUNCTION(futil_unlink_if_exists);
PHP_FUNCTION(futil_rmdir_if_exists);
PHP_FUNCTION(futil_mkdir_if_not_exists);
PHP_FUNCTION(futil_rmtree);
PHP_FUNCTION(futil_replace_extension);



bool futil_stream_is_dir(php_stream *stream TSRMLS_DC);
bool futil_is_dir(char* dirname, int dirname_len TSRMLS_DC);
char * path_concat_from_zargs( int num_varargs , zval ***varargs TSRMLS_DC);

extern zend_module_entry fileutil_module_entry;
#define phpext_fileutil_ptr &fileutil_module_entry

#endif
