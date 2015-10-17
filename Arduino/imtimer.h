
//
//    FILE: imtime.h
// VERSION: 0.1.00
// PURPOSE: Low power for Arduino
// based on lock free queue
//https://github.com/n0m1/Sleep_n0m1/blob/master/Sleep_n0m1.h


//

// HISTORY:
//
#ifndef imTimer_h
#define imTimer_h
#include <avr/interrupt.h>


#include <Arduino.h>

typedef void( * funIMTimer )( byte );



extern "C" void PCINT0_vect(void)__attribute__ ((signal)); // handle pin change interrupt for D8 to D13 here

class  IMTimer
{
  private:
    static IMTimer* ptrr; //static ptr to Sleep class for the ISR
    long waiting;
    long cycle;
    unsigned long nearStage;
    static const int maxStages = 8;
    unsigned long stages[maxStages];
    unsigned long delays[maxStages];
    byte ruptures[maxStages];
    void compute();
    void Next(byte stage);
    byte Find(unsigned long _next);
    void doCycle();


  public:
        IMTimer();
        funIMTimer onStage;
        byte current;
        static const byte IDDLESTAGE = 0;
        static const byte CYCLELOOP = 1;


	friend void PCINT0_vect(void);
//	friend void sleepHandler(void);

        static short ClassTest();
        static volatile byte State;
        void Next(byte stage, unsigned long waittime);
        bool Arrived(byte stage);
        void doRupture(byte state);
        byte WaitStage();
        void Wait();
        void setStage(byte stage);
        long Cycle();

  private:
//    static IMTimer* pTimer; //static ptr to Sleep class for the ISR

} ;


#endif
