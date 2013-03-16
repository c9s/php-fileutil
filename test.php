<?php
$list = futil_scandir('/etc');
var_dump( $list );

$joined = futil_join('path1/','path2');
$joined2 = futil_join('path1/','/path2');
$joined3 = futil_join('path1/','/path2','/path3','/path4');
