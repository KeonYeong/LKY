<!doctype html>
<html>
<head>
<title>로그인화면</title>
</head>
<body>
<form action = "test.php" method = "get">
ID<input type = "text" name = "id" /> <p>
Password<input type = "password" name = "pwd" /> <p>
<input type = "submit" value = "로그인">
</form>
</body>
</html>

<?php
    echo $_GET["id"]." Welcome."
?>
