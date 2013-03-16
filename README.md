FileUtil Extension
==================

Provide an efficient way to operate file paths.

`string[] futil_scanpath($dir)`:

The behavior is different from the built-in function `scandir($dir)`,
`futil_scanpath` concat the items with the base path, and it filters out "." and
".." paths.

    $list = futil_scanpath('/etc');

    /*
    array( 
        '/etc/af.plist',
        '/etc/afpovertcp.cfg',
        '/etc/asl.conf',
    );
    */

`string[] futil_scanpath_dir($dir)`:

As the same as `futil_scanpath($dir)`, but `futil_scanpath_dir` only returns directory entries.


`long futil_lastmtime($filelist)`:

This returns the last modification time from a filelist array.


