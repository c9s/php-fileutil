#ifndef PHP_FILEUTIL_H
#define PHP_FILEUTIL_H 1
#define PHP_FILEUTIL_VERSION "1.0"
#define PHP_FILEUTIL_EXTNAME "fileutil"

PHP_FUNCTION(futil_scandir);
PHP_FUNCTION(futil_scandir_dir);

bool futil_stream_is_dir(php_stream *stream);
bool futil_is_dir(char* dirname, int dirname_len);



extern zend_module_entry fileutil_module_entry;
#define phpext_fileutil_ptr &fileutil_module_entry

#endif
