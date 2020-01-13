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


// Station struct
struct {
    uint8       gwIP[4];
    uint8       sbMsk[4];
    uint8       myIP[4];
    bool        connected;
} StaStruct;


void wifi_init_task( void *param ){
    
    // Clear structures
    memset(&StaStruct, 0, sizeof(StaStruct));

    conn_ap_init();
    
    // Setup a monitor task?
    // Or handle evthg through events
    
    vTaskDelete(NULL);
}

bool wifi_isConnected( void ){
    return StaStruct.connected;
}

uint8 wifi_getGwIP( uint8 *ip ){
    if (StaStruct.connected == false)
        return 1;
    else{
        memcpy(ip, StaStruct.gwIP, 4);
        return 0;
    }
}

uint8 wifi_getNetMsk( uint8 *msk ){
    if (StaStruct.connected == false)
        return 1;
    else{
        memcpy(msk, StaStruct.sbMsk, 4);
        return 0;
    }
}

uint8 wifi_getMyIP( uint8 *ip ){
    if (StaStruct.connected == false)
        return 1;
    else{
        memcpy(ip, StaStruct.myIP, 4);
        return 0;
    }
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
            StaStruct.connected = false;
            DBG_PRINT("disconnect from ssid %s, reason %d\n", evt->event_info.disconnected.ssid,
                    evt->event_info.disconnected.reason);
            break;
        case EVENT_STAMODE_AUTHMODE_CHANGE:
            DBG_PRINT("mode: %d -> %d\n", evt->event_info.auth_change.old_mode, evt->event_info.auth_change.new_mode);
            break;
        case EVENT_STAMODE_GOT_IP:
            // Client-only; store own IP and server's (GW)
            memcpy(&StaStruct.gwIP, &evt->event_info.got_ip.gw, 4);
            memcpy(&StaStruct.myIP, &evt->event_info.got_ip.ip, 4);
            memcpy(&StaStruct.sbMsk, &evt->event_info.got_ip.mask, 4);
            StaStruct.connected = true;
            // Introduce to server
            DBG_PRINT("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR, IP2STR(&evt->event_info.got_ip.ip),
                    IP2STR(&evt->event_info.got_ip.mask), IP2STR(&evt->event_info.got_ip.gw));
            DBG_PRINT("\n");
            break;
        case EVENT_SOFTAPMODE_STACONNECTED:
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
    wifi_set_opmode(STATION_MODE);
    struct station_config config;
    memset(&config, 0, sizeof(config));  //set value of config from address of &config to width of size to be value '0'
    sprintf(config.ssid, DEMO_AP_SSID);
    sprintf(config.password, DEMO_AP_PASSWORD);
    wifi_station_set_config(&config);
    wifi_set_event_handler_cb(wifi_handle_event_cb);
    wifi_station_connect();
}


void soft_ap_disable(void)
{
    printf("Disabling AP");
    wifi_set_opmode(STATION_MODE);
}
