#include "php_fileutil.h"
#include "path.h"


#include <ext/standard/php_standard.h>
#include <ext/standard/php_filestat.h>
#include <ext/standard/php_string.h>

#if HAVE_SPL
#include <ext/spl/spl_array.h>
#include <ext/spl/spl_directory.h>
#include <ext/spl/spl_engine.h>
#include <ext/spl/spl_exceptions.h>
#include <ext/spl/spl_iterators.h>
#endif


// these flags are inside the ext/spl/spl_iterators.c, 
// we can not reuse it by including the header file.
typedef enum {
    RIT_LEAVES_ONLY = 0,
    RIT_SELF_FIRST  = 1,
    RIT_CHILD_FIRST = 2
} RecursiveIteratorMode;



ZEND_BEGIN_ARG_INFO_EX(arginfo_futil_scanpath, 0, 0, 1)
    ZEND_ARG_INFO(0, dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_futil_scanpath_dir, 0, 0, 1)
    ZEND_ARG_INFO(0, dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_futil_lastmtime, 0, 0, 1)
    ZEND_ARG_INFO(0, filelist)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_futil_lastctime, 0, 0, 1)
    ZEND_ARG_INFO(0, filelist)
ZEND_END_ARG_INFO()

static const zend_function_entry fileutil_functions[] = {
    PHP_FE(futil_scanpath, arginfo_futil_scanpath)
    PHP_FE(futil_scanpath_dir, arginfo_futil_scanpath_dir)
    PHP_FE(futil_pathjoin, NULL)
    PHP_FE(futil_pathsplit, NULL)
    PHP_FE(futil_paths_append, NULL)
    PHP_FE(futil_paths_prepend, NULL)
    PHP_FE(futil_lastmtime, arginfo_futil_lastmtime)
    PHP_FE(futil_lastctime, arginfo_futil_lastctime)
    PHP_FE(futil_unlink_if_exists, NULL)
    PHP_FE(futil_rmdir_if_exists, NULL)
    PHP_FE(futil_mkdir_if_not_exists, NULL)
    PHP_FE(futil_rmtree, NULL)
    PHP_FE(futil_replace_extension, NULL)
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


static zend_bool _unlink_file(char *filename, int filename_len, zval *zcontext TSRMLS_DC);
zend_bool futil_file_exists(char * filename, int filename_len TSRMLS_DC);
zend_bool futil_stream_is_dir(php_stream *stream TSRMLS_DC);
zend_bool futil_is_dir(char* dirname, int dirname_len TSRMLS_DC);




zend_bool futil_file_exists(char * filename, int filename_len TSRMLS_DC)
{
    zval tmp;
    php_stat(filename, filename_len, FS_EXISTS, &tmp TSRMLS_CC);
    zend_bool ret = Z_LVAL(tmp) ? true : false;
    zval_dtor( &tmp );
    return ret;
}

zend_bool futil_stream_is_dir(php_stream *stream TSRMLS_DC)
{
    return (stream->flags & PHP_STREAM_FLAG_IS_DIR);
}

zend_bool futil_is_dir(char* dirname, int dirname_len TSRMLS_DC)
{
    zval tmp;
    zend_bool ret;
    php_stat(dirname, dirname_len, FS_IS_DIR, &tmp TSRMLS_CC);
    ret = Z_LVAL(tmp) ? true : false;
    zval_dtor( &tmp );
    return ret;
}






char * path_concat_from_zargs( int num_varargs , zval ***varargs TSRMLS_DC)
{
    char *dst;
    char *newpath;
    int i;
    int len;
    zval **arg;

    len = num_varargs + 1;
    for (i = 0; i < num_varargs; i++) {
        arg = varargs[i];
        len += Z_STRLEN_PP(arg);
    }

    newpath = emalloc( sizeof(char) * len );

    dst = newpath;
    for (i = 0; i < num_varargs; i++ ) {
        arg = varargs[i];
        char *subpath = Z_STRVAL_PP(arg);
        int  subpath_len = Z_STRLEN_PP(arg);

        if( subpath_len == 0 ) {
            continue;
        }

        dst = path_concat_fill(dst, subpath, subpath_len, i > 0 TSRMLS_CC);

        // concat slash to the end
        if ( *(dst-1) != DEFAULT_SLASH && i < (num_varargs - 1) ) {
            *dst = DEFAULT_SLASH;
            dst++;
        }
    }
    *dst = '\0';
    return newpath;
}


char * dir_dir_entry_handler(
        char* dirname, 
        int dirname_len,
        php_stream_dirent * entry TSRMLS_DC)
{
    char * path = path_concat(dirname, dirname_len, entry->d_name, strlen(entry->d_name) TSRMLS_CC);
    int    path_len = dirname_len + strlen(entry->d_name) + 1;
    if ( ! futil_is_dir(path, path_len TSRMLS_CC) ) {
        efree(path);
        return NULL;
    }
    return path;
}

char* dir_entry_handler(
        char* dirname, 
        int dirname_len, 
        php_stream_dirent * entry TSRMLS_DC)
{
    return path_concat(dirname, dirname_len, entry->d_name, strlen(entry->d_name) TSRMLS_CC);
}


void phpdir_scandir_with_handler(
        zval * z_list,
        php_stream * stream, 
        char* dirname, 
        int dirname_len,
        char* (*handler)(char*, int, php_stream_dirent* TSRMLS_DC) TSRMLS_DC) 
{
    php_stream_dirent entry;
    while (php_stream_readdir(stream, &entry)) {
        if (strcmp(entry.d_name, "..") == 0 || strcmp(entry.d_name, ".") == 0) {
            continue;
        }
        char * newpath = (*handler)(dirname, dirname_len, &entry TSRMLS_CC);
        if ( newpath != NULL) {
            add_next_index_string(z_list, newpath, strlen(newpath) );
        }
    }
}







PHP_FUNCTION(futil_scanpath_dir)
{
    char *dirname;
    int dirname_len;
    zval * z_list;
    php_stream *stream;

    // parse parameters
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                    &dirname, &dirname_len ) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Path argument is required.");
        RETURN_FALSE;
    }

    // run is_dir
    if ( ! futil_is_dir(dirname, dirname_len TSRMLS_CC) ) {
        RETURN_FALSE;
    }

    stream = php_stream_opendir(dirname, REPORT_ERRORS, NULL );
    if ( ! stream ) {
        RETURN_FALSE;
    }
    // it's not fclose-able
    stream->flags |= PHP_STREAM_FLAG_NO_FCLOSE;

    MAKE_STD_ZVAL(z_list);
    array_init(z_list);

    phpdir_scandir_with_handler(
            z_list,
            stream, 
            dirname, dirname_len, 
            dir_dir_entry_handler TSRMLS_CC);

    php_stream_close(stream);

    // add reference count
    *return_value = *z_list;
    zval_copy_ctor(return_value);
}

