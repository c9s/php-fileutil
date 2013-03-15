<?php
require 'PHPUnit/TestMore.php';
require 'PHPUnit/Framework/ExtensionTestCase.php';
require 'Universal/ClassLoader/BasePathClassLoader.php';

define('BASEDIR',dirname(dirname(__FILE__)));
$classLoader = new \Universal\ClassLoader\BasePathClassLoader(array( 
    BASEDIR . '/src',
    BASEDIR . '/vendor/pear',
));
$classLoader->useIncludePath(false);
$classLoader->register();
