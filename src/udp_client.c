#include "config.h"
#include "udp_client.h"
#include "espconn.h"
#include "clock.h"
#include "boardIO.h"
#include "freertos/queue.h"

const uint8 udp_server_ip[4] = { 192, 168, 47, 1 };
const uint8 udp_client_ip[4] = { 192, 168, 47, 255 };

os_timer_t time1;
static struct espconn udp_client;

void StaConectApConfig(char*ssid, char*password)
{
    if (wifi_get_opmode() != 0x01 && wifi_get_opmode() != 0x03) {
        ERR_PRINT("Mode not include sta\n");
        wifi_set_opmode(0x03);
        //return;
    }
    struct station_config* sta = (struct station_config*) malloc(sizeof(struct station_config));
    if (sta == NULL) {
        ERR_PRINT("memory not enough,heap_size=%ukByte\n", system_get_free_heap_size() / 1023);
        return;
    }
    bzero(sta, sizeof(struct station_config));
    wifi_station_get_config(sta);
    sprintf(sta->ssid, "%s", ssid);
    sprintf(sta->password, "%s", password);
    wifi_station_set_config(sta);
    wifi_set_event_handler_cb(wifi_handle_event_cb);
    wifi_station_connect();

    bzero(sta, sizeof(struct station_config));
    wifi_station_get_config(sta);
    DBG_LINES("STA_CONNECT_AP");
    DBG_PRINT("ssid:%s\n", sta->ssid);
    DBG_PRINT("password:%d\n", sta->password);
    free(sta);
    sta = NULL;
}

uint8 curpktID = 0;

void UdpRecvCb(void *arg, char *pdata, unsigned short len)
{
    struct espconn* udp_server_local = arg;
    uint32 time;
    uint8 value;
    uint8 data[8];
    uint32 clock = clock_getMS(), tmp1, tmp2;
    // Parser for received packets
    
    switch(*pdata){
        case 0x01:  // Disable AP
            os_printf("Packet: %c", pdata[1]);
            if (pdata[1] == 0x00)
                soft_ap_disable();
            break;
        case 0x02:  // Set event at time
            memcpy((char*)&time, &pdata[1], sizeof(uint32));
            value = pdata[5];
            // set alarm for LED
            break;
        case 0x03:  // Set clock time (ms)
            memcpy((char*)&time, &pdata[1], sizeof(uint32));
            clock_setMS(time);
            break;
        case 0x04:  // Toggle LED
            boardIO_toggleLED();
            break;
        case 0x05:  // Got round-trip msg
            memcpy(&tmp1, &pdata[1], 4);
            memcpy(&tmp2, &pdata[5], 4);
            // own tmpstamp is in tmp1, client tmpstamp is in tmp2
            tmp1 = tmp1-tmp2+(clock-tmp1)/2; // store client correction to tmp1
            //xQueueCRSend(/*queue goes here*/);
           
            // push to queue
            break;
        case 0x06:  // Respond (round-trip LED toggle)
            data[0] = 0x05;
            memcpy(&data[1], &pdata[1], 4);
            memcpy(&data[5], &clock, 4);
            espconn_send(&udp_client, data, 9);
            break;
        default:
            break;
    }

    DBG_LINES("UDP_RECV_CB");
    DBG_PRINT("UDP_RECV_CB len:%d ip:%d.%d.%d.%d port:%d\n", len, udp_server_local->proto.tcp->remote_ip[0],
            udp_server_local->proto.tcp->remote_ip[1], udp_server_local->proto.tcp->remote_ip[2],
            udp_server_local->proto.tcp->remote_ip[3], udp_server_local->proto.tcp->remote_port\
    );
    // Echo data to server
    // espconn_send(udp_server_local, pdata, len);

}
void UdpSendCb(void* arg)
{
    struct espconn* udp_server_local = arg;
    DBG_LINES("UDP_SEND_CB");
    DBG_PRINT("UDP_SEND_CB ip:%d.%d.%d.%d port:%d\n", udp_server_local->proto.tcp->remote_ip[0],
            udp_server_local->proto.tcp->remote_ip[1], udp_server_local->proto.tcp->remote_ip[2],
            udp_server_local->proto.tcp->remote_ip[3], udp_server_local->proto.tcp->remote_port\
    );
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

    espconn_regist_recvcb(&udp_client, UdpRecvCb);
    espconn_regist_sentcb(&udp_client, UdpSendCb);
    int8 res = 0;
    res = espconn_create(&udp_client);
    if (res != 0) {
        DBG_PRINT("UDP SERVER CREAT ERR ret:%d\n", res);
    }

    os_timer_disarm(&time1);
    os_timer_setfn(&time1, t1Callback, NULL);
    // Armed to 1550 bc the beacon interval should be 100ms
    // TODO: try different %100 time intervals (e.g. 20,30...)
    os_timer_arm(&time1, 100, 1);

    vTaskDelete(NULL);
}


