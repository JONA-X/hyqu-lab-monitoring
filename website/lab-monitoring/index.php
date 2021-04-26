<?php


require_once(__DIR__.'/../includes/generate_html.php'); // Include function for generating the HTML page (it adds all code around the main content that is generated in this file)
require_once(__DIR__.'/../includes/databases.php'); // Include functions on the database






$possible_locations_array = get_all_possible_locations($conn); // Array of all possible locations
$sensor_locations_array = get_all_sensor_data($conn);
$success_message = "";


if(isset($_POST["submit"])){ // If the form for changing the mapping between sensors and locations was submitted

	$success = true;
	foreach($sensor_locations_array as &$sensor) { // Loop over all sensors
		$sql_data = array();
		$sql_data["sensorname"] = $sensor[SENSOR_LOCATIONS_SENSORNAME];
		$sql_data["room"] = $_POST["room_sensor_".$sensor[SENSOR_LOCATIONS_SENSORNAME]];
		$sql_data["location"] = $_POST["location_sensor_".$sensor[SENSOR_LOCATIONS_SENSORNAME]];

		try { // Perform update in database according to new values in the form
			$sql = "UPDATE `".SENSOR_LOCATIONS_TABLE_NAME."` SET `".SENSOR_LOCATIONS_ROOM."`=:room, `".SENSOR_LOCATIONS_LOCATION."`=:location WHERE `".SENSOR_LOCATIONS_SENSORNAME."`=:sensorname";
			$stmt = $conn->prepare($sql);
			$stmt->execute($sql_data);

			//$stmt->debugDumpParams();
		} 
		catch(PDOException $e) { 
			$success = false;
			echo "Error: " . $e->getMessage();
		}
	}

	if($success === true){ // Show success message
		$success_message = "
			<div class='alert alert-success' role='alert' class='success_message'><strong>Data stored!</strong> The new data you entered was successfully stored. <button type='button' class='close' data-dismiss='alert' aria-label='Close'><span aria-hidden='true'\>&times;</span></button></div>
		";
	}
	else { // Show error message
		$success_message = "
			<div class='alert alert-danger' role='alert' class='success_message'><strong>There was a problem in the SQL connection!</strong> The data you entered could not be stored. <button type='button' class='close' data-dismiss='alert' aria-label='Close'><span aria-hidden='true'\>&times;</span></button></div>
		";
	}

	$sensor_locations_array = get_all_sensor_data($conn);
}



$form_location_name = "";

if(isset($_POST["submit_locations"])){ // If the form for adding a new location was submitted
	$location_name = $_POST["new_loation_name"];

	if($location_name == ""){ // Show error message that the location name cannot be empty
		$success_message = "
			<div class='alert alert-danger' role='alert' class='success_message'><strong>Problem!</strong> The new location name cannot be empty. <button type='button' class='close' data-dismiss='alert' aria-label='Close'><span aria-hidden='true'\>&times;</span></button></div>
		";
	}
	else {
		$success = insert_new_location($conn, $location_name);
	
		if($success === true){ // Show success message
			$success_message = "
				<div class='alert alert-success' role='alert' class='success_message'><strong>Data stored!</strong> The new location was successfully stored. <button type='button' class='close' data-dismiss='alert' aria-label='Close'><span aria-hidden='true'\>&times;</span></button></div>
			";
		}
		else { // Show error message
			$form_location_name = $location_name;
			$success_message = "
				<div class='alert alert-danger' role='alert' class='success_message'><strong>There was a problem in the SQL connection!</strong> The data you entered could not be stored. <button type='button' class='close' data-dismiss='alert' aria-label='Close'><span aria-hidden='true'\>&times;</span></button></div>
			";
		}
	}

}


if(isset($_POST["submit_update_location_names"])){ // If the form for changing the existing location names was submitted

	$success = true;

	foreach($possible_locations_array as $possible_location){
		$sql_data = array();
		$sql_data["id"] = $possible_location[LOCATIONS_DEFINITION_ID];
		$sql_data["title"] = $_POST["location_name_".$possible_location[LOCATIONS_DEFINITION_ID]];

		try {
			$sql = "UPDATE `".LOCATIONS_DEFINITION_TABLE_NAME."` SET `".LOCATIONS_DEFINITION_TITLE."`=:title WHERE `".LOCATIONS_DEFINITION_ID."`=:id";
			$stmt = $conn->prepare($sql);
			$stmt->execute($sql_data);

			//$stmt->debugDumpParams();
		} 
		catch(PDOException $e) { 
			$success = false;
			echo "Error: " . $e->getMessage();
		}
	}

	if($success === true){ // Show success message
		$success_message = "
			<div class='alert alert-success' role='alert' class='success_message'><strong>Data stored!</strong> The new location titles are successfully stored. <button type='button' class='close' data-dismiss='alert' aria-label='Close'><span aria-hidden='true'\>&times;</span></button></div>
		";
	}
	else { // Show error message
		$success_message = "
			<div class='alert alert-danger' role='alert' class='success_message'><strong>There was a problem in the SQL connection!</strong> The data you entered could not be stored. <button type='button' class='close' data-dismiss='alert' aria-label='Close'><span aria-hidden='true'\>&times;</span></button></div>
		";
	}

}


