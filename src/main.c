#include "config.h"
#include "conn_ap.h"
#include "udp_client.h"
#include "boardIO.h"
#include "serial.h"
#include "clock.h"

uint32 user_rf_cal_sector_set(void);

char chtest[CKT_STRLEN]="... -.. .. ...";

void clk_test(void *param){
    uint32 time, t, del;

    wifi_set_opmode(0);
    
    while(1){
        t = time = clock_getMS() + 1000;
        for (uint8 i=0;i<CKT_STRLEN;i++){
            
            switch(chtest[i]){
            case '.':
                alarm_setTask(time, boardIO_setLED, 0);
                time += CKT_SHO;
                alarm_setTask(time, boardIO_setLED, 1);
                break;
            case '-':
                alarm_setTask(time, boardIO_setLED, 0);
                time += CKT_LON;
                alarm_setTask(time, boardIO_setLED, 1);
                break;
            case ' ':
                time += 2*CKT_SYS;
                break;
            }
            time += CKT_SYS;
        }
        vTaskDelay((time-t+2000)/portTICK_RATE_MS);
    }
    
    vTaskDelete(NULL);
}

void user_init(void)
{

    boardIO_init();
    serial_init(BIT_RATE_9600);
    clock_init();

    printf("Initialized.\n");
    
    xTaskCreate(wifi_init_task, "WiFi init task", 500, NULL, 6, NULL);
    xTaskCreate(udpClient, "udpClient", 500, NULL, 5, NULL);

}

