<?php

function futil_pathsplit($path)
{
    return explode(DIRECTORY_SEPARATOR, $path);
}

function futil_scanpath($path)
{
    $list = scandir($path);
    foreach( $list as &$item ) {
        $item = $path . DIRECTORY_SEPARATOR . $item;
    }
    return $list;
}

function futil_scanpath_dir($path)
{
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



