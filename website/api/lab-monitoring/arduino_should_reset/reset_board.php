<?php


$sensor = $_POST["sensor"];

if($sensor != ""){
	$filename = "data/reset_".$sensor.".txt";

	$myfile = fopen($filename, "w") or die("Unable to open file!");
	$txt = "true";
	fwrite($myfile, $txt);
	fclose($myfile);

	echo "success";
}
else {
	echo "Empty sensor name";
}