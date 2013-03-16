
#include "php.h"
#include "php_fileutil.h"
#include "phpdir.h"
#include "path.h"

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <Zend/zend.h>
#include <ext/standard/php_standard.h>
#include <ext/standard/php_filestat.h>

PHPAPI phpdir * phpdir_open(char * dirname TSRMLS_DC)
{
    phpdir * phpdir = emalloc(sizeof(phpdir));
    phpdir->zcontext = NULL;
    phpdir->stream = NULL;
    phpdir->context = NULL;

    phpdir->context = php_stream_context_from_zval(phpdir->zcontext, 0);
    // phpdir->stream = php_stream_opendir(dirname, REPORT_ERRORS, phpdir->context);
    // phpdir->stream = php_stream_opendir(dirname, REPORT_ERRORS, phpdir->context );
    phpdir->stream = php_stream_opendir(dirname, REPORT_ERRORS, phpdir->context );
    if ( ! phpdir->stream ) {
        efree(phpdir);
        return NULL;
    }

    // it's not fclose-able
    phpdir->stream->flags |= PHP_STREAM_FLAG_NO_FCLOSE;
    return phpdir;
}

PHPAPI zval * phpdir_scandir_with_handler( phpdir * phpdir, 
        char* dirname, 
        int dirname_len,
        char* (*handler)(char*, int, php_stream_dirent*) TSRMLS_DC) 
{

    zval *z_list;
    ALLOC_INIT_ZVAL( z_list );
    array_init( z_list );

    php_stream_dirent entry;
    while (php_stream_readdir(phpdir->stream, &entry)) {
        if (strcmp(entry.d_name, "..") == 0 || strcmp(entry.d_name, ".") == 0) {
            continue;
        }
        char * newpath = (*handler)(dirname, dirname_len, &entry);
        if ( newpath != NULL) {
            add_next_index_string(z_list, newpath, strlen(newpath) );
        }
    }
    return z_list;
}

PHPAPI void phpdir_close( phpdir * phpdir TSRMLS_DC) 
{
    // zend_list_delete(phpdir->stream->rsrc_id);
    php_stream_close(phpdir->stream);
    efree(phpdir);
}

PHPAPI char* phpdir_entry_handler(
        char* dirname, 
        int dirname_len, 
        php_stream_dirent * entry )
{
    return path_concat(dirname, dirname_len, entry->d_name, strlen(entry->d_name) TSRMLS_CC);
}


PHPAPI char* phpdir_dir_entry_handler(
        char* dirname, 
        int dirname_len,
        php_stream_dirent * entry )
{
    char * path = path_concat(dirname, dirname_len, entry->d_name, strlen(entry->d_name) TSRMLS_CC);
    int    path_len = dirname_len + strlen(entry->d_name) + 1;
    if ( ! futil_is_dir(path, path_len TSRMLS_CC) ) {
        efree(path);
        return NULL;
    }
    return path;
}

