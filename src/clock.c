#include "clock.h"
#include "hw_timer.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"


static struct {
    xTaskHandle clockTaskHandle;
    xSemaphoreHandle clockQueueMutexHandle;

    uint32 tickPeriod_us;
    uint32 ticks;

    bool initialized;

    // Events
    clockEvent_t events[CLOCK_EVT_QUEUE_SIZE];
    clockEvent_t *nextEvt;
    uint32 delayToNext;
    uint8 evtNum;

} clockStruct;


static uint8 _alarm_setTask(uint32 timeMS, void(*fun)(uint32), uint32 param);
static void alarm_getNext( void );


uint8 clock_init( uint32 start, uint32 gran_US ){
    if (clockStruct.initialized == true){
        ERR_PRINT("Second initialization of clock.");
        return 1;
    }
    // Initialize hardware clock
    hw_timer_init();
    clockStruct.ticks = start;
    clockStruct.tickPeriod_us = gran_US;
    
    // Initialize clock event queue
    clockStruct.clockQueueMutexHandle = xSemaphoreCreateMutex();
    clockStruct.evtNum = 0;
    clockStruct.delayToNext = 0;
    clockStruct.nextEvt = NULL;
    memset(clockStruct.events, 0, sizeof(clockEvent_t)*CLOCK_EVT_QUEUE_SIZE);
    
    clockStruct.initialized = true;

    DBG_PRINT("Starting clock..\n");
    // Run the clock
    hw_timer_set_func(&clock_InterruptHandler);
    hw_timer_arm(clockStruct.tickPeriod_us, 1);

    return 0;
}

static void alarm_getNext( void ){
    uint32 minTime = UINT32_MAX;
    uint32 time = clock_getMS();
    uint8 minIndex;

    for (uint8 i=0;i<CLOCK_EVT_QUEUE_SIZE;i++){
        if (clockStruct.events[i].time < minTime && clockStruct.events[i].func != NULL){
            minTime = clockStruct.events[i].time;
            minIndex = i;
        }
        // Clear missed events
        if (clockStruct.events[i].time < time){
            clockStruct.events[i].time = 0;
            clockStruct.events[i].func = NULL;
        }
    }
    clockStruct.nextEvt = &clockStruct.events[minIndex];
    clockStruct.delayToNext =  minTime - time;
}

static uint8 _alarm_setTask(uint32 timeMS, void(*fun)(uint32), uint32 param){
    uint32 time = clock_getMS();

    if (timeMS < time)
        return 1;

    //DBG_PRINT("Add evt. at time %d\n", timeMS);

    // check if queue is full
    if (clockStruct.evtNum == CLOCK_EVT_QUEUE_SIZE)
        return 1;

    // Iterate and find a free slot
    uint8 i=0;
    while(clockStruct.events[i].time != 0)i++;
    
    clockStruct.events[i].time = timeMS;
    clockStruct.events[i].func = fun;
    clockStruct.events[i].param = param;
    
    if (clockStruct.evtNum == 0){
        //time = clock_getMS();
        clockStruct.delayToNext = timeMS-time;
        clockStruct.nextEvt = &clockStruct.events[i];
    }
    clockStruct.evtNum++;
    return 0;
}

// Mutex wrapper for _alarm_setTask
uint8 alarm_setTask( uint32 timeMS, void(*fun)(uint32), uint32 param ){
    uint8 retVal;
    if (xSemaphoreTake(clockStruct.clockQueueMutexHandle, (portTickType) 100) != pdTRUE)
        return 2;
    retVal = _alarm_setTask( timeMS, fun, param );
    xSemaphoreGive(clockStruct.clockQueueMutexHandle);
    return retVal;
}


uint32 clock_getTicks( void ){
    return clockStruct.ticks;
}

void clock_setTicks( uint32 newTicks ){
    clockStruct.ticks = newTicks;
}

uint32 clock_getMS( void ){
    return clockStruct.ticks*clockStruct.tickPeriod_us/1000;
}

void clock_setMS( uint32 time ){
    clockStruct.ticks = 1000*time/clockStruct.tickPeriod_us;
}

void clock_InterruptHandler( void ){

    // Increment tick count
    clockStruct.ticks++;

    if (clockStruct.nextEvt != NULL){
        clockStruct.delayToNext--;

        if (clockStruct.delayToNext == 0){
            if (clockStruct.nextEvt->func != NULL){
                clockStruct.nextEvt->func(clockStruct.nextEvt->param);
                // Clear
                clockStruct.nextEvt->func = NULL;
                clockStruct.nextEvt->time = 0;
                // Get next
                if (--clockStruct.evtNum == 0)
                    clockStruct.nextEvt = NULL;
                else
                    alarm_getNext();
            }
        }
    }
}
