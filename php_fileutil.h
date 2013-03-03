

#ifndef PHP_FILEUTIL_H
#define PHP_FILEUTIL_H 1
#define PHP_FILEUTIL_VERSION "1.0"
#define PHP_FILEUTIL_EXTNAME "fileutil"

PHP_FUNCTION(futil_scandir);
PHP_FUNCTION(futil_scandir_dir);

bool _futil_stream_is_dir(php_stream *stream);
bool _futil_is_dir(char* dirname, int dirname_len);
char * concat_path( char* path1, int len1, char * path2 );





// dirp
typedef struct { 
    php_stream_context *context;
    php_stream *stream;
    zval *zcontext;
} dirp;

dirp* dirp_open(char * dirname);
zval* dirp_scandir_with_func( dirp * dirp, char* dirname, int dirname_len, char* (*func)(char*, int, php_stream_dirent*) );
void dirp_close( dirp * dirp );

char* dirp_scandir_entry_handler( char* dirname, int dirname_len, php_stream_dirent * entry);


extern zend_module_entry fileutil_module_entry;
#define phpext_fileutil_ptr &fileutil_module_entry

#endif
