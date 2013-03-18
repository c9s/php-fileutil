
#include "php_fileutil.h"
#include "path.h"



int rmtree_iterator(zend_object_iterator *iter, void *puser TSRMLS_DC)
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
                            futil_unlink_file(fname, fname_len, NULL TSRMLS_CC);
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




zval *recursive_directory_iterator_create(char * dir, zval * arg, zval * arg2 TSRMLS_DC)
{
    zval *iter;
    MAKE_STD_ZVAL(iter);

    if (SUCCESS != object_init_ex(iter, spl_ce_RecursiveDirectoryIterator)) {
        zval_ptr_dtor(&iter);
        zend_throw_exception_ex(spl_ce_BadMethodCallException, 0 TSRMLS_CC, "Unable to instantiate RecursiveDirectoryIterator for %s", dir);
        return NULL;
    }

    zend_call_method_with_2_params(&iter, spl_ce_RecursiveDirectoryIterator, 
            &spl_ce_RecursiveDirectoryIterator->constructor, "__construct", NULL, arg, arg2);

    if (EG(exception)) {
        // decrease reference count for releasing memory
        zval_ptr_dtor(&iter);
        return NULL;
    }
    return iter;
}


zval *recursive_iterator_iterator_create(zval * iter, zval *arg TSRMLS_DC)
{
    zval * iteriter;

    MAKE_STD_ZVAL(iteriter);
    if (SUCCESS != object_init_ex(iteriter, spl_ce_RecursiveIteratorIterator)) {
        zval_ptr_dtor(&iter);
        zval_ptr_dtor(&iteriter);
        // zend_throw_exception_ex(spl_ce_BadMethodCallException, 0 TSRMLS_CC, "Unable to instantiate directory iterator for %s", dir);
        zend_throw_exception_ex(spl_ce_BadMethodCallException, 0 TSRMLS_CC, "Unable to instantiate RecursiveIteratorIterator.");
        return NULL;
    }

    zend_call_method_with_2_params(&iteriter, spl_ce_RecursiveIteratorIterator, 
            &spl_ce_RecursiveIteratorIterator->constructor, "__construct", NULL, iter, arg);

    if (EG(exception)) {
        // decrease reference count for releasing memory
        zval_ptr_dtor(&iter);
        zval_ptr_dtor(&iteriter);
        return NULL;
    }
    return iteriter;
}



PHP_FUNCTION(futil_rmtree)
{
    char *dir;
    int  dir_len;


    
    char *error = NULL;
    zend_bool apply_reg = 0;
    zval arg, arg2;


    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p",
                    &dir, &dir_len
                    ) == FAILURE) {
        RETURN_FALSE;
    }


    if ( futil_is_file(dir,dir_len TSRMLS_CC) ) {
        futil_unlink_file( dir, dir_len, NULL TSRMLS_CC );
        RETURN_TRUE;
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

    zval * iter;
    if ( (iter = recursive_directory_iterator_create(dir, &arg, &arg2  TSRMLS_CC) ) == NULL )
        RETURN_FALSE;

    zval *iteriter;
    zval iteriter_arg;
    INIT_PZVAL(&iteriter_arg);
#if PHP_VERSION_ID < 50300
    ZVAL_LONG(&iteriter_arg, 0);
#else
    ZVAL_LONG(&iteriter_arg, RIT_CHILD_FIRST);
#endif

    if ( (iteriter = recursive_iterator_iterator_create(iter, &iteriter_arg TSRMLS_CC)) == NULL )
        RETURN_FALSE;

    zval_ptr_dtor(&iter);

    int pass = 0;

    if (SUCCESS == spl_iterator_apply(iteriter, (spl_iterator_apply_func_t) rmtree_iterator, (void *) &pass TSRMLS_CC)) {
        if ( futil_is_dir(dir,dir_len TSRMLS_CC) ) {
            RETURN_BOOL(php_stream_rmdir(dir, REPORT_ERRORS, NULL));
        }
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

