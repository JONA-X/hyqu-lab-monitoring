// Sensor libraries
#include <Adafruit_MCP9808.h> // Temperature sensor
#include <Adafruit_BME280.h> // Temperature, pressure, humidity
#include "LSM6DS3.h" // Accelerometer from the Nano, modified library from Arduino
#include "ALS31300.h" // Magnetic Field

// Other libraries
#include <Wire.h> // I2C
#include <RTCZero.h> // RealTimeClock
#include <NTPClient.h> // NTP updating the time
#include <WiFiNINA.h> // Wifi
#include <WiFiUDP.h> // Just needed for RTC
#include <Adafruit_SleepyDog.h> // Include watchdog library for resetting Arduino in case of endless loops or other problems

// Own files
#include "NetworkSettings.h" // Network settings
#include "NetworkFunctions.h" // Network helper functions (connect to WiFi and make POST requests)
#include "InfluxDBConnection.h" // Makes POST request to InfluxDB for sending the measurement data
#include "SensorBoard.h" // Class for storing the measurement data, creating InfluxDB line protocol and requesting board information (room, location, resetting)

// Sensor objects
Adafruit_BME280 bme;
Adafruit_MCP9808 tempsensor;
ALS31300 Mag(96); // Adress according to Wiring and Spec Sheet

// Validity of sensors
bool tempValid;
bool bmeValid;
bool magValid;
bool imuValid;

InfluxDBConnection DBConn = InfluxDBConnection(INFLUXDBADRESS,INFLUXDBNAME,INFLUXUSERNAME,INFLUXDBPASSWORD);

// Objects for time
RTCZero rtc;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// WiFi and sensor board objects
WiFiClient wifi_client;
NetworkFunctions NetworkFunctions_object;
SensorBoard SensorBoard_obj(NetworkFunctions_object);

// Time intervals
const unsigned long update_interval_for_sensors = 20; // Time interval in which the sensors are read for calculating the averages
const unsigned long send_data_time_interval = 10000; // Time interval in which data is sent to InfluxDB
const unsigned long update_parameters_time_interval = 60000; // Time interval in which board parameters are updated. Same time interval is used for sending additional debug info to InfluxDB

unsigned long millisec = 0; // Stores current time
unsigned long prev_millis = (unsigned long)0xFFFFFFFFFFFFFFF; // some large number so that the loop starts immediately
unsigned long prev_millis_sent_data = (unsigned long)0xFFFFFFFFFFFFFFF;
unsigned long prev_millis_updated_parameters = (unsigned long)0xFFFFFFFFFFFFFFF;
int lastsavedseconds = -1;

// Watchdog
int watchdog_countdown = 0;
bool reset_watchdog = true; // As long as this variable is true, the watchdog will regularly be resetted. If this variable is set to false, the watchdog is not resetted or disabled anymore and by latest after 16 seconds, the Arduino will be completely reset

// Sometimes it happens that the connection to InfluxDB is working well but the sensor just deliver stranges value (e.g. humidity = 100% and temperature of -150°C). 
// Here we define lower and upper limits for some variables. If the value of one sensor does not lie within these expected boundaries, the Arduino will be restarted (with a Watchdog)
float upper_limit_humidity_before_reset = 90;
float lower_limit_humidity_before_reset = 5;
float upper_limit_temperature_before_reset = 35;
float lower_limit_temperature_before_reset = 5;
float upper_limit_acceleration_before_reset = 20;
float lower_limit_acceleration_before_reset = 0.01;


bool arduino_just_resetted = true; // This stores if the Arduino was just resetted, i.e. connects to the database for the first time. This parameter is sent to the database as well to track how often the boards reset (to see which ones are more reliable than others)

bool send_additional_debug_data = true; // Should send more debug info to InfluxDB at next connection?




