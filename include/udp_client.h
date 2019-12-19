#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include "config.h"
#include "conn_ap.h"
#include "espconn.h"

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


// Packet headers
#define UDP_RTINIT  0x06
#define UDP_RTRESP  0x05
#define UDP_TMDEVT  0x02
#define UDP_CLKCOR  0x03
#define UDP_SERINT  0x04


void udpClient_Test();

void StaConectApConfig(char*ssid, char*password);

void udp_clientTimeCorrectionAcc( uint32 ip, int32 delta );
void udp_clientTimeCorrection( struct espconn *client, int32 cor );
void udp_roundTripSend( struct espconn *client );

void UdpRecvServerCb(void *arg, char *pdata, unsigned short len);
void UdpRecvClientCb(void *arg, char *pdata, unsigned short len);

void UdpSendCb(void* arg);
void udpServer(void*arg);
void t1Callback(void* arg);
void udpClient(void*arg);
void WifiConfig(void* arg);


#endif /* UDP_CLIENT_H */
