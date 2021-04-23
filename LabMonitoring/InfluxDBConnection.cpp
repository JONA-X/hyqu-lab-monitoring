#include "InfluxDBConnection.h"

InfluxDBConnection::InfluxDBConnection(String adress, String dbName, String username, String password) : 
    adress(adress)
    {
        dataBaseString = "POST /write?db=" + dbName + "&precision=s&u=" + username + "&p=" + password + " HTTP/1.1";
    }

InfluxDBConnection::InfluxDBConnection(String adress, String dbName, String accessToken) : 
    adress(adress)
    {
        dataBaseString = "POST /write?db=" + dbName + "&precision=s&b=" + accessToken + " HTTP/1.1";
    }

bool InfluxDBConnection::writeToDataBase(SensorBoard &SensorBoard_obj, bool arduino_just_resetted, bool rtc_did_not_work_send_data_to_late, bool send_additional_debug_data){
    //change to connectSSL if necessary
    if(client.connect(adress.c_str(),port)){
        //clones a HTTP post request
        client.println(dataBaseString);
        client.println("Host: " + adress + ":" + port);
        client.println("Connection: close"); //connection closes after request finished
        client.println("User-Agent: Arduino/1.0"); // "Browser"
        client.print("Content-Length: ");
        String DataString = SensorBoard_obj.getMeasurements("\n", arduino_just_resetted, rtc_did_not_work_send_data_to_late, send_additional_debug_data);
        client.println(DataString.length());
        Serial.println("");
        Serial.println(DataString);
        client.println();
        client.println(DataString);
        bool successful = client.connected();
        client.flush();
        client.stop();
        SensorBoard_obj.reset_data(); // Reset data object
        return successful;
    }
    return false;
}
