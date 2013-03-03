<?php
require "PHPUnit/TestMore.php";
require "src/PHPUnit/Framework/ExtensionTestCase.php";

class FileUtilTest extends PHPUnit_Framework_ExtensionTestCase
{

    public function getExtensionName()
    {
        return 'fileutil';
    }


    public function getFunctions()
    {
        return array(
            'futil_scandir',
            'futil_scandir_dir',
        );
    }

    public function testReadDir()
    {
        $files = futil_scandir("tests");
        ok($files,"Should return a file list");
        count_ok(1, $files);
    }

}


