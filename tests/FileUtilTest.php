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
}


