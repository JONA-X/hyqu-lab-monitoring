//Sensors
#include <Adafruit_MCP9808.h> // Temperature sensor
#include <Adafruit_BME280.h> // Temperature, pressure, humidity
#include "LSM6DS3.h" // Accelerometer from the Nano, modified library from Arduino
#include "ALS31300.h" // Magnetic Fields

#include <Wire.h> // I2C
#include <RTCZero.h> // RealTimeClock
#include <NTPClient.h> // NTP updating the time
#include "NetworkSettings.h" // Network Seetings
#include <WiFiNINA.h> // Wifi
#include <WiFiUDP.h> // DataObject
#include "DataObject.h"
#include "InfluxDBConnection.h"

// Include watchdog library for resetting Arduino in case of endless loops or other problems
#include <Adafruit_SleepyDog.h>



Adafruit_BME280 bme;
Adafruit_MCP9808 tempsensor;
InfluxDBConnection DBConn = InfluxDBConnection(INFLUXDBADRESS,INFLUXDBNAME,INFLUXUSERNAME,INFLUXDBPASSWORD);

//Adress according to Wiring and Spec Sheet
ALS31300 Mag(96);

//updateinterval to database (s)
const unsigned int databaseinterval = 5;
//origin 10
//updateinterval for the sensors (ms)
const unsigned long millitempupdate = 20; //500
const unsigned long millimagupdate = 100;
const unsigned long send_data_time_interval = 10000; // Send data every 10 seconds
//some large number that the loop starts immediately
unsigned long prevMillis = (unsigned long)0xFFFFFFFFFFFFFFF;
unsigned long millisec = 0;
unsigned long prev_millis_sent_data = (unsigned long)0xFFFFFFFFFFFFFFF;
DataObject Data;
int lastsavedseconds = -1;
//validity of sensors
bool tempValid;
bool bmeValid;
bool magValid;
bool imuValid;

RTCZero rtc;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

int watchdog_countdown = 0;

bool reset_watchdog = true; // As long as this variable is true, the watchdog will regularly be resetted. If this variable is set to false, the watchdog is not resetted or disabled anymore and by latest after 16 seconds, the Arduino will be completely reset
bool reset_watchdog_override_remotely = false;

// Sometimes it happens that the connection to InfluxDB is working well but the sensor just deliver stranges value (e.g. humidity = 100% and temperature of -150°C). 
// Here we define lower and upper limits for some variables. If the value of one sensor does not lie within these expected boundaries, the Arduino will be restarted (with a Watchdog)
float upper_limit_humidity_before_reset = 95;
float lower_limit_humidity_before_reset = 1;
float upper_limit_temperature_before_reset = 40;
float lower_limit_temperature_before_reset = 0;
float upper_limit_acceleration_before_reset = 20;
float lower_limit_acceleration_before_reset = 0.01;


bool arduino_just_resetted = true; // This stores if the Arduino was just resetted, i.e. connects to the database for the first time. This parameter is sent to the database as well to track how often the boards reset (to see which ones are more reliable than others)
#include <ArduinoHttpClient.h>
WiFiClient wifi_client;
HttpClient client_server_reset = HttpClient(wifi_client, REMOTE_RESET_SERVER, 80);


void check_and_reset_watchdog(){
  if(reset_watchdog && !reset_watchdog_override_remotely){
    Watchdog.reset(); // Reset Watchdog
  }
}
void do_immediate_restart(){
  Serial.println("Restart Arduino immediately");
  Watchdog.disable(); // First disable old watchdog
  int watchdog_immediately = Watchdog.enable(100); // Now set a new watchdog with a much shorter time
  delay(200);
}


