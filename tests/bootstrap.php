<?php
require 'PHPUnit/TestMore.php';
require 'Universal/ClassLoader/BasePathClassLoader.php';
require '/Users/c9s/git/Work/PHPUnit_Framework_ExtensionTestCase/src/PHPUnit/Framework/ExtensionTestCase.php';

define('BASEDIR',dirname(dirname(__FILE__)));
$classLoader = new \Universal\ClassLoader\BasePathClassLoader(array( 
    BASEDIR . '/src',
    BASEDIR . '/vendor/pear',
));
$classLoader->useIncludePath(false);
$classLoader->register();
