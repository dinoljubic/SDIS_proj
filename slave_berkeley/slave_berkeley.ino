#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <string.h>

const char *ssid     = "HUAWEI-B315-5F9A";
const char *password = "NH20M0HYENR";

const long utcOffsetInSeconds = 0;

const unsigned int portno = 5000;
long adj_clock;

long clock_treshold = 0;
bool on = true;

//char incomingPacket[255]; // buffer for incoming packets
char replyPacket[255];  // a reply string to send back
char message[255];

int i = 0;
const char server[] = "192.168.8.100" ;

WiFiClient client;

void setup(){
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);

  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 2000 );
    Serial.print ( "." );
  }

  Serial.printf("Now listening at IP %s, TCP port %d, from ", server, portno );
  Serial.println(WiFi.localIP());
  Serial.printf("RTC time = %d\n", system_get_rtc_time());
  
}

void sync(int portno){
   while(!client.connect(server, portno)) {
    Serial.println("connection failed");
    delay(1000);
  }
  client.flush();
  client.setDefaultNoDelay(true);
	// parametros locales
	long currentLogicalClock;
	char buffer[256];
	int packetSize;

	Serial.printf("My logical Clock: %d\n" ,currentLogicalClock); // Printing machine's local logical clock

	Serial.print("Esperando");
	while(client.available() == 0){
    delay(500);
		Serial.print(".");
		continue;
	}
  Serial.println();
  
  
  String incomingPacket;
	if(client.available()){ // while o if yo no se
     incomingPacket = client.readStringUntil('\r');
  }
	
	Serial.print("TCP contents at : "); Serial.println(currentLogicalClock);
  Serial.println(incomingPacket);
  
	long tmp = incomingPacket.toDouble();    // esto no se si esta bien
  currentLogicalClock = millis();
	long diff = currentLogicalClock - tmp;		// Calculating time difference of local machine from Time Daemon
	Serial.print("My Time Difference from TD: " );
	Serial.println(diff);
  
	if (client.connected()) {
      client.println(diff);
  }
  client.flush();

  Serial.print("Esperando");
  while(client.available() == 0){
    delay(500);
    Serial.print(".");
    continue;
  }
  Serial.println();
	if(client.available()){ // while o if yo no se
      incomingPacket = client.readStringUntil('\r');
  }
  
	Serial.print("Clock adjustment is= ");
  Serial.println(incomingPacket);

	adj_clock = incomingPacket.toDouble();

	currentLogicalClock = currentLogicalClock + adj_clock;

	Serial.printf("My Adjusted clock:  %d\n", currentLogicalClock);

  client.stop();

}



void loop(){
  long now_clock = millis();
  if( now_clock + adj_clock >= clock_treshold ){
    Serial.printf("Iteracao num: %d \n", i);
    //  startSync();
    sync(portno);
    i = i + 1;
    clock_treshold = clock_treshold + 60000;
  }

  else if( on && (now_clock + adj_clock) % 2000 >= 1000 ){
    digitalWrite(LED_BUILTIN, LOW);
    on = false;
  }

  else if( !on && (now_clock + adj_clock) % 2000 < 1000 ){
    digitalWrite(LED_BUILTIN, HIGH);
    on = true;
    Serial.println(now_clock + adj_clock);
    
  }
  delay(1);  
}
