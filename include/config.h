#ifndef CONFIG_H
#define CONFIG_H

/**
 * Global config file.
 * Contains common includes for operations with FreeRTOS and definitions for various
 * parameters of the system in order to be easily accessible and modifiable.
 */

#include "esp_common.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"


// Task parameters
#define boardIO_mem             100
#define boardIO_prior           3
#define serial_test_task_mem    100
#define serial_test_task_prior  4

// Various parameters
// Clock
#define CLOCK_HWTIMER_FREQ_DIV  16
#define CLOCK_TIMER_PERIOD_US   1000
#define CLOCK_HWTIMER_PERIOD    CPU_CLK_FREQ/CLOCK_HWTIMER_FREQ_DIV*CLOCK_TIMER_PERIOD_US


#endif /* CONFIG_H */