#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include "config.h"
#include "conn_ap.h"
#include "espconn.h"


// Packet headers
#define UDP_HDR_BCN     0x01


void UdpRecvCb(void *arg, char *pdata, unsigned short len);
void udp_accNodeTimestamp( uint8 id, uint32 clock );
void udp_broadcast( void );
bool udp_isCorrectable( void );
uint32 udp_getTimeCorrection( void );
void udpClient(void*arg);


#endif /* UDP_CLIENT_H */
