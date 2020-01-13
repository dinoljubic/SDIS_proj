#include "config.h"
#include "udp_client.h"
#include "clock.h"
#include "boardIO.h"
#include "freertos/queue.h"
// #include "freertos/semphr.h"

const uint8 udp_server_ip[4] = { 192, 168, 47, 1 };
const uint8 udp_client_ip[4] = { 192, 168, 47, 255 };

os_timer_t time1;
// Broadcast espconn for client and server
static struct espconn udp_client;

// Client struct for individual communication
struct {
    uint32 serverIP;
    esp_udp server_udp;
    struct espconn server;
    bool active;
} ClientStruct;

struct {
    uint32 ip;
    esp_udp client_udp;
    struct espconn client;
    uint32 delayAcc;
    uint8 recvdDel;
    bool active;
} ServerStruct [WIFI_AP_MAX_CONN];

// UDP packet received callback
void UdpRecvClientCb(void *arg, char *pdata, unsigned short len)
{
    struct espconn* udp_server_local = arg;
    uint32 time;
    uint8 value;
    uint8 data[8];
    uint32 clock = clock_getMS(), tmp1, tmp2;
    uint32 ip;
    int32 delta;
    // DBG_PRINT("Got packet from port %d\n", udp_server_local->proto.udp->remote_port);
    //udp_client.proto.udp->remote_port=udp_server_local->proto.udp->remote_port;
    //memcpy(&ip, udp_server_local->proto.udp->remote_ip, 4);
    // Parser for received packets
    // DBG_PRINT("header %d\n", *pdata);
    switch(*pdata){
        // CLIENT-ONLY
        case 0x01:  // Disable AP
            os_printf("Packet: %c", pdata[1]);
            if (pdata[1] == 0x00)
                soft_ap_disable();
            break;
        case UDP_TMDEVT:  // Set event at time
            if (ClientStruct.server_udp.remote_port == 0)
                break;
            // DBG_PRINT("Set timer.\n");
            memcpy((char*)&time, &pdata[1], sizeof(uint32));
            value = pdata[5];
            alarm_setTask(time, boardIO_setLED, value);
            break;
        case UDP_CLKCOR:  // Set clock time (ms)
            // DBG_PRINT("Correct time.\n");
            time = clock_getMS();
            memcpy((char*)&delta, &pdata[1], sizeof(int32));
            DBG_PRINT("Correcting time by %d", delta);
            clock_setMS(time+delta);
            break;
        case UDP_SERINT:  // Server introduction - use this port
            ClientStruct.server_udp.remote_port = pdata[1];
            ClientStruct.active = true;
            break;
        case UDP_RTINIT:  // Round-trip request message
            // If server's port is not yet known, do not try to respond
            ClientStruct.server_udp.remote_port = pdata[1]+UDP_SERVER_LOCAL_PORT;
            // DBG_PRINT("Got RT message. Port %d", ClientStruct.server_udp.remote_port);
            // if (ClientStruct.active == false)
            //     break;
            data[0] = UDP_RTRESP;
            memcpy(&data[1], &pdata[2], 4);
            memcpy(&data[5], &clock, 4);
            espconn_send(&ClientStruct.server, data, 9);
            break;
    }
}


void UdpRecvServerCb(void *arg, char *pdata, unsigned short len)
{
    struct espconn* udp_server_local = arg;
    uint32 time;
    uint8 value;
    uint8 data[8];
    uint32 clock = clock_getMS(), tmp1, tmp2;
    uint32 ip;
    int32 delta;
    uint8 *tmp;

    // Get IP - read opposite way
    memcpy(&ip, udp_server_local->proto.udp->remote_ip, 4);
    // for (uint8 i=0; i<4; i++){
    //     ((uint8*)&ip)[i] = udp_server_local->proto.udp->remote_ip[3-i];
    // }
    // DBG_PRINT("Got msg from ip %ud\n", ip);
    // tmp = udp_server_local->proto.udp->remote_ip;
    // ip = (uint32)tmp[0]<<24 | (uint32)tmp[1]<<16 | (uint32)tmp[2]<<8 | (uint32)tmp[3];
    // DBG_PRINT("Got msg from ip %ud\n", ip);

    switch(*pdata){
        // SERVER ONLY
        case UDP_RTRESP:  // Got round-trip response
            memcpy(&tmp1, &pdata[1], 4);
            memcpy(&tmp2, &pdata[5], 4);
            
            // own tmpstamp is in tmp1, client tmpstamp is in tmp2
            delta = tmp1-tmp2+(clock-tmp1)/2;
            /*
            DBG_PRINT("Got correction for client %d.%d.%d.%d, %d", \
                udp_server_local->proto.tcp->remote_ip[0], \
                udp_server_local->proto.tcp->remote_ip[1], \
                udp_server_local->proto.tcp->remote_ip[2], \
                udp_server_local->proto.tcp->remote_ip[3], delta);*/
            udp_clientTimeCorrectionAcc( ip, delta );
            // push to queue
            break;
        default:
            break;
    }
/*
    DBG_LINES("UDP_RECV_CB");
    DBG_PRINT("UDP_RECV_CB len:%d ip:%d.%d.%d.%d port:%d\n", len, udp_server_local->proto.tcp->remote_ip[0],
            udp_server_local->proto.tcp->remote_ip[1], udp_server_local->proto.tcp->remote_ip[2],
            udp_server_local->proto.tcp->remote_ip[3], udp_server_local->proto.tcp->remote_port\
    );*/
    // Echo data to server
    // espconn_send(udp_server_local, pdata, len);

}