PHP_FUNCTION(futil_scanpath)
{
    char *dirname;
    int dirname_len;
    zval * z_list;
    php_stream * stream;

    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                    &dirname, &dirname_len ) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong parameters.");
        RETURN_FALSE;
    }

    if ( dirname_len < 1 ) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Directory name cannot be empty");
        RETURN_FALSE;
    }

    // run is_dir
    if ( ! futil_is_dir(dirname, dirname_len TSRMLS_CC) ) {
        RETURN_FALSE;
    }


    stream = php_stream_opendir(dirname, REPORT_ERRORS, NULL );
    if ( ! stream ) {
        RETURN_FALSE;
    }
    // it's not fclose-able
    stream->flags |= PHP_STREAM_FLAG_NO_FCLOSE;

    MAKE_STD_ZVAL(z_list);
    array_init(z_list);

    phpdir_scandir_with_handler(
            z_list, 
            stream,
            dirname, dirname_len, 
            dir_entry_handler TSRMLS_CC);

    php_stream_close(stream);

    // add reference count
    *return_value = *z_list;
    zval_copy_ctor(return_value);
}


PHP_FUNCTION(futil_lastctime)
{
    zval *zarr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a",
                    &zarr
                    ) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong parameters.");
        RETURN_FALSE;
    }

    long lastctime = 0;

    zval **entry_data;
    HashTable *zarr_hash;
    HashPosition pointer;
    int array_count;

    char *path;
    int path_len;

    zarr_hash = Z_ARRVAL_P(zarr);
    array_count = zend_hash_num_elements(zarr_hash);

    for(zend_hash_internal_pointer_reset_ex(zarr_hash, &pointer); 
            zend_hash_get_current_data_ex(zarr_hash, (void**) &entry_data, &pointer) == SUCCESS; 
            zend_hash_move_forward_ex(zarr_hash, &pointer)) 
    {
        if ( Z_TYPE_PP(entry_data) == IS_STRING ) {
            // for string type, we try to treat it as a path
            path = Z_STRVAL_PP(entry_data);
            path_len  = Z_STRLEN_PP(entry_data);

            zval mtime;
            php_stat(path, path_len, FS_MTIME, &mtime TSRMLS_CC);
            if (mtime.value.lval > lastctime ) {
                lastctime = mtime.value.lval;
            }
        }
    }
    RETURN_LONG(lastctime);
}




