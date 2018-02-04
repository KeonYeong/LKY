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
        <h1 class="text-center">Welcome to PLCV!</h1>
        <div style="height: 30px"></div>
        <form action='final_init.php' class="form-inline text-center" method="get">
            <div class="form-group" style="width:800px; height:400px">
                <h2 class = "text-center">Input the assembly Code</h2>
                <textarea class="form-control" id="textArea" rows = "15" name="acode" style="width: 780px" placeholder="Enter Code"></textarea>
                <script type="text/javascript">
                var el = document.getElementById('textArea');
                el.onkeydown = function(e) {
                    if (e.keyCode === 9) {
                        var val = this.value,
                            start = this.selectionStart,
                            end = this.selectionEnd;
                        this.value = val.substring(0, start) + '\t' + val.substring(end);
                        this.selectionStart = this.selectionEnd = start + 1;
                        return false;
                    }
                };
                </script>
            </div>
            <div class="form-group text-left" style="width: 400px; height: 400px">
                <div style="height:50px"></div>
                <p style="letter-spacing: 1px;font-family: Arial Narrow, Sans-serif; color: green; font-size: 20px; width:400px">
                <input class="form-control" name="case[]" type="checkbox" value="1" style="width:15px; height:15px">
                Data hazard detection available
                </p>
                <p style="letter-spacing: 1px;font-family: Arial Narrow, Sans-serif; color: green; font-size: 20px; width:400px">
                <input class="form-control" name="case[]" type="checkbox" value="2" style="width:15px; height:15px">
                Error detection available
                </p>
                <p style="letter-spacing: 1px;font-family: Arial Narrow, Sans-serif; color: green; font-size: 20px; width:400px">
                <input name="case[]" type="checkbox" value="3" style="width:15px; height:15px">
                Binary code dump print
                </p>
                <p style="letter-spacing: 1px;font-family: Arial Narrow, Sans-serif; color: green; font-size: 20px; width:400px">
                <input name="case[]" type="checkbox" value="4" style="width:15px; height:15px">
                Simple branch prediction available
                </p>
            </div>
            <div class="text-center">
                <input type="submit" class="btn btn-default btn-lg" value="Commit!"> 
            </div>
        </form>


        <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js"></script>
        <script type="text/javascript" src="js/bootstrap.js"></script>
    </body>
</html>

