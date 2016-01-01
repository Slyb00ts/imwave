//
//    FILE: imtime.h
// VERSION: 0.1.00
//https://github.com/n0m1/Sleep_n0m1/blob/master/Sleep_n0m1.h


//

// HISTORY:
//
#ifndef imTimer_h
#define imTimer_h
#include <avr/interrupt.h>
#include <LowPower.h>


#include <Arduino.h>
#include "imdebug.h"


typedef void( * funIMTimer )( byte );



/****************** Events *******************************/


#define PRINTRADIO 1
#define STARTBROADCAST 2
#define STOPBROADCAST 3
#define STARTDATA 4
#define STOPDATA 5
#define LISTENBROADCAST 101
#define LISTENDATA 102
#define CRONHOUR 201
#define CRONDAY 202




extern "C" void PCINT0_vect(void)__attribute__ ((signal)); // handle pin change interrupt for D8 to D13 here

class  IMTimer
{
  private:
    static IMTimer* ptrr; //static ptr to Sleep class for the ISR
    long waiting;
    long cycle;
    byte watchdog;
    unsigned long start;
    byte current;
    volatile byte _listen;
    unsigned short nearStage;
    unsigned long nearTime;
    static const int maxStages = 8;
    unsigned long stages[maxStages];
    void compute();
    unsigned long getTime();
    unsigned long getTime(unsigned long time);
    void sleep(unsigned long time);
    uint16_t CycleHour();


  public:
        IMTimer();
        funIMTimer onStage;
        funIMTimer onListen;
        uint16_t DeviationPlus;                       //sum deviation in calibrate
        uint16_t DeviationMinus;                     //separate for shorter and longer
        static const byte IDDLESTAGE = 100;
        static const byte PERIOD = 0;
        static const byte LAP = 8;


	friend void PCINT0_vect(void);

        static short ClassTest();
        void Setup(byte stage, unsigned long waittime);
        byte WaitStage();
        void setStage(byte stage);
        void doneListen();
        void doneWrite();
        void ResetDeviation();
        void Watchdog();
        bool Watchdog(byte dog);
        long Cycle();
        void Calibrate(unsigned long time);


  private:

} ;


#endif
