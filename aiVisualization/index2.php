<html>
    <head>
    <title> TITLE</title>
        <link rel="stylesheet" href="css/bootstrap.css">
            <script src="http://maxcdn.bootstrapcdn.com/bootstrap/3.2.0/js/bootstrap.min.js"></script>
            <script src="http://code.jquery.com/jquery-latest.min.js"></script>
            <style type="text/css">
                h1
                    {
                        font: small-caps 72px Algerian, verdana, 궁서체, "Times New Roman", Sans-serif;;
                    }
                h2
                    {
                        font: oblique 20px verdana, Sans-serif;
                    }
            </style>
    </head>
    <body>
		<div style="height:300px"></div>
        <h1 class="text-center">MNIST model Visualization</h1>
        <div style="height: 30px"></div>
        <form action='loading.php' class="form-inline text-center" method="get">
            <div class="form-group" style="width:200px; height:50px">
                <h2 class = "text-center">Image No</h2>
                <textarea class="form-control" id="textArea" rows = "1" name="anum" style="width: 150px" placeholder="Start from 0"></textarea>
            </div>
            <div class="form-group" style="width:200px; height:50px">
                <h2 class = "text-center">Image No.2</h2>
                <textarea class="form-control" id="textArea" rows = "1" name="bnum" style="width: 150px" placeholder="blank for no use"></textarea>
            </div>
            <div class="form-group" style="position:absolute; top:500px">
                <input type="submit" class="btn btn-default btn-lg" value="Check"> 
            </div>
        </form>
        <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js"></script>
        <script type="text/javascript" src="js/bootstrap.js"></script>
    </body>
</html>

