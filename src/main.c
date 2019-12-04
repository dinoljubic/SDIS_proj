#include "config.h"

//#include "conn_ap.h"

#include "boardIO.h"
#include "serial.h"
#include "clock.h"

uint32 user_rf_cal_sector_set(void);

/*
void wifi_test_task (void *param)
{
    //wifi_set_opmode_current();
    conn_ap_init();
    vTaskDelete(NULL);
}
*/

void user_init(void)
{

    boardIO_init();
    serial_init(BIT_RATE_115200);
    clock_init(0);

    //xTaskCreate(&wifi_test_task, "wifi", 2048, NULL, 1, NULL);

    // Start the scheduler
    // vTaskStartScheduler();

    while(1){
        vTaskDelay(100);
        // Nothing to do
    }
}

