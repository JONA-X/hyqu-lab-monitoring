<?php

require_once(__DIR__.'/../includes/database_credentials.php'); // Get credentials from separate file


// Define the names of the tables and column names in the database as constants. Like that, if the names are changed in the database, this has to be modified at only one place in the PHP script
define("SENSOR_LOCATIONS_TABLE_NAME", "sensor_locations"); // Table name
define("SENSOR_LOCATIONS_SENSORNAME", "sensorname"); // Column
define("SENSOR_LOCATIONS_ROOM", "room"); // Column
define("SENSOR_LOCATIONS_LOCATION", "location"); // Column

define("LOCATIONS_DEFINITION_TABLE_NAME", "locations_definition"); // Table name
define("LOCATIONS_DEFINITION_ID", "id"); // Column
define("LOCATIONS_DEFINITION_TITLE", "title"); // Column
define("LOCATIONS_DEFINITION_DELETED", "deleted"); // Column



// The room of the lab are hardcoded because it is unlikely that is changes often which rooms we have for our lab. It is not worth to create a database to store which rooms belong to us.
$room_options = array("F14", "F11", "Somewhere_else");


// Create connection to database
try {
	$conn = new PDO("mysql:host=$servername;dbname=$dbname", $username, $password);
} 
catch(PDOException $e) { 
	echo "Error: " . $e->getMessage();
}



function get_all_possible_locations($conn){ // return array of possible locations such as "dilfridge top", "optical table", "compressor", ...
	try {
		$sql = "SELECT * FROM `".LOCATIONS_DEFINITION_TABLE_NAME."` WHERE `".LOCATIONS_DEFINITION_DELETED."`='0'"; // Get all entries that are not marked as deleted. Note: Locations are never deleted but only hidden so that they can be restored all the time
		$stmt = $conn->prepare($sql);
		$data = array();
		$stmt->execute($data);

	
		$locations_array = array();

		while($row = $stmt->fetch(PDO::FETCH_ASSOC)){
			$locations_array[] = $row;
		}

		return $locations_array;
	} 
	catch(PDOException $e) { 
		echo "Error: " . $e->getMessage();

		return array();
	}

}



function get_all_sensor_data($conn){ // Return array with each row in the first dimension corresponding to one sensor with all stored data about it
	try {
	
		$sql = "SELECT * FROM `".SENSOR_LOCATIONS_TABLE_NAME."` WHERE 1";
		$stmt = $conn->prepare($sql);
		$data = array();
		$stmt->execute($data);

	
		$sensor_locations_array = array();

		while($row = $stmt->fetch(PDO::FETCH_ASSOC)){
			$sensor_locations_array[] = $row;
		}

		return $sensor_locations_array;
	} 
	catch(PDOException $e) { 
		echo "Error: " . $e->getMessage();

		return array();
	}

}



function get_data_for_one_sensor($conn, $sensor_name){ // Return detailed data for one sensor, including the human readable location name (trough the LEFT JOIN we get this name instead of just the id)
	try {
	
		$sql = "SELECT `sensors`.`".SENSOR_LOCATIONS_SENSORNAME."`, `sensors`.`".SENSOR_LOCATIONS_ROOM."`, `locations`.`".LOCATIONS_DEFINITION_TITLE."` as `location_str` FROM `".SENSOR_LOCATIONS_TABLE_NAME."` as `sensors` LEFT JOIN `".LOCATIONS_DEFINITION_TABLE_NAME."` as `locations` ON `sensors`.`".SENSOR_LOCATIONS_LOCATION."`=`locations`.`".LOCATIONS_DEFINITION_ID."` WHERE `sensors`.`".SENSOR_LOCATIONS_SENSORNAME."`=:".SENSOR_LOCATIONS_SENSORNAME."";
		$stmt = $conn->prepare($sql);
		$data = array(
            SENSOR_LOCATIONS_SENSORNAME => $sensor_name
        );
		$stmt->execute($data);
        //$stmt->debugDumpParams();

	
		$sensor_locations_array = array();

		while($row = $stmt->fetch(PDO::FETCH_ASSOC)){
			$sensor_locations_array[] = $row;
		}


		return $sensor_locations_array[0];
	} 
	catch(PDOException $e) { 
		echo "Error: " . $e->getMessage();

		return array();
	}

}


function insert_new_location($conn, $location_name){ // Add new location to list
	try {
	
		$sql = "INSERT INTO `".LOCATIONS_DEFINITION_TABLE_NAME."` (`".LOCATIONS_DEFINITION_TITLE."`, `".LOCATIONS_DEFINITION_DELETED."`)
			VALUES (:".LOCATIONS_DEFINITION_TITLE.", 0)
		"; // The ID is automatically generate by Autoincrement from the database and therefore not specified here
		$stmt = $conn->prepare($sql);
		$data = array(
            LOCATIONS_DEFINITION_TITLE => $location_name
        );
		$stmt->execute($data);
        //$stmt->debugDumpParams();

		return true;
	} 
	catch(PDOException $e) { 
		echo "Error: " . $e->getMessage();

		return false;
	}

}