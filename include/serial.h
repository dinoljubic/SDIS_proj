#ifndef SERIAL_H
#define SERIAL_H

#include "config.h"
#include "uart.h"

void serial_init( UART_BautRate baud );
void serial_test_task( void *param );

#endif /* SERIAL_H */
