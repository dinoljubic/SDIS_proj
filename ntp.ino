#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char *ssid     = "SFR-4a38bis";
const char *password = "Master1212";

const long utcOffsetInSeconds = 0;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 30000);
bool on = false;
long unsigned int last = 0, now = 0;


void setup(){
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);



  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  timeClient.begin();
}

void loop() {
  timeClient.update();

  now = timeClient.getEpochTime();

  if( now % 5 == 0 &&  now != last){
      digitalWrite(LED_BUILTIN, HIGH);
      last = now;
      Serial.print(timeClient.getFormattedTime() + "\n");

  }
  else if(now != last){
      digitalWrite(LED_BUILTIN, LOW);
      last = now;
      Serial.print(timeClient.getFormattedTime() + "\n");
  }

//  if( now % 2 != 0 &&  now != last){
//      digitalWrite(LED_BUILTIN, LOW);
//      last = now;
//      Serial.print(last);
//  }


}