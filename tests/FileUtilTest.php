<?php

class FileUtilTest extends PHPUnit_Framework_ExtensionTestCase
{

    public $repeat = 10;


    public function getExtensionName()
    {
        return 'fileutil';
    }

    public function getFunctions()
    {
        return array(
            'futil_scanpath',
            'futil_scanpath_dir',
            'futil_rmtree',
            'futil_pathjoin',
            'futil_pathsplit',
            'futil_lastmtime',
            'futil_lastctime',
            'futil_replace_extension',
        );
    }


    public function testScanDirOnExistingDir()
    {
        $files = futil_scanpath("tests");
        ok($files,"Should return a file list");
        ok(is_array($files));
        foreach($files as $file) {
            file_exists($file);
            path_ok($file);
        }
    }

    public function testScanDirOnExistingFile()
    {
        $files = futil_scanpath("tests/FileUtilTest.php");
        is(false,$files,"Should return false on file path");
    }

    public function testScanDirOnNonExistingPath()
    {
        $i = $this->repeat;
        while( $i-- ) {
            $files = futil_scanpath("blah/blah");
            is(false,$files,"Should return false on file path");
        }
    }


    public function testScanDirDir()
    {
        $i = $this->repeat;
        while( $i-- ) {
            $paths = futil_scanpath_dir("/");
            ok( $paths );
            foreach($paths as $path) {
                file_exists($path);
                ok(is_dir($path),'is_dir ok');
            }
        }
    }

    public function testPathJoin()
    {
        $joined = futil_pathjoin('path1','path2');
        ok( $joined );
        $this->assertEquals( 'path1/path2' , $joined );
    }

    public function testPathJoinWithDuplicateSlash()
    {
        $joined = futil_pathjoin('tests/','bootstrap.php');
        $this->assertEquals( 'tests/bootstrap.php' , $joined );
        file_exists($joined);

        $joined2 = futil_pathjoin('tests/','/bootstrap.php');
        $this->assertEquals( 'tests/bootstrap.php' , $joined2 );
        file_exists($joined2);

        $joined3 = futil_pathjoin('path1/','/path2','/path3','/path4');
        $this->assertEquals( 'path1/path2/path3/path4' , $joined3 );
    }

    public function testPathSplitWithRoot()
    {
        $parts = futil_pathsplit("/etc/path2/path3");
        count_ok(4, $parts);
    }

    public function testPathSplitRelative()
    {
        $parts = futil_pathsplit("path1/path2/path3");
        count_ok(3,$parts);
    }

    public function testJoinWithArray()
    {
        $joined = futil_pathjoin(array('path1/','path2/','path3'));
        $this->assertEquals( 'path1/path2/path3' , $joined );
    }


    public function testLastmtime()
    {
        $list = array("README.md","config.m4","tests/FileUtilTest.php");
        $mtime = futil_lastmtime($list);
        ok($mtime);
        ok(is_integer($mtime));
    }

    public function testLastctime()
    {
        $list = array("README.md","config.m4","tests/FileUtilTest.php");
        $mtime = futil_lastctime($list);
        ok($mtime);
    }



    public function testRmdirIfExists()
    {
        mkdir("tests/newdir");
        ok( futil_rmdir_if_exists("tests/newdir") );
        futil_rmdir_if_exists("tests/newdir");
    }



    public function testMkdirIfNotExists()
    {
        futil_mkdir_if_not_exists("tests/newdir");
        futil_mkdir_if_not_exists("tests/newdir");
        rmdir("tests/newdir");
    }



    public function testUnlinkIfExists()
    {
        touch("tests/orz");
        ok( file_exists("tests/orz") );
        futil_unlink_if_exists("tests/orz");
        ok( ! file_exists("tests/orz") );
        futil_unlink_if_exists("tests/orz");
        ok( ! file_exists("tests/orz") );
    }


    public function testPathsAppend()
    {
        $list = array( 
            "/dir1",
            "/dir2",
            "/dir3",
        );
        $newlist = futil_paths_append($list, "/file");
        ok( $newlist );
        is( "/dir1/file", $newlist[0] );
        is( "/dir2/file", $newlist[1] );
    }

    public function testPathsAppendValidPath()
    {
        $list = array(
            "tests",
        );
        $newlist = futil_paths_append($list, "/bootstrap.php");
        ok( $newlist );
        is( "tests/bootstrap.php", $newlist[0] );
    }

