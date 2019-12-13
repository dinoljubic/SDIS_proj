#ifndef CLOCK_H
#define CLOCK_H

#include "config.h"

typedef struct {
    uint32 time;
    void (*func)(uint32);
    uint32 param;
} clockEvent_t;

uint8 alarm_setTask( uint32 timeMS, void(*fun)(uint32), uint32 param );

uint8 clock_init( uint32 start, uint32 gran_US );
uint32 clock_getTicks( void );
void clock_setTicks( uint32 newTicks );
uint32 clock_getMS( void );
void clock_setMS( uint32 time );

void clock_InterruptHandler( void );

#endif /* CLOCK_H */