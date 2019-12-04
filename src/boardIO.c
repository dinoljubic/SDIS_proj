#include "boardIO.h"

void boardIO_init( void ){
    xTaskCreate(&boardIO_task, "IO task", boardIO_mem, NULL, boardIO_prior, NULL);
}

void boardIO_task(void* param)
{
    GPIO_ConfigTypeDef io_out_conf;
    io_out_conf.GPIO_IntrType = GPIO_PIN_INTR_DISABLE;
    io_out_conf.GPIO_Mode = GPIO_Mode_Output;
    io_out_conf.GPIO_Pin = LED_IO_PIN;
    io_out_conf.GPIO_Pullup = GPIO_PullUp_DIS;
    gpio_config(&io_out_conf);

    while(1){
        GPIO_OUTPUT(LED_IO_PIN, 0);
        vTaskDelay(1000/portTICK_RATE_MS);
        GPIO_OUTPUT(LED_IO_PIN, 1);
        vTaskDelay(1000/portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}