#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char *ssid     = "SFR-4a38bis";
const char *password = "Master1212";



void setup(){
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);

  

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  
}