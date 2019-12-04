#include "clock.h"
#include "hw_timer.h"

static uint32 clock;
//static bool running;

void clock_init( uint32 start ){
    hw_timer_init();
    clock = start;
    hw_timer_set_func(&clock_InterruptHandler);
    //running = true;
    hw_timer_arm(CLOCK_TIMER_PERIOD_US, 1);
}

uint32 clock_get( void ){
    return clock;
}

void clock_set( uint32 newClock ){
    clock = newClock;
}

void clock_InterruptHandler( void ){
    clock++;
}