PHP_FUNCTION(futil_lastmtime)
{
    zval *zarr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a",
                    &zarr
                    ) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong parameters.");
        RETURN_FALSE;
    }

    long lastmtime = 0;

    zval **entry_data;
    HashTable *zarr_hash;
    HashPosition pointer;
    int array_count;

    char *path;
    int path_len;

    zarr_hash = Z_ARRVAL_P(zarr);
    array_count = zend_hash_num_elements(zarr_hash);

    for(zend_hash_internal_pointer_reset_ex(zarr_hash, &pointer); 
            zend_hash_get_current_data_ex(zarr_hash, (void**) &entry_data, &pointer) == SUCCESS; 
            zend_hash_move_forward_ex(zarr_hash, &pointer)) 
    {
        if ( Z_TYPE_PP(entry_data) == IS_STRING ) {
            // for string type, we try to treat it as a path
            path = Z_STRVAL_PP(entry_data);
            path_len  = Z_STRLEN_PP(entry_data);

            zval mtime;
            php_stat(path, path_len, FS_MTIME, &mtime TSRMLS_CC);
            if (mtime.value.lval > lastmtime ) {
                lastmtime = mtime.value.lval;
            }
        }
    }
    RETURN_LONG(lastmtime);
}


PHP_FUNCTION(futil_pathsplit)
{
    char *path;
    int  path_len;

    zval zdelim;
    zval zstr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                    &path, &path_len
                    ) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong parameters.");
        RETURN_FALSE;
    }

    array_init(return_value);

    char delim_str[2];
    delim_str[0] = DEFAULT_SLASH;
    delim_str[1] = '\0';

    ZVAL_STRINGL(&zstr, path, path_len, 0);
    ZVAL_STRINGL(&zdelim, delim_str, 1, 0);

    // PHPAPI void php_explode(zval *delim, zval *str, zval *return_value, long limit)
    php_explode(&zdelim, &zstr, return_value, LONG_MAX); // LONG_MAX means no limit
}


PHP_FUNCTION(futil_mkdir_if_not_exists)
{

    char *dir;
    int dir_len;
    zval *zcontext = NULL;
    long mode = 0777;
    zend_bool recursive = 0;
    php_stream_context *context;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p|lbr", &dir, &dir_len, &mode, &recursive, &zcontext) == FAILURE) {
        RETURN_FALSE;
    }

    if ( futil_file_exists(dir,dir_len TSRMLS_CC) ) {
        RETURN_FALSE;
    }
    context = php_stream_context_from_zval(zcontext, 0);
    RETURN_BOOL(php_stream_mkdir(dir, mode, (recursive ? PHP_STREAM_MKDIR_RECURSIVE : 0) | REPORT_ERRORS, context));
}


PHP_FUNCTION(futil_rmdir_if_exists)
{

    char *dir;
    int dir_len;
    zval *zcontext = NULL;
    long mode = 0777;
    zend_bool recursive = 0;
    php_stream_context *context;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p|lbr", &dir, &dir_len, &mode, &recursive, &zcontext) == FAILURE) {
        RETURN_FALSE;
    }

    if ( ! futil_file_exists(dir,dir_len TSRMLS_CC) ) {
        RETURN_FALSE;
    }

    context = php_stream_context_from_zval(zcontext, 0);
    RETURN_BOOL(php_stream_rmdir(dir, REPORT_ERRORS, context));
}



