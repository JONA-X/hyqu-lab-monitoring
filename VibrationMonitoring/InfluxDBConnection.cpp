#include "InfluxDBConnection.h"
#include "NetworkSettings.h" // Network Seetings

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

bool InfluxDBConnection::writeToDataBase(double MaxAcc, double AvgAcc){
    String room = ROOM;
    String location = LOCATION;
    
    //change to connectSSL if necessary
    if(client.connect(adress.c_str(),port)){
        //clones a HTTP post request
        client.println(dataBaseString);
        client.println("Host: " + adress + ":" + port);
        client.println("Connection: close"); //connection closes after request finished
        client.println("User-Agent: Arduino/1.0"); // "Browser"
        client.print("Content-Length: ");
        String sensorname = SENSORNAME;
        String DataString =  "Acceleration,sensor=" + sensorname + ",room=" + room + ",location=" + location + " Max=" + String(MaxAcc) + ",Avg=" + String(AvgAcc);
        client.println(DataString.length());
        Serial.println("");
        Serial.println(DataString);
        client.println();
        client.println(DataString);
        bool successful = client.connected();
        client.flush();
        client.stop();
        return successful;
    }
    return false;
}
