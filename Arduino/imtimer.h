//
//    FILE: imtime.h
// VERSION: 0.1.00
//https://github.com/n0m1/Sleep_n0m1/blob/master/Sleep_n0m1.h


//

// HISTORY:
//
#ifndef imTimer_h
#define imTimer_h
#include "imatmega.h"
#include <avr/interrupt.h>
#include <avr/sleep.h>


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
#define REBOOTLAP 250
#define CycleDuration 3000



extern "C" void PCINT0_vect(void)__attribute__ ((signal)); // handle pin change interrupt for D8 to D13 here

class  IMTimer
{
  private:
    static IMTimer* ptrr; //static ptr to Sleep class for the ISR
    //static long counterTimer2; //static ptr to Sleep class for the ISR
    t_Time waiting;
    long cycle;
    uint16_t watchdog;
    t_Time start;
    byte current;
    volatile byte _listen;
    unsigned short nearStage;
    t_Time nearTime;
    static const int maxStages = 8;
    t_Time stages[maxStages];
    void compute();
    t_Time getTime();
//    t_Time getTime(t_Time time);
    void sleep(unsigned long time);


  public:
        IMTimer();
//        funIMTimer onStage;
//        funIMTimer onListen;
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
        static void doneReceived(byte count);
        void doneListen();
        void doneWrite();
        void ResetDeviation();
        void Watchdog();
        bool Watchdog(uint16_t dog);
        long Cycle();
        void Calibrate(unsigned long time);
        void printTime();
        uint16_t CycleHour();


  private:

} ;


#endif
