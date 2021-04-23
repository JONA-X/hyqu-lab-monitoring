#ifndef INFLUXDBCONNECTION
#define INFLUXDBCONNECTION
#include "Arduino.h"
#include "DataBaseConnection.h"
#include "WiFiNINA.h"

class InfluxDBConnection : public DataBaseConnection {
public:
  InfluxDBConnection(String adress, String dbName, String username, String password);
  InfluxDBConnection(String adress, String dbName, String accessToken);
  virtual bool writeToDataBase(SensorBoard &SensorBoard_obj, bool arduino_just_resetted, bool rtc_did_not_work_send_data_to_late, bool send_additional_debug_data) override;
private:
  WiFiClient client;
  String adress;
  String deviceName;
  String dataBaseString;
  const uint32_t port = (uint32_t)80;
};


#endif