static zend_bool _unlink_file(char *filename, int filename_len, zval *zcontext TSRMLS_DC)
{
    php_stream_wrapper *wrapper;
    php_stream_context *context = NULL;

    context = php_stream_context_from_zval(zcontext, 0);
    wrapper = php_stream_locate_url_wrapper(filename, NULL, 0 TSRMLS_CC);

    if (!wrapper || !wrapper->wops) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to locate stream wrapper");
        return false;
    }

    if (!wrapper->wops->unlink) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s does not allow unlinking", wrapper->wops->label ? wrapper->wops->label : "Wrapper");
        return false;
    }
    return wrapper->wops->unlink(wrapper, filename, REPORT_ERRORS, context TSRMLS_CC);
}


PHP_FUNCTION(futil_unlink_if_exists)
{
    char *filename;
    int filename_len;
    zval *zcontext = NULL;
    php_stream_wrapper *wrapper;
    php_stream_context *context = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p|r", &filename, &filename_len, &zcontext) == FAILURE) {
        RETURN_FALSE;
    }

    zval tmp;
    zend_bool ret;
    php_stat(filename, filename_len, FS_EXISTS, &tmp TSRMLS_CC);
    zval_dtor( &tmp );
    if ( Z_LVAL(tmp) == false ) {
        RETURN_FALSE;
    }
    RETURN_BOOL( _unlink_file(filename, filename_len, zcontext TSRMLS_CC) );
}



static int rmtree_iterator(zend_object_iterator *iter, void *puser TSRMLS_DC)
{
    zval **value;

    char *fname;
    int  fname_len;

    iter->funcs->get_current_data(iter, &value TSRMLS_CC);
    if (EG(exception)) {
        return ZEND_HASH_APPLY_STOP;
    }
    if (!value) {
        /* failure in get_current_data */
        // zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0 TSRMLS_CC, "Iterator %v returned no value", ce->name);
        zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0 TSRMLS_CC, "Iterator returned no value");
        return ZEND_HASH_APPLY_STOP;
    }

    switch (Z_TYPE_PP(value)) {
//          case IS_UNICODE:
//              zval_unicode_to_string(*(value) TSRMLS_CC);
        case IS_STRING:
            break;
        case IS_OBJECT:
            if (instanceof_function(Z_OBJCE_PP(value), spl_ce_SplFileInfo TSRMLS_CC)) {

                char *test = NULL;
                zval dummy;
                zend_bool is_dir = false;
                spl_filesystem_object *intern = (spl_filesystem_object*)zend_object_store_get_object(*value TSRMLS_CC);

                switch (intern->type) {
                    case SPL_FS_DIR:
                    case SPL_FS_FILE:
                    case SPL_FS_INFO:

                        php_stat(intern->file_name, intern->file_name_len, FS_IS_DIR, &dummy TSRMLS_CC);
                        is_dir = Z_BVAL(dummy);

#if PHP_VERSION_ID >= 60000
                        if (intern->file_name_type == IS_UNICODE) {
                            zval zv;

                            INIT_ZVAL(zv);
                            Z_UNIVAL(zv) = intern->file_name;
                            Z_UNILEN(zv) = intern->file_name_len;
                            Z_TYPE(zv) = IS_UNICODE;

                            zval_copy_ctor(&zv);
                            zval_unicode_to_string(&zv TSRMLS_CC);
                            fname = expand_filepath(Z_STRVAL(zv), NULL TSRMLS_CC);
                            ezfree(Z_UNIVAL(zv));
                        } else {
                            fname = expand_filepath(intern->file_name.s, NULL TSRMLS_CC);
                        }
#else
                        fname = expand_filepath(intern->file_name, NULL TSRMLS_CC);
#endif
                        if (!fname) {
                            zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0 TSRMLS_CC, "Could not resolve file path");
                            return ZEND_HASH_APPLY_STOP;
                        }

                        if ( is_dir ) {
                            php_stream_rmdir(fname, REPORT_ERRORS, NULL);
                        } else {
                            fname_len = strlen(fname);
                            _unlink_file(fname, fname_len, NULL TSRMLS_CC);
                        }

                        // goto phar_spl_fileinfo;
                        return ZEND_HASH_APPLY_KEEP;
                }
            }
            /* fall-through */
        default:
            // zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0 TSRMLS_CC, "Iterator %v returned an invalid value (must return a string)", ce->name);
            zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0 TSRMLS_CC, "Iterator returned an invalid value (must return a string)");
            return ZEND_HASH_APPLY_STOP;
    }
    

    // printf("got value: %s\n", Z_STRVAL_PP(value) );
    return ZEND_HASH_APPLY_KEEP;
}



