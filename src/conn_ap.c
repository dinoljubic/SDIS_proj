/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include "config.h"
#include "espconn.h"
#include "esp_sta.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "udp_client.h"
#include "conn_ap.h"

// AP struct
struct {
    uint8       num;
    uint32      myIP;
    bool        active[WIFI_AP_MAX_CONN];
    uint32      deviceIP[WIFI_AP_MAX_CONN];
} APStruct;


// Station struct
struct {
    uint32      masterIP;
    uint32      myIP;
} StaStruct;


void wifi_init_task( void *param ){
    
    // Clear structures
    memset(&APStruct, 0, sizeof(APStruct));
    memset(&StaStruct, 0, sizeof(StaStruct));

    #if esp==1
    soft_ap_init();
    #else
    conn_ap_init();
    #endif

    vTaskDelay(1000);
    
    #if esp==1
    xTaskCreate(udpServer, "UDP server", UDP_server_task_mem, NULL, UDP_server_task_prior, NULL);
    #else
    xTaskCreate(udpClient, "UDP client", UDP_client_task_mem, NULL, UDP_client_task_prior, NULL);
    #endif
    
    vTaskDelete(NULL);
}

void wifi_APdevice_connected( void ){

}

uint8 wifi_APdevice_disconnected( uint8 *ip ){
    uint32 ip32;
    memcpy(&ip32, ip, 4);
    for (uint8 i=0;i<WIFI_AP_MAX_CONN;i++){
        if (APStruct.deviceIP[i] == ip32){
            APStruct.deviceIP[i] = 0;
            APStruct.num--;
            APStruct.active[i] = false;
            return 0;
        }
    }
    return 1;
}


void wifi_handle_event_cb(System_Event_t *evt)
{
    //printf("event %x\n", evt->event_id);
    switch (evt->event_id) {
        case EVENT_STAMODE_CONNECTED:
            DBG_PRINT("connect to ssid %s, channel %d\n", evt->event_info.connected.ssid,
                    evt->event_info.connected.channel);
            break;
        case EVENT_STAMODE_DISCONNECTED:
            DBG_PRINT("disconnect from ssid %s, reason %d\n", evt->event_info.disconnected.ssid,
                    evt->event_info.disconnected.reason);
            break;
        case EVENT_STAMODE_AUTHMODE_CHANGE:
            DBG_PRINT("mode: %d -> %d\n", evt->event_info.auth_change.old_mode, evt->event_info.auth_change.new_mode);
            break;
        case EVENT_STAMODE_GOT_IP:
            // Client-only; store own IP and server's (GW)
            memcpy(&StaStruct.masterIP, &evt->event_info.got_ip.gw, 4);
            memcpy(&StaStruct.myIP, &evt->event_info.got_ip.ip, 4);
            // Introduce to server

            DBG_PRINT("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR, IP2STR(&evt->event_info.got_ip.ip),
                    IP2STR(&evt->event_info.got_ip.mask), IP2STR(&evt->event_info.got_ip.gw));
            DBG_PRINT("\n");
            break;
        case EVENT_SOFTAPMODE_STACONNECTED:
            // 
            DBG_PRINT("station: " MACSTR "join, AID = %d\n", MAC2STR(evt->event_info.sta_connected.mac),
                    evt->event_info.sta_connected.aid);
            break;
        case EVENT_SOFTAPMODE_STADISCONNECTED:
            // Find and remove IP from the list
            DBG_PRINT("station: " MACSTR "leave, AID = %d\n", MAC2STR(evt->event_info.sta_disconnected.mac),
                    evt->event_info.sta_disconnected.aid);
            break;
        default:
            break;
    }
}

void conn_ap_init(void)
{
    wifi_set_opmode(STATIONAP_MODE);
    struct station_config config;
    memset(&config, 0, sizeof(config));  //set value of config from address of &config to width of size to be value '0'
    sprintf(config.ssid, DEMO_AP_SSID);
    sprintf(config.password, DEMO_AP_PASSWORD);
    wifi_station_set_config(&config);
    wifi_set_event_handler_cb(wifi_handle_event_cb);
    wifi_station_connect();
}

void soft_ap_init(void)
{
    wifi_set_opmode(SOFTAP_MODE);
    struct softap_config *config = (struct softap_config *) zalloc(sizeof(struct softap_config)); // initialization
    wifi_softap_get_config(config); // Get soft-AP config first.
    sprintf(config->ssid, SOFT_AP_SSID);
    sprintf(config->password, SOFT_AP_PASSWORD);
    config->authmode = AUTH_WPA_WPA2_PSK;
    config->ssid_len = 0; // or its actual SSID length
    config->max_connection = 4;
    wifi_softap_set_config(config); // Set ESP8266 soft-AP config
    free(config);
    struct station_info * station = wifi_softap_get_station_info();
    while (station) {
        printf("bssid : MACSTR, ip : IPSTR/n", MAC2STR(station->bssid), IP2STR(&station->ip));
        station = STAILQ_NEXT(station, next);
    }
    wifi_softap_free_station_info(); // Free it by calling functionss
    wifi_softap_dhcps_stop(); // disable soft-AP DHCP server
    struct ip_info info;
    IP4_ADDR(&info.ip, 192, 168, 47, 1); // set IP
    IP4_ADDR(&info.gw, 192, 168, 47, 1); // set gateway
    IP4_ADDR(&info.netmask, 255, 255, 255, 0); // set netmask
    wifi_set_ip_info(SOFTAP_IF, &info);
    struct dhcps_lease dhcp_lease;
    IP4_ADDR(&dhcp_lease.start_ip, 192, 168, 47, 100);
    IP4_ADDR(&dhcp_lease.end_ip, 192, 168, 47, 100+WIFI_AP_MAX_CONN-1);
    wifi_softap_set_dhcps_lease(&dhcp_lease);
    wifi_softap_dhcps_start(); // enable soft-AP DHCP server
}


void soft_ap_disable(void)
{
    printf("Disabling AP");
    wifi_set_opmode(STATION_MODE);
}
