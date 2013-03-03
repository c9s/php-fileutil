<?php
require "PHPUnit/TestMore.php";
require "src/PHPUnit/Framework/ExtensionTestCase.php";

class FileUtilTest extends PHPUnit_Framework_ExtensionTestCase
{
    public function getFunctions()
    {
        return array(
            'fileutil_readdir',
        );
    }

    public function testReadDir()
    {
        $files = fileutil_readdir("tests");
        ok($files,"Should return a file list");

        count_ok(1, $files);
    }

}