void setup() {
  // Enable Serial port for debugging, Setup LED pin
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Serial.begin(9600);


  watchdog_countdown = Watchdog.enable(16000); // More than 16 seconds is not possible.
  Serial.println("Watchdog enabled for 16 seconds.");
  
  
  SensorBoard_obj.check_and_reset_watchdog(); // Reset Watchdog
  NetworkFunctions_object.setup_wifi(); // Setup Wifi
  SensorBoard_obj.check_and_reset_watchdog(); // Reset Watchdog
  SensorBoard_obj.update_parameters(); // Update board parameters (such as room and location)
  SensorBoard_obj.check_and_reset_watchdog(); // Reset Watchdog
  
  // Enable I2C
  Wire.begin();
  // Sync the time with an online service
  timeClient.begin();
  rtc.begin();
  SyncRTC();
  // Alarm at 2 AM to set the clock
  rtc.setAlarmTime(2,0,0);
  rtc.enableAlarm(RTCZero::Alarm_Match::MATCH_HHMMSS);
  rtc.attachInterrupt(SyncRTC);
  
  SensorBoard_obj.check_and_reset_watchdog(); // Reset Watchdog
  
  Serial.print("Current time: ");
  Serial.print(rtc.getHours());
  Serial.print(":");
  Serial.print(rtc.getMinutes());
  Serial.print(":");
  Serial.println(rtc.getSeconds());
  
  // Init MCP9808 temperature sensor
  tempValid = tempsensor.begin(0x18);
  if(tempValid){
    Serial.println("MCP9808 temperature sensor successfully set up");
  }
  else {
    Serial.println("MCP9808 (temperature): Error with setup!");
  }
  tempsensor.setResolution(3);
  
  // Init BME280
  bmeValid = bme.begin(0x77, &Wire);
  if(bmeValid) {
    Serial.println("BME280 temperature, humidity and pressure sensor successfully set up");
  }
  else {
    Serial.println("BME280 (temperature, humidity and pressure): Error with setup!");
  }
  bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                  Adafruit_BME280::SAMPLING_X1,   // temperature
                  Adafruit_BME280::SAMPLING_X1,   // pressure
                  Adafruit_BME280::SAMPLING_X1,   // humidity
                  Adafruit_BME280::FILTER_X16,
                  Adafruit_BME280::STANDBY_MS_0_5 );
                  
  // Init magnetic field sensor ALS31300
  magValid=Mag.init();
  if(magValid){
    Serial.println("ALS31300 magnetic field sensor successfully set up");
  }
  else{
    Serial.println("ALS31300 (magnetic field): Error with setup!");
  }
  
  // Init accelerometer LSM6DS3
  imuValid = IMU.begin();
  if(imuValid){
    //IMU.setAccelerometer(B01000011,B00000000); //104hz,2g,50hz filter
    IMU.setGyroscope(B00000000,B00000000); // Power-down gyroscope
    Serial.println("LSM6DS3 acceleration sensor successfully set up");
  }
  else{
    Serial.println("LSM6DS3 (acceleration): Error with setup!");
  }
  
  SensorBoard_obj.check_and_reset_watchdog(); // Reset Watchdog
  
  delay(3000);
  Serial.println("Setup complete\n");
}