foreach($possible_locations_array as $possible_location){ // Check for every location if the delete button was pressed
	if(isset($_POST["delete_location_".$possible_location[LOCATIONS_DEFINITION_ID]])){
		$sql_data = array();
		$sql_data["id"] = $possible_location[LOCATIONS_DEFINITION_ID];

		$success = true;
		try {
			$sql = "UPDATE `".LOCATIONS_DEFINITION_TABLE_NAME."` SET `".LOCATIONS_DEFINITION_DELETED."`=1 WHERE `".LOCATIONS_DEFINITION_ID."`=:id"; // The locations are never really deleted, they are just marked as deleted with a flag/column in the table
			$stmt = $conn->prepare($sql);
			$stmt->execute($sql_data);
		} 
		catch(PDOException $e) { 
			$success = false;
			echo "Error: " . $e->getMessage();
		}

		
		if($success === true){ // Show success message
			$success_message = "
				<div class='alert alert-success' role='alert' class='success_message'><strong>Location \"".$possible_location[LOCATIONS_DEFINITION_TITLE]."\" deleted!</strong> <button type='button' class='close' data-dismiss='alert' aria-label='Close'><span aria-hidden='true'\>&times;</span></button></div>
			";
		}
		else { // Show error message
			$success_message = "
				<div class='alert alert-danger' role='alert' class='success_message'><strong>There was a problem in the SQL connection!</strong> The data you entered could not be stored. <button type='button' class='close' data-dismiss='alert' aria-label='Close'><span aria-hidden='true'\>&times;</span></button></div>
			";
		}

		break; // Only one location can be deleted at a time so we can already quit the loop here
	}
}



$possible_locations_array = get_all_possible_locations($conn); // Reload possible locations after change in database




// Create HTML

$html = "
<h1>Lab monitoring</h1>

<h2>Assign locations to sensors</h2>

<form action=\"\" id=\"form_sensor_settings\" method=\"POST\">
</form>

".$success_message."


<div class=\"table table_full_width\">
	<div class=\"table_row\">
		<div class=\"table_cell table_th\">
			Sensor ID
		</div>	
		<div class=\"table_cell table_th\">
			Room
		</div>
		<div class=\"table_cell table_th\">
			Location
		</div>
	</div>
";
foreach($sensor_locations_array as &$sensor) {

	$html .= "
	<div class=\"table_row\">
		<div class=\"table_cell\">
			Sensor ".$sensor[SENSOR_LOCATIONS_SENSORNAME]."
		</div>	
		<div class=\"table_cell\">
			<select name=\"room_sensor_".$sensor[SENSOR_LOCATIONS_SENSORNAME]."\" form=\"form_sensor_settings\">
			";

	foreach($room_options as $room){
		$selected = $room == $sensor[SENSOR_LOCATIONS_ROOM] ? "selected" : "";
		$html .= "<option value=\"".$room."\" ".$selected.">".$room."</option>";
	}

	$html .= "
			</select> 
		</div>
		<div class=\"table_cell\">
			<select name=\"location_sensor_".$sensor[SENSOR_LOCATIONS_SENSORNAME]."\" form=\"form_sensor_settings\">
			";

	foreach($possible_locations_array as $possible_location){
		$selected = ($possible_location[LOCATIONS_DEFINITION_ID] == $sensor[SENSOR_LOCATIONS_LOCATION]) ? "selected" : "";
		$html .= "<option value=\"".$possible_location[LOCATIONS_DEFINITION_ID]."\" ".$selected.">".$possible_location[LOCATIONS_DEFINITION_TITLE]."</option>";
	}

	$html .= "
			</select> 
			<!--<input type=\"text\" name=\"location_sensor_".$sensor[SENSOR_LOCATIONS_SENSORNAME]."\" value=\"".$sensor[SENSOR_LOCATIONS_LOCATION]."\" form=\"form_sensor_settings\">-->
		</div>
	</div>
	";
}




