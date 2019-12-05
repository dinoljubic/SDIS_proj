#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "SFR-4a38bis";
const char* password = "Master1212";

WiFiUDP Udp;
unsigned int  localUdpPort = 4444;  // local port to listen on
char          incomingPacket[255];  // buffer for incoming packets
char          replyPacket[255] ;  // a reply string to send back
char          message[255];

int ss_time_from_server = 0;
int mm_time_from_server = 0;
int hh_time_from_server = 0;

unsigned long external_time = 0;
unsigned long internal_time = 0;

int interval=1000;
unsigned long previousMillis=0;
unsigned int previousTimeFromUDP =0;

boolean sync = true;

void setup()
{
  Serial.begin(115200);
  Serial.println();

  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
  Serial.printf("RTC time = %d\n",system_get_rtc_time());
  
}


void loop(){
  unsigned long currentMillis = millis();
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    Serial.println("-------------------------------------------");
    // receive incoming UDP packets
    //Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 255);
    if (len > 0)incomingPacket[len] = 0;
    
    Serial.printf("UDP packet contents at %ld: %s\n", currentMillis,incomingPacket);
    sscanf(incomingPacket,"%d:%d:%d -- %s",&hh_time_from_server,&mm_time_from_server,&ss_time_from_server,message);
    // send back a reply, to the IP address and port we got the packet from
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    
    Serial.printf("Time from UDP %d:%d:%d \n", hh_time_from_server,mm_time_from_server,ss_time_from_server);
    sprintf(replyPacket,"int_time = %f ext_time = %f",(float)internal_time/1000,(float)ss_time_from_server);

    Serial.printf("currentMillis = %ld\n",currentMillis);
    Serial.printf("previousMillis = %ld\n",previousMillis);
    
    Serial.printf("diff intern = %ld\n",currentMillis-previousMillis);
    Serial.printf("diff UDP    = %ld\n",ss_time_from_server-previousTimeFromUDP);
    previousTimeFromUDP=ss_time_from_server;

    if(ss_time_from_server%10)
      (void) toggleLED(message);
    previousMillis = currentMillis;
   
    
    Udp.write(replyPacket);
    Udp.endPacket();
    Serial.println("-------------------------------------------");
  }
}

void toggleLED(char* message){
  Serial.printf("Message received --> %s\n",message);
  if(strcmp(message,"ON")){
      digitalWrite(LED_BUILTIN, HIGH);
      sync=true;
  }
  else if (strcmp(message,"OFF")){
    digitalWrite(LED_BUILTIN, LOW);
    sync=false;
  }
}


