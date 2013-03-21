<?php
if ( ! extension_loaded('fileutil') ) {

define('SIZE_KB', 1024);
define('SIZE_MB', 1048576);
define('SIZE_GB', 1073741824);

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


function futil_mkdir_if_not_exists($path, $mask = null , $recurse = null)
{
    if ( ! file_exists( $path ) ) {
        return mkdir($path, $mask, $recurse);
    }
    return false;
}

function futil_unlink_if_exists($path)
{
    if ( file_exists($path) ) {
        return unlink($path);
    }
    return false;
}

function futil_rmdir_if_exists($path)
{
    if ( file_exists($path) ) {
        return rmdir($path);
    }
    return false;
}


function futil_rmtree($path)
{
    $iter = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($path), RecursiveIteratorIterator::CHILD_FIRST);
    foreach($iter as $fileinfo){
        if( $fileinfo->isDir() ) {
            if (  $fileinfo->getFilename() === "." 
                || $fileinfo->getFilename() === ".." )
                continue;
            rmdir( $fileinfo->getPathname() );
        } elseif ($fileinfo->isFile()) {
            unlink( $fileinfo->getPathname() );
        }
    }
    if ( is_dir($path) ) {
        rmdir($path);
    } elseif ( is_file($path) ) {
        unlink($path);
    }
    return true;
}


function futil_paths_remove_basepath($paths,$basepath)
{
    return array_map(function($item) use ($basepath) {
        return substr($item,strlen($basepath));
    }, $paths );
}


function futil_paths_append(& $paths,$path2, $modify = false)
{
    if ( $modify ) {
        foreach( $paths as & $path ) {
            $path = futil_pathjoin($path ,$path2);
        }
    } else {
        $newpaths = array();
        foreach( $paths as $path ) {
            $newpaths[] = futil_pathjoin($path ,$path2);
        }
        return $newpaths;
    }
}

function futil_paths_prepend(& $paths,$path2, $modify = false)
{
    if ( $modify ) {
        foreach( $paths as & $path ) {
            $path = futil_pathjoin($path2 ,$path);
        }
    } else {
        $newpaths = array();
        foreach( $paths as $path ) {
            $newpaths[] = futil_pathjoin($path2 ,$path);
        }
        return $newpaths;
    }
}

/**
 * @param array $paths filter out directories
 */
function futil_paths_filter_file($paths)
{
    $newpaths = array();
    foreach( $paths as $path ) {
        if ( is_file($path) ) {
            $newpaths[] = $path;
        }
    }
    return $newpaths;
}


/**
 * @param array $paths filter out directories
 */
function futil_paths_filter_dir($paths)
{
    $newpaths = array();
    foreach( $paths as $path ) {
        if ( is_dir($path) ) {
            $newpaths[] = $path;
        }
    }
    return $newpaths;
}


function futil_replace_extension($filename, $newext)
{
    $parts = explode('.',$filename);
    if (count($parts) > 1) {
        array_pop($parts);
    }
    $parts[] = $newext;
    return join('.',$parts);
}


function futil_get_extension($filename)
{
    return pathinfo($filename, PATHINFO_EXTENSION);
}



function futil_prettysize($bytes)
{
    if ( $bytes < 1024 ) {
        return $bytes . ' B';
    }

    if ( $bytes < SIZE_MB) {
        return sprintf("%d KB", $bytes / SIZE_KB);
    }

    if ( $bytes < SIZE_GB ) {
        return sprintf("%.1f MB", $bytes / SIZE_MB);
    }

    if ( $bytes < SIZE_GB * 1024 ) {
        return sprintf("%.1f GB", $bytes / SIZE_GB );
    }

    return sprintf("%.1f GB", $bytes / SIZE_GB);
}


function futil_filename_append_suffix( $filename , $suffix )
{
    $pos = strrpos( $filename , '.' );
    if ($pos !== false) {
        return substr( $filename , 0 , $pos )
            . $suffix
            . substr( $filename , $pos );
    }
    return $filename . $suffix;
}

function futil_get_contents_from_files($files)
{
    $content = '';
    foreach( $files as $file ) {
        if ( file_exists($file) ) {
            if ( $fileContent = file_get_contents($file) ) {
                $content .= $fileContent;
            }
        }
    }
    return $content;
}


function futil_get_contents_array_from_files($files)
{
    $contents = array();
    foreach( $files as $file ) {
        if ( file_exists($file) ) {
            if ( $fileContent = file_get_contents($file) ) {
                $block = array(
                    'path'    => $file,
                    'content' => $fileContent,
                );
                $contents[] = $block;
            }
        }
    }
    return $contents;
}

}