void loop() {
  
  reset_watchdog = true;
  SensorBoard_obj.check_and_reset_watchdog(); // Reset Watchdog
  
  millisec = millis();
  
  // Update temperature, pressure and humidity values if {update_interval_for_sensors} milliseconds have passed
  if(millisec - prev_millis >= update_interval_for_sensors){
    float valt = 0;
    float valh = 0;
    float valt2 = 0;
    float valp = 0;
    float acceleration_x, acceleration_y, acceleration_z;
    float acceleration_abs_value = 0;
    if(tempValid){
      valt = tempsensor.readTempC();
      SensorBoard_obj.LogTemp(valt);
    }
    if(bmeValid){
      valh = bme.readHumidity();
      SensorBoard_obj.LogHum(valh);
      valt2 = bme.readTemperature();
      SensorBoard_obj.LogTemp2(valt2);
      valp = bme.readPressure();
      SensorBoard_obj.LogPres(valp);
    }
    if(magValid){
      auto value = Mag.readFullLoop();
      SensorBoard_obj.LogMagField(value.mx,value.my,value.mz);
    }
    if(imuValid && IMU.accelerationAvailable()){
      auto value_IMU = IMU.readAcceleration(acceleration_x, acceleration_y, acceleration_z);
      SensorBoard_obj.LogAcc(acceleration_x, acceleration_y, acceleration_z);
      acceleration_abs_value = sqrt(acceleration_x*acceleration_x + acceleration_y*acceleration_y + acceleration_z*acceleration_z);
    }
    
    prev_millis = millisec; // Update previous time for next comparison



    // Check if the lab temperature is in a valid range
    if(valt > upper_limit_temperature_before_reset || valt < lower_limit_temperature_before_reset){
      reset_watchdog = false;
    }
    // Check if the humidity is in a valid range
    if(valh > upper_limit_humidity_before_reset || valh < lower_limit_humidity_before_reset){
      reset_watchdog = false;
    }
    // Check if the acceleration is in a valid range
    if(acceleration_abs_value > upper_limit_acceleration_before_reset || acceleration_abs_value < lower_limit_acceleration_before_reset){
      reset_watchdog = false;
    }

    // Otherwise, if one of the sensors does not seem to work, restart the Arduino (set a very short Watchdog)
    if(!reset_watchdog){
      Serial.println("One of the sensors seems to be wrong. Reset Arduino!");
      SensorBoard_obj.do_immediate_restart();
    }
  }


  // Writing data to InfluxDB
  SensorBoard_obj.check_and_reset_watchdog(); // Reset Watchdog

  if(millisec - prev_millis_updated_parameters >= update_parameters_time_interval){ // Update parameters of the board (such as room and location)
    SensorBoard_obj.check_and_reset_watchdog(); // Reset Watchdog
    SensorBoard_obj.update_parameters();
    SensorBoard_obj.check_and_reset_watchdog(); // Reset Watchdog
    
    prev_millis_updated_parameters = millisec;
    send_additional_debug_data = true; // Send additional debug info to InfluxDB at the next time (soo
  }
  
  int newseconds = rtc.getSeconds();
  bool rtc_did_not_work_send_data_to_late = millisec - prev_millis_sent_data >= send_data_time_interval + 20000;
  if(
    (!(newseconds%10) && lastsavedseconds != newseconds) // Save whenever clock is at 0 in the last digit and only save once
    || rtc_did_not_work_send_data_to_late
  ){
    prev_millis_sent_data = millisec;
    lastsavedseconds = newseconds;
    bool success = false;
    unsigned int counter_trials_connection = 0; // Counts how often it is tried to set up the connection
    Serial.println("\nSending data to database");
    Serial.print("Software version: "); // Just for debugging: If you connect a sensor to your laptop, you will see what sensor version is running on it
    Serial.print(SOFTWARE_VERSION);
    Serial.println("");
    digitalWrite(13,HIGH);
    
    while(!success){ // Sometimes, the connection does not immediately work, so it will be repeatedly tried until it works
      SensorBoard_obj.check_and_reset_watchdog(); // Reset Watchdog

      Serial.println("Try connecting to database");
      // This is the time critical part. Therefore we do a reset of the watchdog before and after the following function call
      success = DBConn.writeToDataBase(SensorBoard_obj, arduino_just_resetted, rtc_did_not_work_send_data_to_late, send_additional_debug_data);
      send_additional_debug_data = false;
      if(success){
        Serial.println("Data successfully written to InfluxDB!");
        arduino_just_resetted = false;
        break;
      }
      else{
        Serial.println("Connection failed");
      }
      
      SensorBoard_obj.check_and_reset_watchdog(); // Reset Watchdog
      
      ++counter_trials_connection;
      if(counter_trials_connection >= 2){ // After trying twice unsuccessfully to connect to InfluxDB, reset WiFi
        Serial.println("Try to reconnect to WiFi");
        delay(1000);
        WiFi.end();
        NetworkFunctions_object.setup_wifi();
      }
      if(counter_trials_connection >= 4){
        Serial.println("Stop resetting the watchdog and let it reset the Arduino.");
        SensorBoard_obj.do_immediate_restart();
      }
    }
    digitalWrite(13,LOW);
  }
  
}


void SyncRTC(){
    timeClient.forceUpdate();
    rtc.setEpoch(timeClient.getEpochTime());
}
