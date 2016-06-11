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
#include <avr/power.h>


// http://code.google.com/p/tinkerit/wiki/SecretVoltmeter
long internalVccOld();
long millisT2();
void setupTimer2();
long incTimer2();

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

/*
  set_sleep_mode(<mode>);
  cli();
  if (some_condition)
  {
    sleep_enable();
    sleep_bod_disable();
    sei();
    sleep_cpu();
    sleep_disable();
  }
  sei();
*/
void delaySleep(unsigned long t);
void delaySleepT2(unsigned long t);
void reboot();
void disableADCB();
#define  counterTCNT2  131;

void enterSleep(void);



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