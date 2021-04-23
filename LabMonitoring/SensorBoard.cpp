#include "SensorBoard.h"
#include "NetworkSettings.h"
#include <math.h>



SensorBoard::SensorBoard(NetworkFunctions NetworkFunctions_object) : NetworkFunctions_object(NetworkFunctions_object) {
  hostname = REMOTE_WEBINTERFACE_SERVER;
  sensorname = SENSORNAME;
  server_path = REMOTE_WEBINTERFACE_URL;
  software_version = SOFTWARE_VERSION;

  watchdog_countdown = Watchdog.enable(16000); // More than 16 seconds is not possible.
  Serial.println("Watchdog enabled for 16 seconds.");
}


String SensorBoard::get_room(){
  return room;
}

String SensorBoard::get_location(){
  return location;
}

void SensorBoard::update_parameters(){
  Serial.println("Update board parameters");
  String post_data = "sensor="+sensorname;
  String data_string = NetworkFunctions_object.get_return_from_post_request(hostname, server_path, post_data);
  Serial.println(data_string);

  String return_array[4];
  get_data_array_from_string(data_string, return_array);
  room = return_array[0];
  location = return_array[1];
  should_be_reset = return_array[2];
  Serial.println("New parameters:");
  Serial.println("Room: " + room);
  Serial.println("Location: " + location);
  Serial.println("Reset: " + should_be_reset);
  //Serial.println("Sensor: " + return_array[3]); // Just for debugging to check if the post request works
}




void SensorBoard::get_data_array_from_string(String data_string, String return_array[4]){
  /* The data from the server comes as one string where the data is stored in the format "key1=val1;key2=val2;key3=val3;" 
   * This function transform this string into an array [val1, val2, val3]. Associative arrays would be nicer but their exist no native such arrays in Arduino. Therefore, we define the order of the values as follows:
   * array[0]: Room
   * array[1]: Location in the room
   * array[2]: Boolean that indicates if the Arduino should be reset
   * array[3]: Sensor name. This is transmitted to the server via post. It is therefore not needed to send this back to the Arduino. This is just for DEBUGGING the POST request.
   * 
   * This array is then returned by the function
   */


  unsigned int length_string = data_string.length();
  unsigned int i = 0;
  String previous_string_key = "";
  String previous_string_val = "";
  bool currently_reading_key = true; // true: read key currently, false: read value currently
  while(i < length_string){
    char current_char = data_string[i];
    
    if(current_char == '='){
      currently_reading_key = false; // Now, the value and not the key anymore will be read
    }
    else if(current_char == ';'){
      if(previous_string_key == "room"){
        return_array[0] = previous_string_val;
      }
      else if(previous_string_key == "location"){
        return_array[1] = previous_string_val;
      }
      else if(previous_string_key == "reset"){
        return_array[2] = previous_string_val;
        if(return_array[2] == "1"){ // 1: Do restart, 0: don't
          do_immediate_restart(); // Better: restart immediately
        }
        else {
          reset_watchdog_override_remotely = false;
        }
      }
      else if(previous_string_key == "sensor"){
        return_array[3] = previous_string_val;
      }
      else {
        Serial.println("Not matching key found. Key = " + previous_string_key + ". Value: " + previous_string_val);
      }
      // Reset arrays for next iteration
      previous_string_key = "";
      previous_string_val = "";
      currently_reading_key = true; // Read the key next again
    }
    else {
      if(currently_reading_key){
        previous_string_key += current_char;
      } else {
        previous_string_val += current_char;
      }
    }

    ++i;
  }


}




void SensorBoard::check_and_reset_watchdog(){
  if(!should_be_reset_soon){
    Watchdog.reset(); // Reset Watchdog
  }
}



void SensorBoard::do_immediate_restart(){
  should_be_reset_soon = true;
  /*
  // The following is a nice solution but it can also cause problems. Watchdogs on very small timescales can prevent uploading new code if the old watchdog is not overwritten.
  // So instead, just mark here in a variable that the Arduino should be reset and due to this variable, the watchdog wont be reset and the Arduino resets itself after max. 16s automatically. --> No need for a new shorter watchdog
  Serial.println("Restart Arduino immediately");
  Watchdog.disable(); // First disable old watchdog
  int watchdog_immediately = Watchdog.enable(100); // Now set a new watchdog with a much shorter time
  delay(200);
  */
}






