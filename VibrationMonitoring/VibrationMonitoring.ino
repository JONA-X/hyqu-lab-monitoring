//Sensors
#include "LSM6DS3.h" // Accelerometer from the Nano, modified library from Arduino

#include <Wire.h> // I2C
#include <NTPClient.h> // NTP updating the time
#include "NetworkSettings.h" // Network Seetings
#include <WiFiNINA.h> // Wifi
#include <WiFiUDP.h> // DataObject
#include "InfluxDBConnection.h"

// Include watchdog library for resetting Arduino in case of endless loops or other problems
#include <Adafruit_SleepyDog.h>


#include <math.h>

InfluxDBConnection DBConn = InfluxDBConnection(INFLUXDBADRESS,INFLUXDBNAME,INFLUXUSERNAME,INFLUXDBPASSWORD);


//updateinterval for the sensors (ms)
const unsigned long measurement_interval = 10; // in milliseconds
const unsigned long send_data_time_interval = 30000; // in milliseconds
//some large number that the loop starts immediately
unsigned long prevMillis = (unsigned long)0xFFFFFFFFFFFFFFF;
unsigned long millisec = 0;
unsigned long prev_millis_sent_data = (unsigned long)0xFFFFFFFFFFFFFFF;
int lastsavedseconds = -1;
//validity of sensors
bool imuValid;


int watchdog_countdown = 0;

bool reset_watchdog = true; // As long as this variable is true, the watchdog will regularly be resetted. If this variable is set to false, the watchdog is not resetted or disabled anymore and by latest after 16 seconds, the Arduino will be completely reset
bool reset_watchdog_override_remotely = false;


bool arduino_just_resetted = true; // This stores if the Arduino was just resetted, i.e. connects to the database for the first time. This parameter is sent to the database as well to track how often the boards reset (to see which ones are more reliable than others)



double acceleration_abs_value = 0;
double AvgAcc = 0;
double acceleration_maximum_value = 0;
unsigned int ADataPoints = 0;
    
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

  
  //Enable I²C
  Wire.begin();
  
  check_and_reset_watchdog(); // Reset Watchdog
  
  
  
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
  if(millisec - prevMillis >= measurement_interval){
    float acceleration_x, acceleration_y, acceleration_z;
    
    //get accelerometer data
    if(imuValid && IMU.accelerationAvailable()){
      auto value_IMU = IMU.readAcceleration(acceleration_x, acceleration_y, acceleration_z);
      //Serial.print("\r\n Acceleration:");
      //Serial.print(acceleration_x);
      //Serial.print(",");
      //Serial.print(acceleration_y);
      //Serial.print(",");
      //Serial.println(acceleration_z);
      acceleration_abs_value = sqrt(acceleration_x*acceleration_x + acceleration_y*acceleration_y + acceleration_z*acceleration_z);
      AvgAcc += (acceleration_abs_value-AvgAcc)/(++ADataPoints);
      if(acceleration_abs_value > acceleration_maximum_value) {
          acceleration_maximum_value = acceleration_abs_value;
      }
    }
    //set current Millis
    prevMillis=millisec;

  }


  // Writing data to InfluxDB
  check_and_reset_watchdog(); // Reset Watchdog


  if(millisec - prev_millis_sent_data >= send_data_time_interval){
    prev_millis_sent_data = millisec;
    bool success = false;
    unsigned int counter_trials_connection = 0; // Counts how often it is tried to set up the connection
    Serial.println("");
    digitalWrite(13,HIGH);
    
    while(!success){ // Sometimes, the connection does not immediately work, so it will be repeatedly tried until it works
      check_and_reset_watchdog(); // Reset Watchdog

      Serial.println("Try connecting to database");
      // This is the time critical part. Therefore we do a reset of the watchdog before and after the following function call
      success = DBConn.writeToDataBase(acceleration_maximum_value, AvgAcc);
      if(success){
        Serial.println("Data successfully written to InfluxDB!");
        
        ADataPoints = 0;
        AvgAcc = 0;
        acceleration_maximum_value = 0;
        
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
