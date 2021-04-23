#ifndef NETWORKFUNCTIONSOBJECT
#define NETWORKFUNCTIONSOBJECT
#include "WiFiNINA.h"

#include <Arduino.h>

class NetworkFunctions {
public:  
  void setup_wifi();
  String get_return_from_post_request(char* hostname, String server_path, String post_data);
private:
  WiFiClient client;
  const unsigned int port = 443;
};

#endif
