<?php
$list = futil_scanpath('/etc');
var_dump( $list );

$i = 100;
while( $i-- ) {
    $joined = futil_pathjoin('path1/','path2');
    $joined2 = futil_pathjoin('path1/','/path2');
    // $joined3 = futil_pathjoin('path1/','/path2');

    $joined3 = futil_pathjoin('path1/','/path2','/path3','/path4');
}
