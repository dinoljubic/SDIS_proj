#include "boardIO.h"
#include "freertos/queue.h"

#define boardIO_queue_size 10

static uint8 ledState = 0;

void boardIO_init( void ){
    
    GPIO_ConfigTypeDef io_out_conf;
    io_out_conf.GPIO_IntrType = GPIO_PIN_INTR_DISABLE;
    io_out_conf.GPIO_Mode = GPIO_Mode_Output;
    io_out_conf.GPIO_Pin = LED_IO_PIN;
    io_out_conf.GPIO_Pullup = GPIO_PullUp_DIS;
    gpio_config(&io_out_conf);
    GPIO_OUTPUT(LED_IO_PIN, ledState = 0);

    xTaskCreate(&boardIO_task, "IO task", boardIO_mem, NULL, boardIO_prior, NULL);
}

void boardIO_setLED( uint32 value ){
    GPIO_OUTPUT(LED_IO_PIN, ledState = (uint8)value);
}

void boardIO_toggleLED( uint32 param ){
    GPIO_OUTPUT(LED_IO_PIN, ledState);
    ledState = (ledState == 0);
    DBG_PRINT("led state: %d", ledState);
}

void boardIO_task(void* param)
{
    xQueueHandle ledQueueHandle;
    ledQueueHandle = xQueueCreate(boardIO_queue_size, sizeof(uint));
    if (ledQueueHandle == NULL)
    {
        // error handler
    }
    
    while(1){
        /*
        GPIO_OUTPUT(LED_IO_PIN, 0);
        vTaskDelay(1000/portTICK_RATE_MS);
        GPIO_OUTPUT(LED_IO_PIN, 1);
        */
        vTaskDelay(1000/portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}