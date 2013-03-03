<?php
require "PHPUnit/TestMore.php";

abstract class PHPUnit_Framework_ExtensionTestCase extends PHPUnit_Framework_TestCase
{
    abstract function getFunctions();

    public function testFunctions() 
    {
        $functions = $this->getFunctions();
        foreach( $functions as $function) {
            $this->assertTrue( function_exists($function));
        }
    }
}

class FileUtilTest extends PHPUnit_Framework_ExtensionTestCase
{
    public function getFunctions()
    {
        return array(
            'fileutil_readdir',
        );
    }
}