void SensorBoard::LogTemp(float value){
    Temperature += (value-Temperature)/(++TDataPoints);
}
void SensorBoard::LogTemp2(float value){
    Temperature2 += (value-Temperature2)/(++T2DataPoints);
}
void SensorBoard::LogPres(float value){
    Pressure += (value-Pressure)/(++PDataPoints);
}

void SensorBoard::LogHum(float value){
    Humidity += (value-Humidity)/(++HDataPoints); 
}

void SensorBoard::LogAcc(float x,float y,float z){
    float absvalue = sqrt(x*x+y*y+z*z);
    AvgAcc += (absvalue-AvgAcc)/(++ADataPoints);
    if(absvalue>MaxAcc) {
        MaxAcc = absvalue;
    }
}

void SensorBoard::LogMagField(float x,float y, float z){
    ++MDataPoints;
    float value[] = {x,y,z};
    for(int i = 0; i<3; ++i){
        AvgMagField[i] += (value[i]-AvgMagField[i])/(MDataPoints);
    }
    float absvalue = sqrt(value[0]*value[0]+value[1]*value[1]+value[2]*value[2]);
    if(absvalue>MaxMagAbsField) {
        MaxMagAbsField = absvalue;
        MaxMagField[0] = x;
        MaxMagField[1] = y;
        MaxMagField[2] = z;
    }
}


//Gets measurement and resets the class
String SensorBoard::getMeasurements(String separator, bool arduino_just_resetted = false, bool rtc_did_not_work_send_data_to_late = false, bool send_additional_debug_data = false){
    String sensorname = SENSORNAME;
    String retstring =  "Temperature,sensor=" + sensorname  + ",room=" + room + ",location=" + location + ",sensorPCB=MCP9808 value=" + String(Temperature) + separator +
                        "Temperature,sensor=" + sensorname  + ",room=" + room + ",location=" + location + ",sensorPCB=BME280 value=" + String(Temperature2) + separator + 
                        "Pressure,sensor=" + sensorname  + ",room=" + room + ",location=" + location + " value=" + String(Pressure) + separator +
                        "Humidity,sensor=" + sensorname  + ",room=" + room + ",location=" + location + " value=" + String(Humidity) + separator +
                        "Acceleration,sensor=" + sensorname  + ",room=" + room + ",location=" + location + " Max=" + String(MaxAcc) + ",Avg=" + String(AvgAcc) + separator +
                        "MagField,sensor=" + sensorname  + ",room=" + room + ",location=" + location + " abs=" + String(MaxMagAbsField) + ",x_max=" + String(MaxMagField[0]) + ",y_max=" + String(MaxMagField[1])+ ",z_max=" + String(MaxMagField[2]) + 
                        ",x_avg=" + String(AvgMagField[0]) + ",y_avg=" + String(AvgMagField[1])+ ",z_avg=" + String(AvgMagField[2]);
    if(arduino_just_resetted){
      retstring += separator + "Reset,sensor=" + sensorname + " value=1";
    }
    if(rtc_did_not_work_send_data_to_late){
      retstring += separator + "RTCNotWorking,sensor=" + sensorname + " value=1";
    }
    if(send_additional_debug_data){
      retstring += separator + "SoftwareVersion,sensor=" + sensorname + " value=\"" + software_version + "\"";
    }
    return retstring;
}

void SensorBoard::reset_data(){
    Temperature = 0;
    TDataPoints = 0;
    Temperature2 = 0;
    T2DataPoints = 0;
    Pressure = 0;
    PDataPoints = 0;
    Humidity = 0;
    HDataPoints = 0;
    AvgAcc = 0;
    MaxAcc = 0;
    ADataPoints = 0;
    AvgMagField[0] = 0;
    AvgMagField[1] = 0;
    AvgMagField[2] = 0;
    MaxMagField[0] = 0;
    MaxMagField[1] = 0;
    MaxMagField[2] = 0;
    MaxMagAbsField = 0;
    MDataPoints = 0;
}