void t1Callback(void* arg)
{
    static uint8 value = 0;
    static uint8 data[5];
    uint32 clock = clock_getMS();

    os_printf("Second.\n");
    //espconn_send(&udp_client, UDP_SERVER_GREETING, strlen(UDP_SERVER_GREETING));
    
    // Test for round-trip correction
    /*
    data[0] = 0x06;
    memcpy(&data[1], &clock, 4);
    espconn_send(&udp_client, data, 5);
    */

   // Test for setting timed events

}

void udpClient(void*arg)
{

    static esp_udp udp;
    udp_client.type = ESPCONN_UDP;
    udp_client.proto.udp = &udp;
    udp.remote_port = UDP_SERVER_LOCAL_PORT;
    udp.local_port = UDP_CLIENT_LOCAL_PORT;

    memcpy(udp.remote_ip, udp_server_ip, sizeof(udp_server_ip));
    uint8 i = 0;
    os_printf("serve ip:\n");
    for (i = 0; i <= 3; i++) {
        os_printf("%u.", udp_server_ip[i]);
    }
    os_printf("\n remote ip\n");
    for (i = 0; i <= 3; i++) {
        os_printf("%u.", udp.remote_ip[i]);
    }
    os_printf("\n");
    espconn_regist_recvcb(&udp_client, UdpRecvCb);
    espconn_regist_sentcb(&udp_client, UdpSendCb);
    int8 res = 0;
    res = espconn_create(&udp_client);

    if (res != 0) {
        DBG_PRINT("UDP CLIENT CREAT ERR ret:%d\n", res);
    }

    vTaskDelete(NULL);
}

/*
void udp_task(void* param){

    WiFiUDP Udp;

    int ss_time_from_server = 0;
    int mm_time_from_server = 0;
    int hh_time_from_server = 0;

    unsigned long external_time = 0;
    unsigned long internal_time = 0;

    int interval=1000;
    unsigned long previousMillis=0;
    unsigned int previousTimeFromUDP =0;

    bool sync = true;
    
    while(1){
        unsigned long currentMillis = millis();
        int packetSize = Udp.parsePacket();
        if (packetSize)
        {
            printf("-------------------------------------------");
            // receive incoming UDP packets
            // printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
            int len = Udp.read(incomingPacket, 255);
            if (len > 0)incomingPacket[len] = 0;
            
            printf("UDP packet contents at %ld: %s\n", currentMillis,incomingPacket);
            sscanf(incomingPacket,"%d:%d:%d -- %s",&hh_time_from_server,&mm_time_from_server,&ss_time_from_server,message);
            // send back a reply, to the IP address and port we got the packet from
            Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
            
            printf("Time from UDP %d:%d:%d \n", hh_time_from_server,mm_time_from_server,ss_time_from_server);
            sprintf(replyPacket,"int_time = %f ext_time = %f",(float)internal_time/1000,(float)ss_time_from_server);

            printf("currentMillis = %ld\n",currentMillis);
            printf("previousMillis = %ld\n",previousMillis);
            
            printf("diff intern = %ld\n",currentMillis-previousMillis);
            printf("diff UDP    = %ld\n",ss_time_from_server-previousTimeFromUDP);
            previousTimeFromUDP=ss_time_from_server;
            
            (void) toggleLED(message);
            previousMillis = currentMillis;
        
            
            Udp.write(replyPacket);
            Udp.endPacket();
            println("-------------------------------------------");
        }
    }
}
*/