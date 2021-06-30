# hyqu-lab-monitoring

## Included libraries
- Adafruit_MCP9808.h: Temperature sensor (standard version from library manager in Arduino IDE)
- Adafruit_BME280.h: Temperature, humidity and pressure sensor (standard version from library manager in Arduino IDE)
- LSM6DS3.h: Acceleration sensor
- ALS31300.h: Magnetic field sensor
- Wire.h
- RTCZero.h (standard version from library manager in Arduino IDE)
- NTPClient.h (standard version from library manager in Arduino IDE)
- WiFiNINA.h
- WiFiUDP.h
- Adafruit_SleepyDog.h: For setting up a watchdog to make the system more robust and reliable (standard version from library manager in Arduino IDE)



## Version history (Arduino Lab Monitoring)
- v5.3_2021-05-14: In the regular debugging interval, also the number of datapoints from which the temperature average is calculated, is written to InfluxDB. That should make it easier to check if there are enough datapoints for a proper average or if more datapoints should be taken. Right now, all measurements (temp, hum, pressure, acc, ...) are made with the same frequency, i.e. the number of datapoints for the averages should be all the same. If this is changed, also the other numbers of datapoints should be written to InfluxDB.