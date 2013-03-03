#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_fileutil.h"

#include <Zend/zend.h>
#include <ext/standard/php_standard.h>
#include <ext/standard/php_filestat.h>

ZEND_BEGIN_ARG_INFO_EX(arginfo_futil_scandir, 0, 0, 1)
    ZEND_ARG_INFO(0, dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_futil_scandir_dir, 0, 0, 1)
    ZEND_ARG_INFO(0, dir)
ZEND_END_ARG_INFO()

static const zend_function_entry fileutil_functions[] = {
    PHP_FE(futil_scandir, arginfo_futil_scandir)
    PHP_FE(futil_scandir_dir, arginfo_futil_scandir_dir)
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


typedef struct {
    char * string;
    int len;
    int alloc;
} string ;

string * string_new() 
{
    string * s = emalloc(sizeof(string));
    s->string = emalloc(sizeof(char) * 512);
    s->alloc = 512;
    s->len = 1;
    s->string = '\0';
    return s;
}

void string_free(string *s)
{
    efree(s->string);
    efree(s);
}

char * concat_path( char* path1, char * path2 ) 
{
    int len1 = strlen(path1);
    int len2 = strlen(path2);
    char * newpath = emalloc( sizeof(char) * (len1 + len2 + 1) );
    char * p = path1;
    char * p2 = newpath;
    while( len1-- ) {
        *p2 = *p;
        p2++;
        p++;
    }
    *newpath = DEFAULT_SLASH;
    newpath++;
    p = path2;
    while( len2-- ) {
        *p2 = *p;
        p++;
        p2++;
    }
    *newpath = '\0';
    return newpath;
}



typedef struct { 
    php_stream_context *context;
    php_stream *stream;
    zval *zcontext;
} dirp;

dirp* dirp_open(char * dirname) 
{
    dirp * dirp = emalloc(sizeof(dirp));
    dirp->context = NULL;
    dirp->zcontext = NULL;
    dirp->stream = NULL;
    dirp->context = php_stream_context_from_zval(dirp->zcontext, 0);
    dirp->stream = php_stream_opendir(dirname, REPORT_ERRORS, dirp->context);
    if (dirp->stream == NULL) {
        efree(dirp);
        return NULL;
    }

    // it's not fclose-able
    dirp->stream->flags |= PHP_STREAM_FLAG_NO_FCLOSE;
    return dirp;
}

void dirp_scandir_with_func( dirp * dirp , void (*func)(php_stream_dirent*) ) 
{
    php_stream_dirent entry;
    while (php_stream_readdir(dirp->stream, &entry)) {
        if (strcmp(entry.d_name, "..") == 0 || strcmp(entry.d_name, ".") == 0)
            continue;
        (*func)(&entry);
    }
}

void dirp_close( dirp * dirp ) 
{
    zend_list_delete(dirp->stream->rsrc_id);
    php_stream_close(dirp->stream);
    // efree(dirp);
}


bool _futil_stream_is_dir(php_stream *stream)
{
    return (stream->flags & PHP_STREAM_FLAG_IS_DIR);
}

bool _futil_is_dir(char* dirname, int dirname_len)
{
    zval tmp;
    php_stat(dirname, dirname_len, FS_IS_DIR, &tmp TSRMLS_CC);
    return Z_LVAL(tmp) ? true : false;
}


PHP_FUNCTION(futil_scandir_dir)
{


}

PHP_FUNCTION(futil_scandir)
{
    dirp *dirp;
    zval *z_list;
    char *dirname;
    int dirname_len;


    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                    &dirname, &dirname_len ) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong parameters.");
        RETURN_FALSE;
    }


    // run is_dir
    if( ! _futil_is_dir(dirname, dirname_len) ) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "The path is not a directory.");
        RETURN_FALSE;
    }

    ALLOC_INIT_ZVAL( z_list );
    array_init(z_list);

    dirp = dirp_open(dirname);

    if( dirp == NULL ) {
        RETURN_FALSE;
    }

    php_stream_dirent entry;
    while (php_stream_readdir(dirp->stream, &entry)) {
        if (strcmp(entry.d_name, "..") == 0 || strcmp(entry.d_name, ".") == 0)
            continue;

        char *newpath = concat_path(dirname, entry.d_name);
        add_next_index_string(z_list, newpath ,  strlen(newpath) );

        /*
        char *newpath = (char *) emalloc(512);
        int newpath_len = dirname_len + 1 + strlen(entry.d_name);
        sprintf(newpath,"%s%c%s", dirname, DEFAULT_SLASH, entry.d_name);
        add_next_index_string(z_list, newpath ,  strlen(newpath) );
        */
    }

    *return_value = *z_list;
    // add reference count
    zval_copy_ctor(return_value);

    // closedir
    // rsrc_id = dirp->rsrc_id;
    dirp_close(dirp);
}

