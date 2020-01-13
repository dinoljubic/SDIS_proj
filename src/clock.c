#include "clock.h"
#include "hw_timer.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "boardIO.h"
#include "udp_client.h"


static struct {
    xTaskHandle clockTaskHandle;
    xSemaphoreHandle clockQueueMutexHandle;

    uint32 nextPeriod;
    uint32 defaultPeriod;
    uint32 currentPeriod;
    uint32 lastLoad;

    bool initialized;

} clockStruct;


uint8 clock_init( void ){
    if (clockStruct.initialized == true){
        ERR_PRINT("Second initialization of clock.");
        return 1;
    }
    // Initialize hardware clock
    hw_timer_init();

    clockStruct.defaultPeriod = CLK_PERIOD_MS/CLK_TICK_PERIOD_US*1000;
    clockStruct.nextPeriod = clockStruct.defaultPeriod;
    clockStruct.currentPeriod = clockStruct.nextPeriod;
    clockStruct.lastLoad = clockStruct.nextPeriod;
    
    clockStruct.initialized = true;

    // Run the clock
    hw_timer_set_func(&clock_InterruptHandler);
    hw_timer_arm(CLK_TICK_PERIOD_US, 1);

    return 0;
}

uint32 clock_getTicks( void ){
    return clockStruct.currentPeriod;
}

uint32 clock_getDefaultPeriod( void ){
    return clockStruct.defaultPeriod;
}

uint32 clock_getLastLoad( void ){
    return clockStruct.lastLoad;
}

void clock_correct( void ){
    uint32 corr, period;

    period = clock_getLastLoad();
    corr = udp_getTimeCorrection();
    DBG_PRINT("1\n");
    if (corr < period/4)
        clock_modifyNextPeriod((int32)corr);
    else
        clock_modifyNextPeriod((int32)-(period/2-corr));
}

/**
 * Sets the duration of next period loaded. Value in MS
 */
uint8 clock_setNextPeriod( uint32 period ){
    // Maybe don't load new value if close to the end, 2% is arbitrary
    // if (clockStruct.currentPeriod < clockStruct.defaultPeriod*2/100)
    //     return 1;
    
    clockStruct.nextPeriod = period;
    return 0;
}

uint8 clock_modifyNextPeriod( int32 delta ){
    DBG_PRINT("Correcting period by %d to %d", delta, clockStruct.defaultPeriod+delta);
    return clock_setNextPeriod( \
        (uint32)((int32)clockStruct.defaultPeriod+delta));
}


void clock_InterruptHandler( void ){

    // Handle events here
    clockStruct.currentPeriod--;

    if (clockStruct.currentPeriod == 0){
        // Get new period
        
        boardIO_toggleLED( 0 );
        udp_broadcast();
        clock_correct();
        
        clockStruct.currentPeriod = clockStruct.nextPeriod;
        clockStruct.lastLoad = clockStruct.nextPeriod;
        clockStruct.nextPeriod = clockStruct.defaultPeriod;
    }

    // hw_timer_arm(CLK_TICK_PERIOD_US, 0);
}
