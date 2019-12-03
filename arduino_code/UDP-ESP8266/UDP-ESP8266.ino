#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "SFR-4a38bis";
const char* password = "Master1212";

WiFiUDP Udp;
unsigned int  localUdpPort = 4444;  // local port to listen on
char          incomingPacket[255];  // buffer for incoming packets
char          replyPacket[255] ;  // a reply string to send back

int ss_time_from_server = 0;
int mm_time_from_server = 0;
int hh_time_from_server = 0;

unsigned long external_time = 0;
unsigned long internal_time = 0;

char message[255];

void setup()
{
  Serial.begin(115200);
  Serial.println();

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
  internal_time = millis();
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    // receive incoming UDP packets
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 255);
    if (len > 0)incomingPacket[len] = 0;
    
    Serial.printf("UDP packet contents at %ld: %s\n", internal_time,incomingPacket);
    sscanf(incomingPacket,"%d:%d:%d -- %s",&hh_time_from_server,&mm_time_from_server,&ss_time_from_server,message);
    Serial.printf("Message received --> %s\n",message);
    // send back a reply, to the IP address and port we got the packet from
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    
    Serial.printf("Time from UDP %d:%d:%d \n", hh_time_from_server,mm_time_from_server,ss_time_from_server);
    sprintf(replyPacket,"int_time = %f ext_time = %f",(float)internal_time/1000,(float)ss_time_from_server);
    Udp.write(replyPacket);
    Udp.endPacket();
  }
}