PHP_FUNCTION(futil_rmtree)
{
    char *dir;
    int  dir_len;


    
    char *error, *regex = NULL;
    int regex_len = 0;
    zend_bool apply_reg = 0;
    zval arg, arg2, *iter, *iteriter, *regexiter = NULL;

    zval iteriter_arg2;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p",
                    &dir, &dir_len
                    ) == FAILURE) {
        RETURN_FALSE;
    }


    MAKE_STD_ZVAL(iter);

    if (SUCCESS != object_init_ex(iter, spl_ce_RecursiveDirectoryIterator)) {
        zval_ptr_dtor(&iter);
        zend_throw_exception_ex(spl_ce_BadMethodCallException, 0 TSRMLS_CC, "Unable to instantiate directory iterator for %s", dir);
        RETURN_FALSE;
    }

    INIT_PZVAL(&arg);
    ZVAL_STRINGL(&arg, dir, dir_len, 0);
    INIT_PZVAL(&arg2);
#if PHP_VERSION_ID < 50300
    ZVAL_LONG(&arg2, 0);
#else

    // Possible values
    //   SPL_FILE_DIR_CURRENT_AS_PATHNAME
    //   SPL_FILE_DIR_CURRENT_AS_FILEINFO
    //   SPL_FILE_DIR_FOLLOW_SYMLINKS


    // Iterator constants
    //   typedef enum {
    //       RIT_LEAVES_ONLY = 0,
    //       RIT_SELF_FIRST  = 1,
    //       RIT_CHILD_FIRST = 2
    //   } RecursiveIteratorMode;
    //    


    // ZVAL_LONG(&arg2, SPL_FILE_DIR_SKIPDOTS|SPL_FILE_DIR_UNIXPATHS);
    ZVAL_LONG(&arg2, SPL_FILE_DIR_SKIPDOTS|SPL_FILE_DIR_UNIXPATHS);
#endif

    zend_call_method_with_2_params(&iter, spl_ce_RecursiveDirectoryIterator, 
            &spl_ce_RecursiveDirectoryIterator->constructor, "__construct", NULL, &arg, &arg2);

    if (EG(exception)) {
        zval_ptr_dtor(&iter);
        RETURN_FALSE;
    }

    MAKE_STD_ZVAL(iteriter);

    if (SUCCESS != object_init_ex(iteriter, spl_ce_RecursiveIteratorIterator)) {
        zval_ptr_dtor(&iter);
        zval_ptr_dtor(&iteriter);
        zend_throw_exception_ex(spl_ce_BadMethodCallException, 0 TSRMLS_CC, "Unable to instantiate directory iterator for %s", dir);
        RETURN_FALSE;
    }

    INIT_PZVAL(&iteriter_arg2);
#if PHP_VERSION_ID < 50300
    ZVAL_LONG(&iteriter_arg2, 0);
#else
    ZVAL_LONG(&iteriter_arg2, RIT_CHILD_FIRST);
#endif

    zend_call_method_with_2_params(&iteriter, spl_ce_RecursiveIteratorIterator, 
            &spl_ce_RecursiveIteratorIterator->constructor, "__construct", NULL, iter, &iteriter_arg2);

    if (EG(exception)) {
        zval_ptr_dtor(&iter);
        zval_ptr_dtor(&iteriter);
        RETURN_FALSE;
    }

    zval_ptr_dtor(&iter);


    int pass = 0;

    if (SUCCESS == spl_iterator_apply(iteriter, (spl_iterator_apply_func_t) rmtree_iterator, (void *) &pass TSRMLS_CC)) {

    }

    RETURN_FALSE;
}




