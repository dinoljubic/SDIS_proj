#ifndef CLOCK_H
#define CLOCK_H

#include "config.h"

void clock_init( uint32 start );
uint32 clock_get( void );
void clock_set( uint32 newClock );
void clock_InterruptHandler( void );

#endif /* CLOCK_H */