<?php
    $current = $_GET["lCur"];
    $myfile = fopen("result.txt", "r");
    $i = 0;
    $numMemory = fgets($myfile, 1024);
    while(!feof($myfile)){
        $content[$i] = fgets($myfile, 1024);
        $i = $i + 1;
    }
    fclose($myfile);
    $numInCycle = $numMemory + 29;
?>
<html>
    <head>
    <title>Result</title>
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
        <h1 class="text-center">Result</h1>
        <div style="height: 30px; width:1000px">
            <h2 class = "text-center"> Now
            <?php
                echo $content[$current*$numInCycle + 0];
            ?>
            Cycle! </h2>
        </div>
        <div class="form-inline" style="height:450px">
            <div class="form-group" style="position: absolute; height:450px; width:1000px">
            <img src="imgs/background.jpg" style="height:450px; width:1000px">
                <div name="PC" style="position:absolute; left:54px; top:213px">
                    <p style="font-family: Arial Narrow, Sans-serif; font-size: 17px; font-color: red">
                        <?php
                            echo $content[$current*$numInCycle + 1];
                        ?>
                    </p>
                </div>
                <div name="IF_ID" style="position: absolute; left:183px; top:0px">
                    <div name="IF_PC" style="position: absolute; left:5px; top:123px">
                       <p style="font-family: Arial Narrow, Sans-serif; font-size: 16px;">
                       <?php
                            echo $content[$current*$numInCycle + $numMemory + 8 + 2 + 1];
                        ?>
                        </p>
                    </div>
                    <div name="IF_INST" style="position: absolute; left:5px; top:200px; width:40px">
                        <?php
                            echo $content[$current*$numInCycle + $numMemory + 8 + 2];
                        ?>
                    </div>
                </div>
                <div name="ID_EX" style="position: absolute; left:427px; top:0px">
                    <div name="ID_PC" style="position: absolute; left:5px; top:123px">
                        <?php
                            echo $content[$current*$numInCycle + $numMemory + 8 + 2 + 2 + 1];
                        ?>
                    </div>
                    <div name="ID_valA" style="position: absolute; left:5px; top:193px">
                        <?php
                            echo $content[$current*$numInCycle + $numMemory + 8 + 2 + 2 + 2];
                        ?>
                    </div>
                    <div name="ID_valB" style="position: absolute; left:5px; top:242px">
                        <?php
                            echo $content[$current*$numInCycle + $numMemory + 8 + 2 + 2 + 3];
                        ?>
                    </div>
                    <div name="ID_offset" style="position: absolute; left:5px; top:293px">
                        <?php
                            echo $content[$current*$numInCycle + $numMemory + 8 + 2 + 2 + 4];
                        ?>
                    </div>

                    <div name="ID_INST" style="position: absolute; left:5px; top:353px; width:40px">
                        <?php
                            echo $content[$current*$numInCycle + $numMemory + 8 + 2 + 2];
                        ?>
                    </div>

                </div>
                <div name="EX_MEM" style="position: absolute; left:655px; top:0px">
                    <div name="EX_target" style="position: absolute; left:5px; top:105px">
                        <?php
                            echo $content[$current*$numInCycle + $numMemory + 8 + 2 + 2 + 5 + 1];
                        ?>
                    </div>
                    <div name="EX_ALU" style="position: absolute; left:5px; top:217px">
                        <?php
                            echo $content[$current*$numInCycle + $numMemory + 8 + 2 + 2 + 5 + 2];
                        ?>
                    </div>
                    <div name="EX_valB" style="position: absolute; left:5px; top:311px">
                        <?php
                            echo $content[$current*$numInCycle + $numMemory + 8 + 2 + 2 + 5 + 3];
                        ?>
                    </div>


                    <div name="EX_INST" style="position: absolute; left:5px; top:353px; width:40px">
                        <?php
                            echo $content[$current*$numInCycle + $numMemory + 8 + 2 + 2 + 5];
                        ?>
                    </div>
                </div>

                <div name="MEM_WB" style="position: absolute; left:867px; top:0px">
                    <div name="MEM_valB" style="position: absolute; left:5px; top:173px">
                        <?php
                            echo $content[$current*$numInCycle + $numMemory + 8 + 2 + 2 + 5 + 2];
                        ?>
                    </div>
                    <div name="MEM_offset" style="position: absolute; left:5px; top:218px">
                        <?php
                            echo $content[$current*$numInCycle + $numMemory + 8 + 2 + 2 + 5 + 4 + 1];
                        ?>
                    </div>

                    <div name="MEM_INST" style="position: absolute; left:5px; top:353px; width:40px">
                        <?php
                            echo $content[$current*$numInCycle + $numMemory + 8 + 2 + 2 + 5 + 4];
                        ?>
                    </div>
                </div>

           </div>
           <div class="form-group text-center" style="position: absolute; left: 1100px; height:450px">
               <h2 class = "text-center"> Register Files </h2>
               <textarea class="text-left" style="font-family: Arial narrow, Sans-serif; font-size:18px; width:100px; height:270px; resize:none">
                    <?php
                        $j = 0;
                        echo "\n";
                        for($j = 0; $j < 8; $j++){
                            echo $content[$current*$numInCycle + $numMemory + 2 + $j];
                        }
                    ?>
               </textarea>
           </div>
           <div class="form-group text-center" style="position: absolute; left: 1300px; height:450px">
                <h2 class="text-center"> Data Memories </h2>
                <textarea class="text-left" style="font-family: Arial narrow, Sans-serif; font-size:18px; width:200px; height:270px; resize:none">
                    <?php
                        $j = 0;
                        echo "\n";
                        for($j = 0; $j < 8; $j++){
                            echo $content[$current*$numInCycle + 2 + $j];
                        }
                    ?>
               </textarea>
           </div>
        </div>
        <div class="text-center form-inline" style="position: relative; top : 50px; height : 200px">
            <form class="form-group" action="final_l.php" method="get">
                <textarea class="form-control" name="lCur" style="display:none"><?=($current-1)?></textarea>
                <button class="btn" id="lArrow" type="submit">
                <img src="imgs/lArrow.png" style="width: 50px; height:50px">
                </button>
            </form>
            <div class="form-group" style="width:50px"></div>
            <form class="form-group" action="final_r.php" method="get">
                <textarea class="form-control" name="rCur" style="display:none"><?=($current+1)?></textarea>
                <button class="btn" id="rArrow" type="submit">
                <img src="imgs/rArrow.png" style="width: 50px; height:50px">
                </button>
            </form>
        </div>
        <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js"></script>
        <script type="text/javascript" src="js/bootstrap.js"></script>
    </body>
</html>


