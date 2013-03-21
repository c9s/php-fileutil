#ifndef PHP_FILEUTIL_H
#define PHP_FILEUTIL_H 1
#define PHP_FILEUTIL_VERSION "1.0"
#define PHP_FILEUTIL_EXTNAME "fileutil"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "zend.h"
#include "zend_API.h"
#include "zend_compile.h"
#include "zend_hash.h"
#include "zend_extensions.h"

#include "zend_constants.h"
#include "zend_execute.h"
#include "zend_exceptions.h"
#include "zend_hash.h"
#include "zend_interfaces.h"
#include "zend_operators.h"
#include "zend_qsort.h"
#include "zend_vm.h"


#include <ext/standard/php_standard.h>
#include <ext/standard/php_filestat.h>
#include <ext/standard/php_string.h>

// these flags are inside the ext/spl/spl_iterators.c, 
// we can not reuse it by including the header file.
typedef enum {
    RIT_LEAVES_ONLY = 0,
    RIT_SELF_FIRST  = 1,
    RIT_CHILD_FIRST = 2
} RecursiveIteratorMode;

#if HAVE_SPL
#include <ext/spl/spl_array.h>
#include <ext/spl/spl_directory.h>
#include <ext/spl/spl_engine.h>
#include <ext/spl/spl_exceptions.h>
#include <ext/spl/spl_iterators.h>
#endif

PHP_FUNCTION(futil_scanpath);
PHP_FUNCTION(futil_scanpath_dir);
PHP_FUNCTION(futil_pathjoin);
PHP_FUNCTION(futil_pathsplit);

PHP_FUNCTION(futil_paths_append);
PHP_FUNCTION(futil_paths_prepend);
PHP_FUNCTION(futil_paths_remove_basepath);
PHP_FUNCTION(futil_paths_filter_dir);
PHP_FUNCTION(futil_paths_filter_file);

PHP_FUNCTION(futil_lastmtime);
PHP_FUNCTION(futil_lastctime);
PHP_FUNCTION(futil_mtime_compare);
PHP_FUNCTION(futil_ctime_compare);

PHP_FUNCTION(futil_unlink_if_exists);
PHP_FUNCTION(futil_rmdir_if_exists);
PHP_FUNCTION(futil_mkdir_if_not_exists);
PHP_FUNCTION(futil_rmtree);
PHP_FUNCTION(futil_replace_extension);
PHP_FUNCTION(futil_get_extension);
PHP_FUNCTION(futil_prettysize);
PHP_FUNCTION(futil_filename_append_suffix);
PHP_FUNCTION(futil_get_contents_from_files);
PHP_FUNCTION(futil_get_contents_array_from_files);

#define false 0
#define true 1

zend_bool futil_file_exists(char * filename, int filename_len TSRMLS_DC);
zend_bool futil_stream_is_dir(php_stream *stream TSRMLS_DC);
zend_bool futil_is_dir(char* dirname, int dirname_len TSRMLS_DC);
zend_bool futil_is_file(char* dirname, int dirname_len TSRMLS_DC);

zend_bool futil_unlink_file(char *filename, int filename_len, zval *zcontext TSRMLS_DC);

extern zend_module_entry fileutil_module_entry;
#define phpext_fileutil_ptr &fileutil_module_entry

#define SIZE_KB 1024
#define SIZE_MB (1024 * 1024)
#define SIZE_GB (1024 * 1024 * 1024)
#define SIZE_TB (1024 * 1024 * 1024 * 1024)

#endif
