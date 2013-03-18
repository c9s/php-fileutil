FileUtil Extension
==================

[![Build Status](https://travis-ci.org/c9s/php-fileutil.png?branch=master)](https://travis-ci.org/c9s/php-fileutil)


Fast File Utility Extension (with Pure PHP Implementation Support)

FileUtil Extension provides an efficient way to operate file related operations.


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


Synopsis
--------

```php
$filelist = futil_scanpath("/etc");

$dirlist = futil_scanpath_dir("/etc");

$path = futil_pathjoin("etc","folder","file1"); // Returns "etc/folder/file1", as the same as "etc" . DIRECTORY_SEPARATOR . "folder" . DIRECTORY_SEPARATOR . "file1"
$subpaths  = futil_pathsplit("etc/folder/file1");

$newFilename = futil_replace_extension("manifest.yml","json"); // Returns manifest.json

$extension = futil_get_extension("manifest.yml");   // Returns "yml"

$filelist = array(
    "file1",
    "file2",
    "path2/file3",
    "path3/file4",
    "path4/file5",
);
futil_paths_prepend($filelist, "/root");
/* $filelist = array(
    "/root/file1",
    "/root/file2",
    "/root/path2/file3",
    "/root/path3/file4",
    "/root/path4/file5",
);
*/


// clean up whole directory
if ( false === futil_rmtree("/path/to/delete") ) {

}

$newfilename = futil_filename_append_suffix("Picture.png", "_suffix");  // Returns "Picture_suffix.png"
$newfilename = futil_filename_append_suffix("Picture", "_suffix");  // Returns "Picture_suffix"

$content = futil_get_contents_from_files(array("file1","file2","file3"));

$contents = futil_get_contents_array_from_files(array("config.m4","php_fileutil.c"));
foreach( $contents as $content ) {
    echo $content['path'];
    echo $content['content'];
}



```


Usage
-----
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

#### `futil_paths_prepend($paths, $prepend)`

#### `futil_paths_append($paths, $append)`

#### `futil_replace_extension($path, $newExtension)`

#### `futil_get_extension($path)`

#### `futil_filename_append_suffix($path, $suffix)`


## Hacking

- Fork this project.
- Clone the git repository.
- Run commands under the repository:

        pear channel-discover pear.corneltek.com
        pear channel-update -f corneltek
        pear install -f corneltek/Universal
        pear install -f corneltek/PHPUnit_TestMore
        pear install -f corneltek/PHPUnit_Framework_ExtensionTestCase
        pear install -f corneltek/ExtUnit
        phpize
        ./configure
        make

- Run extunit to test extension functions:

        extunit --phpunit

- Run phpunit to test pure php functions:

        phpunit


## Author

Pedro <cornelius.howl@gmail.com>

## License

See LICENSE file.