void setup_wifi(){
  //IPAddress ip(10, 249, 144, 10);
  //WiFi.config(ip);
  //Comment if WPA2, Uncomment for WPA2 enterprise
  //WiFi.beginEnterprise(NETWORKSSID,USERNAME,PASSWORD);
  //Uncomment if WPA2, comment for WPA2 enterprise
  WiFi.begin(NETWORKSSID,PASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    Serial.println("Waiting for connection");
    if(WiFi.status() == 6){
      WiFi.end();
      delay(1000);
      WiFi.begin(NETWORKSSID,PASSWORD);
    }
    //Serial.println(WiFi.status());
    delay(1000);
  }
  WiFi.lowPowerMode();
  Serial.print("Connected to ");
  Serial.println(NETWORKSSID);
}


void check_server_for_remote_reset(){

  String url = REMOTE_RESET_SERVER_URL;
  String sensorname = SENSORNAME;

  Serial.println("Request at server if the Arduino should be reset");


  String httpRequestData = "sensor=" + sensorname;
  String contentType = "application/x-www-form-urlencoded";
  String postData = httpRequestData;
  
  client_server_reset.post("/" + url + "?", contentType, postData);
  int statusCode = client_server_reset.responseStatusCode();
  String response = client_server_reset.responseBody();
  Serial.print("Response: ");
  Serial.println(response);

  if(response == "true"){
    reset_watchdog_override_remotely = true;
    do_immediate_restart(); // Better: restart immediately
  }
  else {
    reset_watchdog_override_remotely = false;
  }
}



void setup() {
  // Enable Serial port for debugging, Setup LED pin
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Serial.begin(9600);

  watchdog_countdown = Watchdog.enable(16000); // More than 16 seconds is not possible.
  Serial.println("Watchdog enabled for 16 seconds.");
  
  check_and_reset_watchdog(); // Reset Watchdog
  setup_wifi();
  check_and_reset_watchdog(); // Reset Watchdog

  
  //Alarm at 2 AM to set the clock
  //Enable I²C
  Wire.begin();
  // Sync the Time with an online service
  timeClient.begin();
  rtc.begin();
  SyncRTC();
  rtc.setAlarmTime(2,0,0);
  rtc.enableAlarm(RTCZero::Alarm_Match::MATCH_HHMMSS);
  rtc.attachInterrupt(SyncRTC);
  
  check_and_reset_watchdog(); // Reset Watchdog
  
  Serial.print("Time: ");
  Serial.print(rtc.getHours());
  Serial.print(":");
  Serial.print(rtc.getMinutes());
  Serial.print(":");
  Serial.println(rtc.getSeconds());
  
  //init Temperature Sensor
  tempValid = tempsensor.begin(0x18);
  if(tempValid){
    Serial.println("MCP9808 temperature sensor successfully set up");
  }
  else {
    Serial.println("MCP9808 (temperature): Error with setup!");
  }
  tempsensor.setResolution(3);
  
  //Init BME280
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
                  
  //init Magnetic Field Sensor
  magValid=Mag.init();
  if(magValid){
    Serial.println("ALS31300 magnetic field sensor successfully set up");
  }
  else{
    Serial.println("ALS31300 (magnetic field): Error with setup!");
  }
  
  //Set up accelerometer
  imuValid = IMU.begin();
  if(imuValid){
    //IMU.setAccelerometer(B01000011,B00000000); //104hz,2g,50hz filter
    IMU.setGyroscope(B00000000,B00000000); // Power-down gyroscope
    Serial.println("LSM6DS3 acceleration sensor successfully set up");
  }
  else{
    Serial.println("LSM6DS3 (acceleration): Error with setup!");
  }
  
  check_and_reset_watchdog(); // Reset Watchdog
  Serial.println("Setup complete\n");
}

