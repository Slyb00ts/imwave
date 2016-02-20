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
#include <avr/sleep.h>
#include <avr/interrupt.h>


// http://code.google.com/p/tinkerit/wiki/SecretVoltmeter
long internalVccOld();


//http://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
uint16_t internalVcc();
uint16_t internalTemp();
void pciSetup(uint8_t pin);




uint16_t internalrandom();

#define  goSleep()                     \
do {                                   \
    sleep_enable();                    \
    sei();                             \
    sleep_cpu();                       \
    sleep_disable();                   \
    sei();                             \
}  while (0)


#define  goSleepA()                     \
do {                                   \
//  set_sleep_mode (SLEEP_MODE_IDLE);  \
//  cli()                              \
//  if (some_condition){               \
    sleep_enable();                    \
    //sleep_bod_disable();             \
    sei();                             \
    sleep_cpu();                       \
    sleep_disable();                   \
    sei();                             \
// } sei();                            \
}  while (0)

void delaySleep(unsigned long t);
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
