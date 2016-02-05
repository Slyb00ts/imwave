/*
 Copyright (C) 2015 Dariusz Mazur <darekm@emadar.com>

 */
 
/**
 * @file imAtmega.h
 *
 * Setup necessary to imwave library
 */

#ifndef __imATMEGA_H__
#define __imATMEGA_H__
#include "Arduino.h"
#include <avr/wdt.h>


// http://code.google.com/p/tinkerit/wiki/SecretVoltmeter
long internalVccOld();


//http://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
uint16_t internalVcc();
uint16_t internalTemp();



uint16_t internalrandom();

void reboot();

#define soft_restart()        \
do                          \
{                           \
    wdt_enable(WDTO_15MS);  \
    for(;;)                 \
    {                       \
    }                       \
} while(0)

int freeRam ();

#endif // __imATMEGA_H__
