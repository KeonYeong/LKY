<?php
$fp = fopen("spec.txt", 'a') or die("can't open");
fwrite($fp, "NEW line\n");
fclose($fp);
//echo"<pre>$output</pre>";
?>
