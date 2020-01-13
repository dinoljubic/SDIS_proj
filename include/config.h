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

#define DEBUG   1

#define WIFI_AP_MAX_CONN    5
#define DEMO_AP_SSID        "laptop"
#define DEMO_AP_PASSWORD    "pasvordddd"

#define UDP_SEND_PORT   1024
#define UDP_RECV_PORT   4444

// Clock testing parameters
#define CKT_SHO     60
#define CKT_LON     180
#define CKT_SYS     60
#define CKT_STRLEN  15

// Task parameters
#define boardIO_mem             100
#define boardIO_prior           3
#define serial_test_task_mem    100
#define serial_test_task_prior  4
#define UDP_client_task_mem     200
#define UDP_client_task_prior   5

#define UDP_TASK_PERIOD         1000


// Various parameters
// Clock
#define CLK_TICK_PERIOD_US      1000
#define CLK_PERIOD_MS           1000
#define CLK_UPDATE_PERC         5

#define CLOCK_EVT_QUEUE_SIZE    40
#define CLOCK_HWTIMER_FREQ_DIV  16
#define CLOCK_TIMER_PERIOD_US   1000
#define CLOCK_HWTIMER_PERIOD    CPU_CLK_FREQ/CLOCK_HWTIMER_FREQ_DIV*CLOCK_TIMER_PERIOD_US

#define DELAY_MS(t)             (t)/portTICK_RATE_MS

// Sync
#define UDP_MIN_MEAS            3

// Debug macros
#if DEBUG
#define DBG_PRINT(fmt,...)	do{\
	    os_printf("[Dbg]");\
	    os_printf(fmt,##__VA_ARGS__);\
	}while(0)

#define ERR_PRINT(fmt,...) do{\
	    os_printf("[Err] Fun:%s Line:%d ",__FUNCTION__,__LINE__);\
	    os_printf(fmt,##__VA_ARGS__);\
	}while(0)
#define DBG_LINES(v) os_printf("------------------%s---------------\n",v)
#else
#define DBG_PRINT(fmt,...)
#define ERR_PRINT(fmt,...)
#endif


#endif /* CONFIG_H */