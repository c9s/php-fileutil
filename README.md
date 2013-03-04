FileUtil Extension
==================

Provide an efficient way to operate file paths.


`futil_scandir($dir)`:

The behavior is different from the built-in function `scandir($dir)`,
`futil_scandir` concat the items with the base path, and it filters out "." and
".." paths.

    $list = futil_scandir('/etc');

    /*
    array( 
        '/etc/af.plist',
        '/etc/afpovertcp.cfg',
        '/etc/asl.conf',
    );
    */


`futil_scandir_dir($dir)`:

As the same as `futil_scandir($dir)`, but `futil_scandir_dir` only returns directory entries.