PHP_FUNCTION(futil_paths_append)
{
    zval *zarr;

    zval **entry_data;
    HashTable *zarr_hash;
    HashPosition pointer;
    int zarr_count;

    char *str_append;
    int   str_append_len;

    char *str;
    int   str_len;

    char *newpath;
    int   newpath_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "as", &zarr, &str_append, &str_append_len) == FAILURE) {
        RETURN_FALSE;
    }

    zarr_hash = Z_ARRVAL_P(zarr);
    zarr_count = zend_hash_num_elements(zarr_hash);

    for(zend_hash_internal_pointer_reset_ex(zarr_hash, &pointer); 
            zend_hash_get_current_data_ex(zarr_hash, (void**) &entry_data, &pointer) == SUCCESS; 
            zend_hash_move_forward_ex(zarr_hash, &pointer)) 
    {
        if ( Z_TYPE_PP(entry_data) == IS_STRING ) {
            str = Z_STRVAL_PP(entry_data);
            str_len = Z_STRLEN_PP(entry_data);

            newpath = path_concat(str, str_len, str_append, str_append_len);
            newpath_len = strlen(newpath);

            // free up the previous string
            efree(Z_STRVAL_PP(entry_data));

            Z_STRVAL_PP(entry_data) = newpath;
            Z_STRLEN_PP(entry_data) = newpath_len;
        }
    }
}


PHP_FUNCTION(futil_paths_prepend)
{
    zval *zarr;

    zval **entry_data;
    HashTable *zarr_hash;
    HashPosition pointer;
    int zarr_count;

    char *str_prepend;
    int   str_prepend_len;

    char *str;
    int   str_len;

    char *newpath;
    int   newpath_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "as", &zarr, &str_prepend, &str_prepend_len) == FAILURE) {
        RETURN_FALSE;
    }

    zarr_hash = Z_ARRVAL_P(zarr);
    zarr_count = zend_hash_num_elements(zarr_hash);

    for(zend_hash_internal_pointer_reset_ex(zarr_hash, &pointer); 
            zend_hash_get_current_data_ex(zarr_hash, (void**) &entry_data, &pointer) == SUCCESS; 
            zend_hash_move_forward_ex(zarr_hash, &pointer)) 
    {
        if ( Z_TYPE_PP(entry_data) == IS_STRING ) {
            str = Z_STRVAL_PP(entry_data);
            str_len = Z_STRLEN_PP(entry_data);

            newpath = path_concat(str_prepend, str_prepend_len, str, str_len);
            newpath_len = strlen(newpath);

            // free up the previous string
            efree(Z_STRVAL_PP(entry_data));

            Z_STRVAL_PP(entry_data) = newpath;
            Z_STRLEN_PP(entry_data) = newpath_len;
        }
    }
}


PHP_FUNCTION(futil_replace_extension)
{
    char *filename;
    int   filename_len;

    char *extension;
    int   extension_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
                    &filename, &filename_len, &extension, &extension_len
                    ) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong parameters.");
        RETURN_FALSE;
    }

    char *newfilename;
    int newfilename_len;

    char *dot;
    char *basename;
    int basename_len;

    dot = strrchr(filename, (int) '.');
    if ( dot ) {
        basename_len = dot - filename + 1;
        if ( basename_len == filename_len ) {
            RETURN_STRINGL(filename, filename_len, 0);
        }
    } else {
        basename_len = filename_len;
        char *new_extension = emalloc( sizeof(char) * (extension_len + 1) );
        *new_extension = '.';
        strncpy(new_extension + 1, extension, extension_len);
        efree(extension);
        extension = new_extension;
        extension_len++;
    }

    // basename with dot
    basename = estrndup(filename, basename_len); // create basename

    // create newfilename string
    newfilename = strncat(basename, extension, extension_len);
    newfilename_len = basename_len + extension_len;
    RETURN_STRINGL(newfilename, newfilename_len, 0);
}

PHP_FUNCTION(futil_pathjoin)
{
    int num_varargs;
    zval ***varargs = NULL;
    char *newpath;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "*",
                    &varargs, &num_varargs
                    ) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong parameters.");
        RETURN_FALSE;
    }

    if ( num_varargs == 1 && Z_TYPE_PP(varargs[0]) == IS_ARRAY ) {
        newpath = path_concat_from_zarray(varargs[0] TSRMLS_CC);
    } else if ( num_varargs > 1  && Z_TYPE_PP(varargs[0]) == IS_STRING ) {
        newpath = path_concat_from_zargs( num_varargs , varargs TSRMLS_CC);
    } else {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong parameters.");
    }

    if (varargs) {
        efree(varargs);
    }
    RETURN_STRING(newpath,0);
}

