<?php
$a = $_GET["anum"];
//$b = $_GET["bnum"];
//$myfile = fopen("newinput.txt", "w");
shell_exec("python tmp2.py " + $a);
//fwrite($myfile, $a);
//fclose($myfile);

echo $a;
?>

<html>
    <head>
        <link rel="stylesheet" href="css/bootstrap.css">
        <script src="http://maxcdn.bootstrapcdn.com/bootstrap/3.2.0/js/bootstrap.min.js"></script>
        <script src="http://code.jquery.com/jquery-latest.min.js"></script>
        <style type="text/css">
            h1
                {
                    font: small-caps 72px Algerian, verdana, 궁서체, "Times New Roman", Sans-serif;
                }
            h2
                {
                    font: oblique 20px verdana, Sans-serif;
                }
        </style>
    </head>
    <body>
        <h1 class="text-center" style="position:absolute; top:500px; left:200px">Loading...</h1>
        <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js"></script>
        <script type="text/javascript" src="js/bootstrap.js"></script>
    </body>
</html>

