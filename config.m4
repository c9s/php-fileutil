
PHP_ARG_ENABLE(fileutil,
    [Whether to enable the "fileutil" extension],
    [  --enable-fileutil      Enable "fileutil" extension support])

if test $PHP_FILEUTIL != "no"; then
    PHP_REQUIRE_CXX()
    PHP_SUBST(FILEUTIL_SHARED_LIBADD)
    PHP_ADD_LIBRARY(stdc++, 1, FILEUTIL_SHARED_LIBADD)
    PHP_NEW_EXTENSION(fileutil, php_fileutil.c php_fileutil_rmtree.c path.c, $ext_shared)
fi
