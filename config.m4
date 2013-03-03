
PHP_ARG_ENABLE(fileutil, whether to enable FileUtil support,
[ --enable-fileutil   Enable FileUtil support])
if test "$PHP_FILEUTIL" = "yes"; then
  AC_DEFINE(HAVE_FILEUTIL, 1, [Whether you have FileUtil])
  PHP_NEW_EXTENSION(fileutil, fileutil.c, $ext_shared)
fi