void udp_clientSend( uint32 ip, uint8 *pdata, uint8 size ){
    // Not used bc if using semaphores, we want delay between timestamp
    // and sending msg to be as small as possible
    // get semaphore
    memcpy(&udp_client.proto.udp->remote_ip, &ip, 4);
    espconn_send(&udp_client, pdata, (uint16)size);
    // give semaphore
}

// Accumulate corrections
void udp_clientTimeCorrectionAcc( uint32 ip, int32 delta ){
    for (uint8 i=0; i<WIFI_AP_MAX_CONN; i++){
        if (ServerStruct[i].ip == ip){
            ServerStruct[i].delayAcc += delta;
            if (++ServerStruct[i].recvdDel == TIME_CORR_AVERAGE){
                udp_clientTimeCorrection( &ServerStruct[i].client, (int32)ServerStruct[i].delayAcc/TIME_CORR_AVERAGE );
                // reset variables
                ServerStruct[i].delayAcc = 0;
                ServerStruct[i].recvdDel = 0;
            }
        }
    }
}

void udp_clientTimeCorrection( struct espconn *client, int32 cor ){
    uint8 data[10];
    // DBG_PRINT("Correcting client %d from port %d to port %d\n", client->proto.udp->remote_ip, \
        client->proto.udp->local_port, client->proto.udp->remote_port );
    // take semaphore if necessary
    data[0] = UDP_CLKCOR;
    memcpy(&data[1], &cor, 4);
    //memcpy(&udp_client.proto.udp->remote_ip, &ip, 4);
    espconn_send(client, data, 5);
    // give semaphore
}

void udp_roundTripSend( struct espconn *client ){
    uint8 data[10];
    uint32 time;
    // take semaphore if necessary
    time = clock_getMS();
    data[0] = UDP_RTINIT;
    data[1] = (uint8)(client->proto.udp->local_port-UDP_SERVER_LOCAL_PORT);
    memcpy(&data[2], &time, 4);
    //memcpy(&udp_client.proto.udp->remote_ip, &ip, 4);
    espconn_send(client, data, 6);
    // give semaphore
}

void udpServer(void*arg)
{
    //static struct espconn udp_client;
    static esp_udp udp;
    udp_client.type = ESPCONN_UDP;
    udp_client.proto.udp = &udp;
    udp.local_port = UDP_SERVER_LOCAL_PORT;
    udp.remote_port = UDP_CLIENT_LOCAL_PORT;

    memcpy(udp.remote_ip, udp_client_ip, sizeof(udp_client_ip));

    espconn_regist_recvcb(&udp_client, UdpRecvServerCb);
    // espconn_regist_sentcb(&udp_client, UdpSendCb);
    int8 res = 0;
    res = espconn_create(&udp_client);
    if (res != 0) {
        DBG_PRINT("UDP SERVER CREAT ERR ret:%d\n", res);
    }

    struct dhcps_lease dhcp;
    while(wifi_softap_get_dhcps_lease(&dhcp) == false){
        DBG_PRINT("Attempting to get DHCP lease info.");
        vTaskDelay(DELAY_MS(100));
    }

    os_timer_disarm(&time1);
    os_timer_setfn(&time1, t1Callback, NULL);
    os_timer_arm(&time1, 1000, 1);

    // Initialize possible client IPs
    for (uint8 i=0; i<WIFI_AP_MAX_CONN; i++){
        uint32 ip = (uint32)dhcp.start_ip.addr+i;
        ServerStruct[i].ip = dhcp.start_ip.addr+(i<<24);
        // Set address
        memcpy(ServerStruct[i].client_udp.remote_ip, &dhcp.start_ip.addr, 4);
        ServerStruct[i].client_udp.remote_ip[3] += i;
        ServerStruct[i].client_udp.remote_port = UDP_CLIENT_LOCAL_PORT+1;
        ServerStruct[i].client_udp.local_port = UDP_SERVER_LOCAL_PORT+i+1;
        ServerStruct[i].client.type = ESPCONN_UDP;
        ServerStruct[i].client.proto.udp = &ServerStruct[i].client_udp;
        espconn_regist_recvcb(&ServerStruct[i].client, UdpRecvServerCb);
        res = espconn_create(&ServerStruct[i].client);
        if (res != 0) {
            DBG_PRINT("UDP SERVER CREAT ERR ret:%d\n", res);
        }
        else{
            DBG_PRINT("ip: %ud port: %d  local port: %d\n %d.%d.%d.%d\n", ip, ServerStruct[i].client_udp.remote_port, ServerStruct[i].client_udp.local_port, \
              ServerStruct[i].client_udp.remote_ip[0], ServerStruct[i].client_udp.remote_ip[1], \
              ServerStruct[i].client_udp.remote_ip[2], ServerStruct[i].client_udp.remote_ip[3] );
        }
    }

    uint32 start = dhcp.start_ip.addr;
    while(1){
        for (uint8 i=0; i<WIFI_AP_MAX_CONN; i++){
            udp_roundTripSend(&ServerStruct[i].client);
            vTaskDelay(DELAY_MS(50));
            // DBG_PRINT("ip: %d port: %d  local port: %d", ServerStruct[i].client_udp.remote_ip[0], ServerStruct[i].client_udp.remote_port, ServerStruct[i].client_udp.local_port);
        }
        vTaskDelay(DELAY_MS(500));
    }

    vTaskDelete(NULL);
}



