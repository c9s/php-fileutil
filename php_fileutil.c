#include "php_fileutil.h"
#include "path.h"

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
    PHP_FE(futil_get_extension, NULL)
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

zend_bool futil_is_file(char* dirname, int dirname_len TSRMLS_DC)
{
    zval tmp;
    zend_bool ret;
    php_stat(dirname, dirname_len, FS_IS_FILE, &tmp TSRMLS_CC);
    ret = Z_LVAL(tmp) ? true : false;
    zval_dtor( &tmp );
    return ret;
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
    zval **entry_data;
    HashTable *zarr_hash;
    HashPosition pointer;
    int array_count;
    long lastctime = 0;
    char *path;
    int path_len;
    zval mtime;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a",
                    &zarr
                    ) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong parameters.");
        RETURN_FALSE;
    }

    zarr_hash = Z_ARRVAL_P(zarr);
    array_count = zend_hash_num_elements(zarr_hash);

    if ( array_count == 0 )
        RETURN_FALSE;

    for(zend_hash_internal_pointer_reset_ex(zarr_hash, &pointer); 
            zend_hash_get_current_data_ex(zarr_hash, (void**) &entry_data, &pointer) == SUCCESS; 
            zend_hash_move_forward_ex(zarr_hash, &pointer)) 
    {
        if ( Z_TYPE_PP(entry_data) == IS_STRING ) {
            // for string type, we try to treat it as a path
            path = Z_STRVAL_PP(entry_data);
            path_len  = Z_STRLEN_PP(entry_data);

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

    zval mtime;

    zarr_hash = Z_ARRVAL_P(zarr);
    array_count = zend_hash_num_elements(zarr_hash);


    if ( array_count == 0 )
        RETURN_FALSE;


    for(zend_hash_internal_pointer_reset_ex(zarr_hash, &pointer); 
            zend_hash_get_current_data_ex(zarr_hash, (void**) &entry_data, &pointer) == SUCCESS; 
            zend_hash_move_forward_ex(zarr_hash, &pointer)) 
    {
        if ( Z_TYPE_PP(entry_data) == IS_STRING ) {
            // for string type, we try to treat it as a path
            path = Z_STRVAL_PP(entry_data);
            path_len  = Z_STRLEN_PP(entry_data);

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

    if ( path_len == 0 )
        RETURN_FALSE;

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

    if ( dir_len == 0 )
        RETURN_FALSE;

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

    if ( dir_len == 0 )
        RETURN_FALSE;

    if ( ! futil_file_exists(dir,dir_len TSRMLS_CC) ) {
        RETURN_FALSE;
    }

    context = php_stream_context_from_zval(zcontext, 0);
    RETURN_BOOL(php_stream_rmdir(dir, REPORT_ERRORS, context));
}



zend_bool futil_unlink_file(char *filename, int filename_len, zval *zcontext TSRMLS_DC)
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

    if ( filename_len == 0 )
        RETURN_FALSE;

    zval tmp;
    zend_bool ret;
    php_stat(filename, filename_len, FS_EXISTS, &tmp TSRMLS_CC);
    zval_dtor( &tmp );
    if ( Z_LVAL(tmp) == false ) {
        RETURN_FALSE;
    }
    RETURN_BOOL( futil_unlink_file(filename, filename_len, zcontext TSRMLS_CC) );
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

    if ( str_append_len == 0 )
        RETURN_FALSE;

    zarr_hash = Z_ARRVAL_P(zarr);
    zarr_count = zend_hash_num_elements(zarr_hash);

    for(zend_hash_internal_pointer_reset_ex(zarr_hash, &pointer); 
            zend_hash_get_current_data_ex(zarr_hash, (void**) &entry_data, &pointer) == SUCCESS; 
            zend_hash_move_forward_ex(zarr_hash, &pointer)) 
    {
        if ( Z_TYPE_PP(entry_data) == IS_STRING ) {
            str = Z_STRVAL_PP(entry_data);
            str_len = Z_STRLEN_PP(entry_data);

            newpath = path_concat(str, str_len, str_append, str_append_len TSRMLS_CC);
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

    if ( str_prepend_len == 0 )
        RETURN_FALSE;

    zarr_hash = Z_ARRVAL_P(zarr);
    zarr_count = zend_hash_num_elements(zarr_hash);

    for(zend_hash_internal_pointer_reset_ex(zarr_hash, &pointer); 
            zend_hash_get_current_data_ex(zarr_hash, (void**) &entry_data, &pointer) == SUCCESS; 
            zend_hash_move_forward_ex(zarr_hash, &pointer)) 
    {
        if ( Z_TYPE_PP(entry_data) == IS_STRING ) {
            str = Z_STRVAL_PP(entry_data);
            str_len = Z_STRLEN_PP(entry_data);

            newpath = path_concat(str_prepend, str_prepend_len, str, str_len TSRMLS_CC);
            newpath_len = strlen(newpath);

            // free up the previous string
            efree(Z_STRVAL_PP(entry_data));

            Z_STRVAL_PP(entry_data) = newpath;
            Z_STRLEN_PP(entry_data) = newpath_len;
        }
    }
}

PHP_FUNCTION(futil_get_extension)
{
    char *filename;
    int   filename_len;
    char *dot;

    char *extension;
    int   extension_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                    &filename, &filename_len ) == FAILURE) {
        RETURN_FALSE;
    }

    if ( filename_len == 0 )
        RETURN_FALSE;

    dot = strrchr(filename, (int) '.');
    if ( dot != NULL ) {
        extension_len = filename_len - (dot - filename) - 1;

        if ( extension_len == 0 )
            RETURN_FALSE;

        extension = dot + 1;
        RETURN_STRINGL(extension, extension_len , 1);
    }
    RETURN_FALSE;
}


PHP_FUNCTION(futil_replace_extension)
{
    char *filename;
    int   filename_len;

    char *extension;
    int   extension_len;

    char *newfilename;
    int newfilename_len;

    char *dot;
    char *basename;
    int basename_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
                    &filename, &filename_len, &extension, &extension_len
                    ) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong parameters.");
        RETURN_FALSE;
    }

    if ( filename_len == 0 )
        RETURN_FALSE;


    dot = strrchr(filename, (int) '.');
    if ( dot != NULL ) {
        basename_len = dot - filename + 1;
    } else {
        basename_len = filename_len;
        char *new_extension = emalloc( sizeof(char) * (extension_len + 1) );
        *new_extension = '.';
        strncpy(new_extension + 1, extension, extension_len);
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