    public function testPathsAppendWithoutSlashValidPath()
    {
        $list = array(
            "tests",
        );
        $newlist = futil_paths_append($list, "bootstrap.php");
        ok( $newlist );
        is( "tests/bootstrap.php", $newlist[0] );
        path_ok( $newlist[0] );
    }

    public function testPathsRemoveBasepathValidPath()
    {
        $list = array( 
            "/root/tests/bootstrap.php",
        );
        $newlist = futil_paths_remove_basepath($list, "/root/");
        ok( $newlist );
        is("tests/bootstrap.php", $newlist[0]);
        path_ok( $newlist[0] );
    }

    public function testPathsRemoveBasepath()
    {
        $list = array( 
            "/root/file1",
            "/root/file2",
            "/root/file3",
        );
        $newlist = futil_paths_remove_basepath($list, "/root");
        ok( $newlist );
        is("/file1", $newlist[0]);
        is("/file2", $newlist[1]);
        is("/file3", $newlist[2]);
    }

    public function testPathPrependValidPath()
    {
        $list = array( 
            "bootstrap.php",
        );
        $newlist = futil_paths_prepend($list, "tests");
        ok( $newlist );
        is( "tests/bootstrap.php", $newlist[0] );
        path_ok( $newlist[0] );
    }

    public function testPathPrepend()
    {
        $list = array( 
            "/file1",
            "/file2",
            "/file3",
        );
        $newlist = futil_paths_prepend($list, "/root");
        ok( $newlist );
        is( "/root/file1", $newlist[0] );
        is( "/root/file2", $newlist[1] );
    }

    public function testRmtree()
    {
        futil_mkdir_if_not_exists("tests/root/path1/path2",0755,true);
        $files = array();
        $files[] = "tests/root/file1";
        $files[] = "tests/root/file2";
        $files[] = "tests/root/file3";
        $files[] = "tests/root/path1/path2/file4";
        $files[] = "tests/root/path1/path2/file5";
        foreach( $files as $file ) {
            touch( $file );
        }
        futil_rmtree("tests/root");

        foreach( $files as $file ) {
            ok( ! file_exists($file) );
        }
        ok( ! file_exists("tests/root/path1/path2") );
    }

    public function testGetExtension()
    {
        $extension = futil_get_extension("manifest.yml");
        is('yml', $extension);

        $extension = futil_get_extension("manifest.php");
        is('php', $extension);

        $extension = futil_get_extension("manifest");
        is(false, $extension);
    }

    public function testReplaceExtension()
    {
        touch("manifest.php");
        touch("manifest.json");

        $file = futil_replace_extension("manifest.yml","json");
        is( "manifest.json", $file );
        file_exists($file); // ensure it is a valid path
        unlink($file);

        $file = futil_replace_extension("manifest.yml","php");
        is( "manifest.php", $file );
        file_exists($file); // ensure it is a valid path
        unlink($file);
    }

    public function testReplaceExtensionWithNonExtensionFile()
    {
        $i = $this->repeat;
        while( $i-- ) {
            $file = futil_replace_extension("manifest","json");
            is( "manifest.json", $file );
        }
    }

    public function testPrettySize()
    {
        is( '1 KB', futil_prettysize(1024) );
        is( '1.0 MB', futil_prettysize(1024 * 1024) );
        is( '1.0 GB', futil_prettysize(1024 * 1024 * 1024) );
    }


    public function testFilenameSuffixWithExtension()
    {
        $filename = "Hack.png";
        $expected = "Hack_suffix.png";
        touch($expected);

        $newfilename =  futil_filename_append_suffix($filename, "_suffix");
        is( $expected , $newfilename );
        path_ok($newfilename);
        unlink($newfilename);
    }

    public function testFilenameSuffixWithoutExtension()
    {
        is( "Hack2" , futil_filename_append_suffix("Hack", "2") );
        is( "Hack_blah_suffix" , futil_filename_append_suffix("Hack", "_blah_suffix") );
    }

    public function testGetContentsArrayFromFiles()
    {
        $contents = futil_get_contents_array_from_files(array("config.m4","php_fileutil.c"));
        ok($contents,'Get contents');
        ok(is_array($contents),'Should be an array');
        foreach( $contents as $content ) {
            ok($content, "Content OK");
            ok($content['path']);
            ok($content['content']);
            path_ok($content['path']);
        }
    }

    public function testGetContentsFromFiles()
    {
        $i = $this->repeat;
        while( $i-- ) {
            $content = futil_get_contents_from_files(array("config.m4","php_fileutil.c"));
            ok($content);
            unset($content);
        }
    }
}