void t1Callback(void* arg)
{
    static uint8 value = 0;
    static uint8 data[6];
    
    // Take semaphore
    uint32 clock = clock_getMS()+500;
    data[0] = UDP_TMDEVT;
    memcpy(&data[1], &clock, 4);
    data[5] = value;
    // memcpy(udp_client.proto.udp->remote_ip, udp_client_ip, 4);
    espconn_send(&udp_client, data, 6);
    alarm_setTask(clock, boardIO_setLED, value);
    value = (value == 0);
    /*
    uint32 time, t, del;
    char chtest[CKT_STRLEN]="... -.. .. ...";
    
    t = time = clock_getMS() + 1000;
    for (uint8 i=0;i<CKT_STRLEN;i++){
        
        switch(chtest[i]){
        case '.':
            alarm_setTask(time, boardIO_setLED, 0);
            memcpy(&data[1], &time, 4);
            data[5] = 0;
            espconn_send(&udp_client, data, 6);
            time += CKT_SHO;
            alarm_setTask(time, boardIO_setLED, 1);
            memcpy(&data[1], &time, 4);
            data[5] = 1;
            espconn_send(&udp_client, data, 6);
            break;
        case '-':
            alarm_setTask(time, boardIO_setLED, 0);
            memcpy(&data[1], &time, 4);
            data[5] = 0;
            espconn_send(&udp_client, data, 6);
            time += CKT_LON;
            alarm_setTask(time, boardIO_setLED, 1);
            memcpy(&data[1], &time, 4);
            data[5] = 1;
            espconn_send(&udp_client, data, 6);
            break;
        case ' ':
            time += 2*CKT_SYS;
            break;
        }
        time += CKT_SYS;
    }*/
    // give
}

void udpClient(void*arg)
{
    static esp_udp udp;
    udp_client.type = ESPCONN_UDP;
    udp_client.proto.udp = &udp;
    udp.remote_port = UDP_SERVER_LOCAL_PORT;
    udp.local_port = UDP_CLIENT_LOCAL_PORT;

    memcpy(udp.remote_ip, udp_server_ip, sizeof(udp_server_ip));

    espconn_regist_recvcb(&udp_client, UdpRecvClientCb);
    // espconn_regist_sentcb(&udp_client, UdpSendCb);
    int8 res = 0;
    res = espconn_create(&udp_client);
    if (res != 0) {
        DBG_PRINT("UDP CLIENT CREAT ERR ret:%d\n", res);
    }

    // Individual access
    ClientStruct.server.type = ESPCONN_UDP;
    ClientStruct.server.proto.udp = &ClientStruct.server_udp;
    memcpy(ClientStruct.server_udp.remote_ip, &udp_server_ip, 4);
    ClientStruct.server_udp.local_port = UDP_CLIENT_LOCAL_PORT+1;
    // Remote port is not yet known
    espconn_regist_recvcb(&ClientStruct.server, UdpRecvClientCb);
    res = espconn_create(&ClientStruct.server);
    if (res != 0) {
        DBG_PRINT("UDP CLIENT CREAT ERR ret:%d\n", res);
    }

    while(1){
        // Nothing to do?
        vTaskDelay(DELAY_MS(100));
    }

    vTaskDelete(NULL);
}
