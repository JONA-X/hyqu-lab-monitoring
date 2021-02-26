
#include "DataObject.h"
#include "NetworkSettings.h"
#include <math.h>

void DataObject::LogTemp(float value){
    Temperature += (value-Temperature)/(++TDataPoints);
}
void DataObject::LogTemp2(float value){
    Temperature2 += (value-Temperature2)/(++T2DataPoints);
}
void DataObject::LogPres(float value){
    Pressure += (value-Pressure)/(++PDataPoints);
}

void DataObject::LogHum(float value){
    Humidity += (value-Humidity)/(++HDataPoints); 
}

void DataObject::LogAcc(float x,float y,float z){
    float absvalue = sqrt(x*x+y*y+z*z);
    AvgAcc += (absvalue-AvgAcc)/(++ADataPoints);
    if(absvalue>MaxAcc) {
        MaxAcc = absvalue;
    }
}

void DataObject::LogMagField(float x,float y, float z){
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
String DataObject::getMeasurements(String separator, bool arduino_just_resetted, bool rtc_did_not_work_send_data_to_late){
    String sensorname = SENSORNAME;
    String retstring =  "Temperature,sensor=" + sensorname + ",sensorPCB=MCP9808 value=" + String(Temperature) + separator +
                        "Temperature,sensor=" + sensorname + ",sensorPCB=BME280 value=" + String(Temperature2) + separator + 
                        "Pressure,sensor=" + sensorname + " value=" + String(Pressure) + separator +
                        "Humidity,sensor=" + sensorname + " value=" + String(Humidity) + separator +
                        "Acceleration,sensor=" + sensorname + " Max=" + String(MaxAcc) + ",Avg=" + String(AvgAcc) + separator +
                        "MagField,sensor=" + sensorname + " abs=" + String(MaxMagAbsField) + ",x_max=" + String(MaxMagField[0]) + ",y_max=" + String(MaxMagField[1])+ ",z_max=" + String(MaxMagField[2]) + 
                        ",x_avg=" + String(AvgMagField[0]) + ",y_avg=" + String(AvgMagField[1])+ ",z_avg=" + String(AvgMagField[2]);
    if(arduino_just_resetted){
      retstring += separator + "Reset,sensor=" + sensorname + " value=1";
    }
    if(rtc_did_not_work_send_data_to_late){
      retstring += separator + "RTCNotWorking,sensor=" + sensorname + " value=1";
    }
    return retstring;
}

void DataObject::Reset(){
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
