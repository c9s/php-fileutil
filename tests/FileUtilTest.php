<?php

class FileUtilTest extends PHPUnit_Framework_ExtensionTestCase
{

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
        $i = 100;
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
        $i = 100;
        while( $i-- ) {
            $files = futil_scanpath("tests/FileUtilTest.php");
            is(false,$files,"Should return false on file path");
        }
    }

    public function testScanDirOnNonExistingPath()
    {
        $i = 100;
        while( $i-- ) {
            $files = futil_scanpath("blah/blah");
            is(false,$files,"Should return false on file path");
        }
    }


    public function testScanDirDir()
    {
        $i = 100;
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
        $i = 100;
        while( $i-- ) {
            $joined = futil_pathjoin('path1','path2');
            ok( $joined );
            $this->assertEquals( 'path1/path2' , $joined );
        }
    }

    public function testJoinWithDuplicateSlash()
    {
        $i = 100;
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
        $i = 100;
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


    public function testJoinArray()
    {
        $i = 100;
        while( $i-- ) {
            $joined = futil_pathjoin(array('path1/','path2/','path3'));
            $this->assertEquals( 'path1/path2/path3' , $joined );
        }
    }
}


