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


#include <Arduino.h>
#include "imdebug.h"


typedef void( * funIMTimer )( byte );



extern "C" void PCINT0_vect(void)__attribute__ ((signal)); // handle pin change interrupt for D8 to D13 here

class  IMTimer
{
  private:
    static IMTimer* ptrr; //static ptr to Sleep class for the ISR
    long waiting;
    long cycle;
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


  public:
        IMTimer();
        funIMTimer onStage;
        funIMTimer onListen;
        static const byte IDDLESTAGE = 100;
        static const byte PERIOD = 0;


	friend void PCINT0_vect(void);

        static short ClassTest();
        void Setup(byte stage, unsigned long waittime);
        byte WaitStage();
        void setStage(byte stage);
        void doneListen();
        void doneWrite();
        long Cycle();
        void Calibrate();


  private:

} ;


#endif
