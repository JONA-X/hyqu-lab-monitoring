#include "NetworkFunctions.h"
#include "NetworkSettings.h"


void NetworkFunctions::setup_wifi(){
  //IPAddress ip(10, 249, 144, 10);
  //WiFi.config(ip);
  //Comment if WPA2, Uncomment for WPA2 enterprise
  //WiFi.beginEnterprise(NETWORKSSID,USERNAME,PASSWORD);
  //Uncomment if WPA2, comment for WPA2 enterprise
  
  unsigned long time_start_connecting = millis();
  
  WiFi.begin(NETWORKSSID, PASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    Serial.println("Waiting for connection");
    if(WiFi.status() == 6){
      WiFi.end();
      delay(1000);
      WiFi.begin(NETWORKSSID,PASSWORD);
    }
    

    // If Arduino wasn't able to connect to WiFi within 6 seconds, abort the try and start a new try
    if(millis() - time_start_connecting > 6000){
      Serial.println("");
      Serial.print(F("Unable to connect to network. Starting a new try."));
      time_start_connecting = millis();
      WiFi.disconnect();
      WiFi.begin(NETWORKSSID, PASSWORD); 
    }
    
    delay(1000);
  }
  WiFi.lowPowerMode();
  Serial.print("Connected to ");
  Serial.println(NETWORKSSID);
}


String NetworkFunctions::get_return_from_post_request(char* hostname, String server_path, String post_data){
  String http_method = "POST";

  //Serial.println("Connect to server " + String(hostname) + " over port " + String(port));
  
  if(client.connectSSL(hostname, port)) {// if connected
    //Serial.println("Connected to server");

    // Begin HTTP header
    client.println(http_method + " " + server_path + " HTTP/1.1");
    client.println("Host: " + String(hostname));
    client.println("Content-Type: application/x-www-form-urlencoded"); // This header is needed so that the POST data is sent in the correct format
    client.println("Connection: close");
    // End HTTP header

    client.print("Content-Length: ");
    client.println(post_data.length());
    client.println();
    client.println(post_data);

        
    String return_string = "";
    String html_message = "";
    char prev_char = ' ';
    bool real_part_began = false;
    unsigned int newline_counter = 0;
    
    while(client.connected()) {
      if(client.available()){
        char c = client.read(); // read an incoming byte from the server
        prev_char = c;
        return_string += c;

        if(real_part_began == true){
          if(c == '\n'){
            ++newline_counter;
          }
          // The first two lines are empty and the third line contains the number of bytes of the real payload. Thus, we are only interested in all lines after the third line
          if(newline_counter >= 3){
            if(newline_counter > 3 || c != '\n'){
              html_message += c;
            }
          }
        }


        // Check if the last line of the header was already sent. In that case, now the interesting payload begins
        String last_header_before_payload = "Content-Type: text/html; charset=UTF-8";
        if(return_string.substring(return_string.length() - last_header_before_payload.length(), return_string.length()) == last_header_before_payload){ 
          real_part_began = true;
        }
        
      }
    }
    html_message = html_message.substring(0, html_message.length() - 7); // Delete the last characters of the payload because they are just a 0 marking the end of the payload and whitespace
    //Serial.println(html_message);
    // the server's disconnected, stop the client:
    client.stop();
    return html_message;
  } else {// if not connected:
    Serial.println("connection failed");
    return "0";
  }

}
