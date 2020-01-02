#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <string.h>

const char *ssid     = "HUAWEI-B315-5F9A";
const char *password = "NH20M0HYENR";

const long utcOffsetInSeconds = 0;

const unsigned int portno = 5000;
char incomingPacket[255]; // buffer for incoming packets
char replyPacket[255];  // a reply string to send back
char message[255];

int i = 0;
const char server[] = "192.168.8.103" ;

WiFiUDP Udp;

void setup(){
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  Udp.begin(portno);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), portno);
  Serial.printf("RTC time = %d\n", system_get_rtc_time());
  
}

int sync(int portno){
	// parametros locales
	unsigned long currentLogicalClock = millis();
	char buffer[256];
  int packetSize;

	// srand(time(0));						// Initiating the random function with current time as input
  //	currentLogicalClock = (random(5,25));				// Defining the range of random numbers from 5 to 30



	Serial.printf("My logical Clock: %d\n" ,currentLogicalClock); // Printing machine's local logical clock

	do{
		packetSize = Udp.parsePacket();
		delay(10);
	} while(!packetSize);
	int len = Udp.read(incomingPacket, 255);
  if (len > 0)incomingPacket[len] = 0;
  Serial.printf("UDP packet contents at %ld: %s\n", currentLogicalClock,incomingPacket);

  unsigned long tmp = atol(incomingPacket);// esto no se si esta bien

	unsigned long diff = currentLogicalClock - tmp;		// Calculating time difference of local machine from Time Daemon
	Serial.print("My Time Difference from TD: " );
	Serial.println(diff);

	Udp.beginPacket(server, portno);
	sprintf(replyPacket,"%f", diff);

	do{
		packetSize = Udp.parsePacket();
		delay(10);
	} while(!packetSize);
	len = Udp.read(incomingPacket, 255);
  
  if (len > 0) incomingPacket[len] = 0;

	Serial.printf("Average is= %s\n", buffer);

	int adj_clock = atol(incomingPacket);

	currentLogicalClock = currentLogicalClock + adj_clock;

	Serial.printf("My Adjusted clock:  %d\n", currentLogicalClock);

	return currentLogicalClock;
}

void loop(){

	Serial.printf("Iteracao num: %d \n", i);
	sync(portno);
  i = i + 1;
  
//	closeSync();
//  sleep(1);
}
