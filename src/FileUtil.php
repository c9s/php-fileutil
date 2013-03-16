<?php

function futil_pathsplit($path)
{
    return explode(DIRECTORY_SEPARATOR, $path);
}

function futil_scanpath($path)
{
    if ( ! file_exists($path) )
        return false;
    if ( is_file($path) )
        return false;

    $list = scandir($path);
    foreach( $list as &$item ) {
        $item = $path . DIRECTORY_SEPARATOR . $item;
    }
    return $list;
}

function futil_scanpath_dir($path)
{
    if ( ! file_exists($path) )
        return false;
    if ( is_file($path) )
        return false;


    $list = scandir($path);
    $newlist = array();
    foreach( $list as $item ) {
        if ( is_dir($item) ) {
            $newlist[] = $path . DIRECTORY_SEPARATOR . $item;
        }
    }
    return $newlist;
}

function futil_lastmtime($filelist)
{
    $lastmtime = 0;
    foreach($filelist as $file) {
        $mtime = filemtime($file);
        if ( $mtime > $lastmtime ) 
            $lastmtime = $mtime;
    }
    return $lastmtime;
}

function futil_lastctime($filelist)
{
    $lastctime = 0;
    foreach($filelist as $file) {
        $ctime = filectime($file);
        if ( $ctime > $lastctime ) 
            $lastctime = $ctime;
    }
    return $lastctime;
}

function futil_pathjoin($a = null)
{
    if( ! is_array($a) ) {
        $a = func_get_args();
    }
    $len = count($a);
    $len1 = $len - 1;
    for ( $i = 0;  $i < $len ; $i++ ) {
        if ( $i < $len1 ) {
            $a[$i] = rtrim($a[$i],DIRECTORY_SEPARATOR);
        }
        if ( $i > 0 ) {
            $a[$i] = ltrim($a[$i],DIRECTORY_SEPARATOR);
        }
    }
    return join(DIRECTORY_SEPARATOR , $a);
}


