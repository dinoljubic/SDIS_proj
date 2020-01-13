#ifndef CLOCK_H
#define CLOCK_H

#include "config.h"

typedef struct {
    uint32 time;
    void (*func)(uint32);
    uint32 param;
} clockEvent_t;

uint8 clock_init( void );
uint32 clock_getTicks( void );
uint32 clock_getDefaultPeriod( void );
uint8 clock_setNextPeriod( uint32 period );
uint8 clock_modifyNextPeriod( int32 delta );
void clock_InterruptHandler(void);

#endif /* CLOCK_H */