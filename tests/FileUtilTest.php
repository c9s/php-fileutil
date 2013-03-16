<?php

class FileUtilTest extends PHPUnit_Framework_ExtensionTestCase
{

    public $repeat = 5;


    public function getExtensionName()
    {
        return 'fileutil';
    }

    public function getFunctions()
    {
        return array(
            'futil_scanpath',
            'futil_scanpath_dir',
            'futil_pathjoin',
        );
    }


    public function testScanDirOnExistingDir()
    {
        $i = $this->repeat;
        while( $i-- ) {
            $files = futil_scanpath("/etc");
            ok($files,"Should return a file list");
            ok(is_array($files));
            foreach($files as $file) {
                path_ok($file);
            }
        }
    }

    public function testScanDirOnExistingFile()
    {
        $i = $this->repeat;
        while( $i-- ) {
            $files = futil_scanpath("tests/FileUtilTest.php");
            is(false,$files,"Should return false on file path");
        }
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
                ok(is_dir($path),'is_dir ok');
            }
        }
    }

    public function testJoin()
    {
        $i = $this->repeat;
        while( $i-- ) {
            $joined = futil_pathjoin('path1','path2');
            ok( $joined );
            $this->assertEquals( 'path1/path2' , $joined );
        }
    }

    public function testJoinWithDuplicateSlash()
    {
        $i = $this->repeat;
        while( $i-- ) {
            $joined = futil_pathjoin('path1/','path2');
            $this->assertEquals( 'path1/path2' , $joined );

            $joined2 = futil_pathjoin('path1/','/path2');
            $this->assertEquals( 'path1/path2' , $joined2 );

            $joined3 = futil_pathjoin('path1/','/path2','/path3','/path4');
            $this->assertEquals( 'path1/path2/path3/path4' , $joined3 );
        }
    }

    public function testPathSplit()
    {
        $i = $this->repeat;
        while( $i-- ) {
            $parts = futil_pathsplit("path1/path2/path3");
            count_ok(3,$parts);
        }
    }

    public function testLastmtime()
    {
        $list = array("README.md","config.m4","tests/FileUtilTest.php");
        $mtime = futil_lastmtime($list);
        ok($mtime);
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

    public function testJoinArray()
    {
        $i = $this->repeat;
        while( $i-- ) {
            $joined = futil_pathjoin(array('path1/','path2/','path3'));
            $this->assertEquals( 'path1/path2/path3' , $joined );
        }
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

}


