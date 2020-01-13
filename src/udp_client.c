#include "config.h"
#include "udp_client.h"
#include "clock.h"
#include "boardIO.h"
#include "freertos/queue.h"
// #include "freertos/semphr.h"

os_timer_t time1;
static struct espconn udp_bc;

// Supporting structures
struct {
    uint8 id;
    uint32 delayAcc;
    uint8 recvdTmstp;
    bool active;
    uint8 trials;   // for monitoring inactivity, to clear active flag
} nodesStruct [WIFI_AP_MAX_CONN];

struct {
    uint8 id;
    bool bcActive;
    struct espconn udp_listener;
    esp_udp udp;
} configStruct;


// UDP packet received callback
void UdpRecvCb(void *arg, char *pdata, unsigned short len)
{
    struct espconn* udp_server_local = arg;
    uint32 time;
    uint8 value;
    uint8 data[8];
    uint32 ticks = clock_getLastLoad()-clock_getTicks();
    uint8 id;
    uint32 t;
   
    switch(*pdata){
    case UDP_HDR_BCN:
        id = pdata[1];
        udp_accNodeTimestamp(id, ticks);
        break;
    default:
        break;
    }
}

void udp_accNodeTimestamp( uint8 id, uint32 clock ){
    // TODO: Compensate clock for constant delay?
    uint8 freeIndex = -1;
    for (uint8 i=0;i<WIFI_AP_MAX_CONN;i++){
        if (nodesStruct[i].id == id){
            nodesStruct[i].delayAcc += clock;
            nodesStruct[i].recvdTmstp++;
            return;
        }
        if (nodesStruct[i].active == false)
            freeIndex = i;
    }
    // If received for the first time, find free slot
    if (freeIndex != -1){
        nodesStruct[freeIndex].id = id;
        nodesStruct[freeIndex].active = true;
        nodesStruct[freeIndex].delayAcc += clock;
        nodesStruct[freeIndex].recvdTmstp++;
    }
}


void udp_broadcast( void ){
    uint8 data[2];
    if (configStruct.bcActive == false)
        return;
    data[0] = UDP_HDR_BCN;
    data[1] = configStruct.id;
    espconn_send(&udp_bc, data, 2);
}


bool udp_isCorrectable( void ){
    for (uint8 i=0;i<WIFI_AP_MAX_CONN;i++){
        if (nodesStruct[i].active == true &&    \
            nodesStruct[i].recvdTmstp < UDP_MIN_MEAS)
            return true;
    }
    return false;
}


uint32 udp_getTimeCorrection( void ){
    uint32 avg = 0;
    uint8 num = 0;

    for (uint8 i=0;i<WIFI_AP_MAX_CONN;i++){
        if (nodesStruct[i].active == true){
            if (nodesStruct[i].recvdTmstp == 0)
                continue;
            avg += nodesStruct[i].delayAcc/nodesStruct[i].recvdTmstp;
            nodesStruct[i].delayAcc = 0;
            nodesStruct[i].recvdTmstp = 0;
            num++;
        }
    }
    if (num == 0)
        return 0;
    // Take half of mean average delay
    avg /= 2*num;
    return avg;
}

/*
uint32 udp_getTimeCorrection( void ){
    uint32 avg = 0;
    uint8 num = 0;

    for (uint8 i=0;i<WIFI_AP_MAX_CONN;i++){
        if (nodesStruct[i].active == true){
            avg += nodesStruct[i].delayAcc;
            nodesStruct[i].delayAcc = 0;
            num++;
        }
    }
    // Take half of mean average delay
    if (num == 0)
        return 0;
    
    avg /= (2*num);
    return avg;
}
*/
void udpClient(void*arg)
{
    // Broadcast espconn
    static esp_udp udp;
    udp_bc.type = ESPCONN_UDP;
    udp_bc.proto.udp = &udp;
    udp.remote_port = UDP_RECV_PORT;
    udp.local_port = UDP_SEND_PORT;
    //espconn_regist_recvcb(&udp_bc, UdpRecvCb);
   
    int8 res = 0;
    res = espconn_create(&udp_bc);
    if (res != 0) {
        DBG_PRINT("UDP CLIENT CREAT ERR ret:%d\n", res);
    }

    // Receiving espconn
    configStruct.udp_listener.type = ESPCONN_UDP;
    configStruct.udp_listener.proto.udp = &configStruct.udp;
    //udp.remote_port = UDP_RECV_PORT;
    configStruct.udp.local_port = UDP_RECV_PORT;
    espconn_regist_recvcb(&configStruct.udp_listener, UdpRecvCb);
   
    res = espconn_create(&configStruct.udp_listener);
    if (res != 0) {
        DBG_PRINT("UDP CLIENT CREAT ERR ret:%d\n", res);
    }

    
    uint8 IP[4];
    uint8 id;

    while(1){
        // Init
        memset(nodesStruct, 0, sizeof(nodesStruct));
        // memset(&configStruct, 0, sizeof(configStruct));
        configStruct.bcActive = false;
        configStruct.id = 0;

        // While not connected, spam
        while ( wifi_isConnected()!=true )
        {
            DBG_PRINT("UDP: Not connected\n");
            vTaskDelay(DELAY_MS(1000));
        }
        // When connected, setup broadcast address
        if ( wifi_getGwIP(IP) != 0 )
            continue;
        IP[3] = 255;
        memcpy(udp_bc.proto.udp->remote_ip, IP, 4);

        // Setup data structure
        if ( wifi_getMyIP(IP) != 0 )
            continue;
        configStruct.id = IP[3];
        configStruct.bcActive = true;

        // While connected, do normal ops
        while ( wifi_isConnected()==true )
        {
            /*
            if ( udp_isCorrectable()==true )
            {
                DBG_PRINT("Correcting own time.\n");
                period = clock_getDefaultPeriod();
                corr = udp_getTimeCorrection();
                DBG_PRINT("delta: %d, T/2: %d\n", corr, period/2);
                if (corr < period/4)
                    while (clock_modifyNextPeriod((int32)corr))
                        vTaskDelay(DELAY_MS(50));
                else
                    while (clock_modifyNextPeriod((int32)-(period/2-corr)))
                        vTaskDelay(DELAY_MS(50));
            }
            */
            vTaskDelay(DELAY_MS(UDP_TASK_PERIOD));
        }
    }
    vTaskDelete(NULL);
}
