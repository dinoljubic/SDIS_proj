#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <string.h>

const char *ssid     = "SFR-4a38bis";
const char *password = "Master1212";

const long utcOffsetInSeconds = 0;

const unsigned int portno = 5000;
char          incomingPacket[255];  // buffer for incoming packets
char          replyPacket[255] ;  // a reply string to send back
char          message[255];

const string server = "192.168.32.1" ;

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
	char buffer[256];y

	// srand(time(0));						// Initiating the random function with current time as input
	l_clock = (random(5,25));				// Defining the range of random numbers from 5 to 30



	Serial.printf("My logical Clock: %d\n" ,l_clock); // Printing machine's local logical clock

	do{
		int packetSize = Udp.parsePacket();
		delay(10);
	} while(!packetSize);
	int len = Udp.read(incomingPacket, 255);
    if (len > 0)incomingPacket[len] = 0;
    Serial.printf("UDP packet contents at %ld: %s\n", currentMillis,incomingPacket);

    unsigned long tmp = atol(incomingPacket.c_str());

	unsigned long diff = currentLogicalClock - tmp;		// Calculating time difference of local machine from Time Daemon
	Serial.print("My Time Difference from TD: " );
	Serial.println(diff);

	Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());;
	sprintf(replyPacket,"%f", diff);

	do{
		int packetSize = Udp.parsePacket();
		delay(10);
	} while(!packetSize);
	int len = Udp.read(incomingPacket, 255);
    if (len > 0) incomingPacket[len] = 0;

	Serial.printf("Average is= %s\n", buffer);

	int adj_clock = atol(incomingPacket.c_str());

	currentLogicalClock = currentLogicalClock + adj_clock;

	Serial.printf("My Adjusted clock: %f \n", currentLogicalClock);

	return currentLogicalClock;
}

void loop(){

	Serial.println("Iteracao num: " << i << endl);
	sync(portno);
	// closeSync();
	// sleep(1);
}