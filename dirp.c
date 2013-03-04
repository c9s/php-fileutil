
#include "php.h"
#include "php_fileutil.h"
#include "dirp.h"
#include "path.h"

#include <Zend/zend.h>
#include <ext/standard/php_standard.h>
#include <ext/standard/php_filestat.h>


dirp* dirp_open(char * dirname) 
{
    dirp * dirp = emalloc(sizeof(dirp));
    dirp->context = NULL;
    dirp->zcontext = NULL;
    dirp->stream = NULL;
    dirp->context = php_stream_context_from_zval(dirp->zcontext, 0);

    if(dirp->context == NULL) {
        efree(dirp);
        return NULL;
    }

    dirp->stream = php_stream_opendir(dirname, REPORT_ERRORS, dirp->context);
    if (dirp->stream == NULL) {
        efree(dirp);
        return NULL;
    }

    // it's not fclose-able
    dirp->stream->flags |= PHP_STREAM_FLAG_NO_FCLOSE;
    return dirp;
}

zval* dirp_scandir_with_handler( dirp * dirp, 
        char* dirname, 
        int dirname_len,
        char* (*handler)(char*, int, php_stream_dirent*) ) 
{
    zval *z_list;
    ALLOC_INIT_ZVAL( z_list );
    array_init(z_list);
    
    php_stream_dirent entry;
    while (php_stream_readdir(dirp->stream, &entry)) {
        if (strcmp(entry.d_name, "..") == 0 || strcmp(entry.d_name, ".") == 0)
            continue;
        char * newpath = (*handler)(dirname, dirname_len, &entry);
        if(newpath != NULL) {
            add_next_index_string(z_list, newpath ,  strlen(newpath) );
        }
    }
    return z_list;
}

void dirp_close( dirp * dirp ) 
{
    zend_list_delete(dirp->stream->rsrc_id);
    php_stream_close(dirp->stream);
    // efree(dirp);
}

char* dirp_entry_handler(
        char* dirname, 
        int dirname_len, 
        php_stream_dirent * entry )
{
    return path_concat(dirname, dirname_len, entry->d_name, strlen(entry->d_name) );
}


char* dirp_dir_entry_handler(
        char* dirname, 
        int dirname_len,
        php_stream_dirent * entry )
{
    char * path = path_concat(dirname, dirname_len, entry->d_name, strlen(entry->d_name) );
    int    path_len = dirname_len + strlen(entry->d_name) + 1;
    if( futil_is_dir(path, path_len) ) {
        return path;
    }
    efree(path);
    return NULL;
}


