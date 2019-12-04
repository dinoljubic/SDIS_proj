#ifndef BOARDIO_H
#define BOARDIO_H

#include "esp_common.h"
#include "config.h"
#include "gpio.h"

#define LED_IO_PIN      GPIO_Pin_2

void boardIO_init( void );
void boardIO_task(void* param);

#endif /* BOARDIO_H */