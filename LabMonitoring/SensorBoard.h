#ifndef SENSORBOARDOBJECT
#define SENSORBOARDOBJECT
#include "WiFiNINA.h"
#include <Adafruit_SleepyDog.h>

#include "NetworkFunctions.h"


#include <Arduino.h>

class SensorBoard {
public:  
  SensorBoard(NetworkFunctions NetworkFunctions_object);
  void update_parameters();
  String get_return_from_post_request();
  String get_room();
  String get_location();

  
  void LogTemp(float value);
  void LogTemp2(float value);
  void LogPres(float value);
  void LogHum(float value);
  void LogAcc(float x,float y,float z);
  void LogMagField(float x,float y,float z);
  String getMeasurements(String separator, bool arduino_just_resetted, bool rtc_did_not_work_send_data_to_late, bool send_additional_debug_data);
  void reset_data();
  void do_immediate_restart();
  void check_and_reset_watchdog();

  
private:
  String sensorname;
  NetworkFunctions NetworkFunctions_object;
  char* hostname;
  String server_path;
  String software_version;
  void get_data_array_from_string(String data_string, String return_array[4]);
  String room;
  String location;
  String should_be_reset;
  bool reset_watchdog_override_remotely;
  bool should_be_reset_soon = false;
  

  // Data storage:
  float Temperature = 0;
  unsigned int TDataPoints;
  float Temperature2 = 0;
  unsigned int T2DataPoints;
  float Pressure = 0;
  unsigned int PDataPoints;
  float Humidity = 0;
  unsigned int HDataPoints;
  float AvgAcc = 0;
  float MaxAcc = 0;
  unsigned int ADataPoints;
  float AvgMagField[3] = {0,0,0};
  float MaxMagField[3] = {0,0,0};
  float MaxMagAbsField = 0;
  unsigned int MDataPoints;
};

#endif
