<?php

require_once(__DIR__.'/../../includes/databases.php');


$sensor = $_POST["sensor"];


$sensor_data = get_data_for_one_sensor($conn, $sensor);



// Get reset data
$filename = "arduino_should_reset/data/reset_".$sensor.".txt";
$reset_value = 0;
if (file_exists($filename) and file_get_contents($filename) == "true") {
	$reset_value = 1;
	unlink($filename);
}
else {
	$reset_value = 0;
}



echo "room=".$sensor_data[SENSOR_LOCATIONS_ROOM].";location=".$sensor_data["location_str"].";reset=".$reset_value.";sensor=".$sensor.";";