$html .= "
</div>



<input type=\"submit\" value=\"Save\" form=\"form_sensor_settings\" name=\"submit\" class=\"save_form\">


<br><br><br>


<h2>Manage locations</h2>

<h3>Add new location:</h3>
<form action=\"\" method=\"POST\">
	<input type=\"text\" value=\"".$form_location_name."\" name=\"new_loation_name\" class=\"form_locations_text\">
	<input type=\"submit\" value=\"Add new location\" name=\"submit_locations\" class=\"form_locations_text_save\">
</form

<br><hr>

<h3>Current saved locations:</h3>

<form action=\"\" id=\"form_possible_locations\" method=\"POST\">
</form>

<div class=\"table table_wo_border\">
";


foreach($possible_locations_array as $possible_location){
	$selected = ($possible_location[LOCATIONS_DEFINITION_ID] == $sensor[SENSOR_LOCATIONS_LOCATION]) ? "selected" : "";
	$html .= "
	<div class=\"table_row\">
		<div class=\"table_cell\">
			<input type=\"text\" value=\"".$possible_location[LOCATIONS_DEFINITION_TITLE]."\" name=\"location_name_".$possible_location[LOCATIONS_DEFINITION_ID]."\" class=\"form_locations_text\" form=\"form_possible_locations\">
		</div>	
		<div class=\"table_cell cell_delete\">
			<button type=\"button\" form=\"form_possible_locations\" class=\"button_without_style\" onclick=\"show_delete_options_for_location(".$possible_location[LOCATIONS_DEFINITION_ID].")\">
				<i class=\"bi bi-trash icon_trash\"></i>
			</button>
			<input type=\"button\" value=\"Cancel\" class=\"form_locations_text_save button_delete_cancel\" id=\"button_delete_cancel_id_".$possible_location[LOCATIONS_DEFINITION_ID]."\" form=\"form_possible_locations\" onclick=\"hide_delete_options_for_location(".$possible_location[LOCATIONS_DEFINITION_ID].")\">
			<input type=\"submit\" value=\"Delete\" name=\"delete_location_".$possible_location[LOCATIONS_DEFINITION_ID]."\" class=\"form_locations_text_save button_delete\"  id=\"button_delete_id_".$possible_location[LOCATIONS_DEFINITION_ID]."\" form=\"form_possible_locations\">
		</div>	
	</div>";
}


$html .= "
</div>

<input type=\"submit\" value=\"Update location names\" name=\"submit_update_location_names\" class=\"form_locations_text_save\" form=\"form_possible_locations\">


<br><br><br>


<h2>Reset sensors</h2>
<script>
function show_delete_options_for_location(id){
	$('#button_delete_cancel_id_' + id).css(\"display\", \"inline-block\");
	$('#button_delete_id_' + id).css(\"display\", \"inline-block\");
}

function hide_delete_options_for_location(id){
	$('#button_delete_cancel_id_' + id).css(\"display\", \"none\");
	$('#button_delete_id_' + id).css(\"display\", \"none\");
}


function restart_sensor(sensor){
	var success_msg = \"<div class='alert alert-success' role='alert'><strong>Restart successfully saved!</strong> <span id='sensor_name'>\" + sensor + \"</span> will be restarted in the next few seconds. <button type='button' class='close' data-dismiss='alert' aria-label='Close'><span aria-hidden='true'\>&times;</span></button></div>\";

	$.ajax({
		type: \"POST\",
		url: \"/api/lab-monitoring/arduino_should_reset/reset_board.php\",
		data: {sensor: sensor},
		dataType: \"text\", // data type of server response
		success: function(data){
			$('#success_message').html(success_msg);
			console.log(\"Successfully stored data for restarting: \" + sensor + \". Answer by PHP script: \" + data);
		},
		failure: function(errMsg) {
		    alert(errMsg);
		}
	});
}
</script>

<div id=\"success_message\"></div>

";

foreach($sensor_locations_array as &$sensor) {

	$html .= "<button class=\"restart_sensor\" onclick=\"restart_sensor('".$sensor[SENSOR_LOCATIONS_SENSORNAME]."')\">Restart Sensor ".$sensor[SENSOR_LOCATIONS_SENSORNAME]."</button><br>\n";
}






// Define parameters for function that builds the HTML page and call that function
$parameters = array(
	"site_title" => "HyQU - Lab monitoring",
	"html" => $html,
	"linkback" => "/|Back to the main page",
	"js" => "",
	"css" => "",
	"forwarding" => "",
);


echo generate_html($parameters);