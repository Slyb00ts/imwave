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
//#include <util/delay.h>


typedef unsigned long t_Time;
//typedef uint16_t t_Time;
// http://code.google.com/p/tinkerit/wiki/SecretVoltmeter
long internalVccOld();
t_Time millisT2();
t_Time millisTNow();
void setupTimer2();
void setSleepModeT2();

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
    sleep_enable();                    \
    sei();                             \
    sleep_cpu();                       \
    sleep_disable();                   \
    sei();                             \
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
void delayT2();
void setupTimer2();
void reboot();
void resetPin();
unsigned int sqrt32(unsigned long n);

#if defined(__AVR_ATmega328P__)
#define  __sleepT2  1
#define  CRYSTAL32K
void disableADCB();
void ShutOffADC(void);
void ShutDownADC(void);
void  SetupADC(void);
void waitASSR(void);
#endif

void syncTimer2(int8_t step);

#ifdef CRYSTAL32K
byte nextTimer2(void);
byte addTimer2(byte aTime);
void  stopTimer2(t_Time aNext);
void setMaxStepTimer(byte aStep);
#else
 t_Time incTimer2(void);
 void  stopTimer2(t_Time aNext);

#endif

#define  counterTCNT2  131;

void enterSleep(void);
//void  SoftResetFunc (void); //declare reset function @ address 0
void SoftReset();

//void(* SoftResetFunc) (void)=0; //declare reset function @ address 0


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
