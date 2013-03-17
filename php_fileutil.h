#ifndef PHP_FILEUTIL_H
#define PHP_FILEUTIL_H 1
#define PHP_FILEUTIL_VERSION "1.0"
#define PHP_FILEUTIL_EXTNAME "fileutil"

#include <Zend/zend.h>
#include <Zend/zend_constants.h>
#include <Zend/zend_execute.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_hash.h>
#include <Zend/zend_interfaces.h>
#include <Zend/zend_operators.h>
#include <Zend/zend_qsort.h>
#include <Zend/zend_vm.h>

#include <ext/standard/php_standard.h>
#include <ext/standard/php_filestat.h>
#include <ext/standard/php_string.h>


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


#if HAVE_SPL
#include <ext/spl/spl_array.h>
#include <ext/spl/spl_directory.h>
#include <ext/spl/spl_engine.h>
#include <ext/spl/spl_exceptions.h>
#include <ext/spl/spl_iterators.h>
#endif

#ifndef bool
#define bool short
#define false 0
#define true 1
#endif

#ifndef tsrm_ls
#define tsrm_ls NULL
#endif

bool futil_stream_is_dir(php_stream *stream TSRMLS_DC);
bool futil_is_dir(char* dirname, int dirname_len TSRMLS_DC);
char * path_concat_from_zargs( int num_varargs , zval ***varargs TSRMLS_DC);

extern zend_module_entry fileutil_module_entry;
#define phpext_fileutil_ptr &fileutil_module_entry

#endif
