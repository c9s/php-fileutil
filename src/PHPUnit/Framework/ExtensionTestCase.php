<?php

abstract class PHPUnit_Framework_ExtensionTestCase extends PHPUnit_Framework_TestCase
{
    abstract function getFunctions();

    public function getExtensionName() 
    {
    }

    public function testExtensionLoad() 
    {
        if( $name = $this->getExtensionName() ) {
            $this->assertTrue( extension_loaded($name) );
        }
    }

    public function testFunctions() 
    {
        $functions = $this->getFunctions();
        foreach( $functions as $function) {
            $this->assertTrue( function_exists($function));
        }
    }
}

