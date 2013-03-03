<?php
require "PHPUnit/TestMore.php";
require "src/PHPUnit/Framework/ExtensionTestCase.php";

class FileUtilTest extends PHPUnit_Framework_ExtensionTestCase
{
    public function getFunctions()
    {
        return array(
            'futil_readdir',
            'futil_readdir_for_dir',
        );
    }

    public function testReadDir()
    {
        $files = futil_readdir("tests");
        ok($files,"Should return a file list");

        count_ok(1, $files);
    }

}


