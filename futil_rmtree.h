#ifndef FUTIL_RMTREE_H
#define FUTIL_RMTREE_H 1

PHP_FUNCTION(futil_rmtree);

int rmtree_iterator(zend_object_iterator *iter, void *puser TSRMLS_DC)

zval *recursive_directory_iterator_create(char * dir, zval * arg, zval * arg2 TSRMLS_DC);
zval *recursive_iterator_iterator_create(zval * iter, zval *arg TSRMLS_DC);

#endif
