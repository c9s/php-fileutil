FileUtil Extension
==================

[![Build Status](https://travis-ci.org/c9s/php-fileutil.png?branch=master)](https://travis-ci.org/c9s/php-fileutil)

Fast File Utility Extension (with Pure PHP Implementation Support)


Installation
------------


Install the PHP extension:

```sh
git clone https://github.com/c9s/php-fileutil.git
cd php-fileutil
phpize
./configure
make
make install
phpbrew ext enable fileutil # if you use phpbrew
```

Install the pure PHP implementation:

```sh
$ pear channel-discover pear.corneltek.com
$ pear install corneltek/FileUtil
```

Usage
-----

Provide an efficient way to operate file paths.

#### `path[] futil_scanpath($dir)`

The behavior is different from the built-in function `scandir($dir)`,
`futil_scanpath` concat the items with the base path, and it filters out "." and
".." paths.


```php
$list = futil_scanpath('/etc');

/*
array( 
    '/etc/af.plist',
    '/etc/afpovertcp.cfg',
    '/etc/asl.conf',
);
*/
```


#### `path[] futil_scanpath_dir($dir)`

As the same as `futil_scanpath($dir)`, but `futil_scanpath_dir` only returns directory entries.


#### `long futil_lastmtime($filelist)`

This returns the last modification time from a filelist array.


#### `long futil_lastctime($filelist)`

This returns the last creation time from a filelist array.


#### `string futil_pathjoin($path1, $path2, $path3)`

#### `array futil_pathsplit($path)`

#### `futil_unlink_if_exists($path)`

#### `futil_rmdir_if_exists($path)`

#### `futil_mkdir_if_not_exists($path)`

#### `futil_rmtree($path)`

Recursively delete files and directories through RecursiveDirectoryIterator.