void loop() {
  reset_watchdog = true;
  check_and_reset_watchdog(); // Reset Watchdog
  
  millisec = millis();
  
  // update temperature,pressure and humidity values if {millitempupdate} milliseconds have passed
  if(millisec-prevMillis >= millitempupdate){
    float valt = 0;
    float valh = 0;
    float valt2 = 0;
    float valp = 0;
    float acceleration_x, acceleration_y, acceleration_z;
    float acceleration_abs_value = 0;
    if(tempValid){
      valt = tempsensor.readTempC();
      Data.LogTemp(valt);
      //Serial.print("Temperature:");
      //Serial.println(valt);
    }
    if(bmeValid){
      valh = bme.readHumidity();
      Data.LogHum(valh);
      //Serial.print("Humidity:");
      //Serial.println(valh);
      valt2 = bme.readTemperature();
      Data.LogTemp2(valt2);
      valp = bme.readPressure();
      Data.LogPres(valp);
      //Serial.print("Pressure:");
      //Serial.println(val);
    }
    if(magValid){
      auto value = Mag.readFullLoop();
      //Serial.print("\r\n Magnetic Field:");
      //Serial.print(value.mx);
      //Serial.print(",");
      //Serial.print(value.my);
      //Serial.print(",");
      //Serial.println(value.mz);
      Data.LogMagField(value.mx,value.my,value.mz);
    }
    //get accelerometer data
    if(imuValid && IMU.accelerationAvailable()){
      auto value_IMU = IMU.readAcceleration(acceleration_x, acceleration_y, acceleration_z);
      //Serial.print("\r\n Acceleration:");
      //Serial.print(acceleration_x);
      //Serial.print(",");
      //Serial.print(acceleration_y);
      //Serial.print(",");
      //Serial.println(acceleration_z);
      Data.LogAcc(acceleration_x, acceleration_y, acceleration_z);
      acceleration_abs_value = sqrt(acceleration_x*acceleration_x + acceleration_y*acceleration_y + acceleration_z*acceleration_z);
    }
    //set current Millis
    prevMillis=millisec;



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
      do_immediate_restart();
    }
  }


  // Writing data to InfluxDB
  int newseconds = rtc.getSeconds();
  //save whenever clock is at 0 in the last digit, only save once

  check_and_reset_watchdog(); // Reset Watchdog


  bool rtc_did_not_work_send_data_to_late = millisec - prev_millis_sent_data >= send_data_time_interval + 10000;
  if(
    (!(newseconds%10) && lastsavedseconds != newseconds)
    || rtc_did_not_work_send_data_to_late
  ){
    check_server_for_remote_reset();
    
    prev_millis_sent_data = millisec;
    lastsavedseconds = newseconds;
    bool success = false;
    unsigned int counter_trials_connection = 0; // Counts how often it is tried to set up the connection
    Serial.println("\nSending data to database");
    Serial.print("Software version: ");
    Serial.print(SOFTWARE_VERSION);
    Serial.println("");
    digitalWrite(13,HIGH);
    
    while(!success){ // Sometimes, the connection does not immediately work, so it will be repeatedly tried until it works
      check_and_reset_watchdog(); // Reset Watchdog

      Serial.println("Try connecting to database");
      // This is the time critical part. Therefore we do a reset of the watchdog before and after the following function call
      success = DBConn.writeToDataBase(Data, arduino_just_resetted, rtc_did_not_work_send_data_to_late);
      if(success){
        Serial.println("Data successfully written to InfluxDB!");
        arduino_just_resetted = false;
        break;
      }
      else{
        Serial.println("Connection failed");
      }
      
      check_and_reset_watchdog(); // Reset Watchdog
      
      ++counter_trials_connection;
      if(counter_trials_connection >= 2){ // After trying twice unsuccessfully to connect to InfluxDB, reset WiFi
        Serial.println("Try to reconnect to WiFi");
        delay(1000);
        WiFi.end();
        setup_wifi();
      }
      if(counter_trials_connection >= 4){
        Serial.println("Stop resetting the watchdog and let it reset the Arduino.");
        reset_watchdog = false; // Don't reset the watchdog anymore and thus let the Arduino be reset soon by the Watchdog
      }
    }
    digitalWrite(13,LOW);
  }
  
}

void SyncRTC(){
    timeClient.forceUpdate();
    rtc.setEpoch(timeClient.getEpochTime());
}
