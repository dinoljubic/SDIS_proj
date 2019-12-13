#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include "config.h"
#include "conn_ap.h"

#define UDP_SERVER_GREETING "Hello!This is a udp server test\n"
#define UDP_Client_GREETING "Hello!This is a udp Client test\n"

/*Sta Connect ap config
 #define AP_CONNECT_SSID      "come on baby"
 #define AP_CONNECT_PASSWORD  "1234567890"
 //Softap config
 #define SOFTAP_SSID "ap_test"
 #define SOFTAP_PASSWORD "123456789"
 #define SOFTAP_CHANNEL 5
 */

void udpClient_Test();

void StaConectApConfig(char*ssid, char*password);
void UdpRecvCb(void *arg, char *pdata, unsigned short len);
void UdpSendCb(void* arg);
void udpServer(void*arg);
void t1Callback(void* arg);
void udpClient(void*arg);
void WifiConfig(void* arg);


#endif /* UDP_CLIENT_H */
