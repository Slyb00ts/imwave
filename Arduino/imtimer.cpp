#include <imtimer.h>

#ifdef DBGCLOCK
byte toggle;
#endif

IMTimer* IMTimer::ptrr = 0;
funStepTimer IMTimer::funStep=&IMTimer::StepTimerNull;


IMTimer::IMTimer()
{
	ptrr = this;	//the ptr points to this object
        noInterrupts();
        funStep=&StepTimerNull;

  #if defined(__sleepT2)
        setupTimer2();
        void disableADCB();
  #endif
        DeviationPlus=0;
        DeviationMinus=0;
        watchdog=0;
        cycle=0;
        start=0;
        _synchronizeStart=0;
        nearTime=0;
        nearTimeTT=0;
        SynchronizeStep=0;
        SynchronizeCycle=0;
        for (byte i=0;i<maxStages;i++)
          stages[i]=0;
        Setup(IMTimer::PERIOD,CycleDuration);
}

#define syncRate 100
void IMTimer::Calibrate(t_Time time)
{
  t_Time del=start;
  t_Time  xCycle=((time-start)*syncRate)/CycleDuration;
  start=time;
  if (time> 10000)
   start=time-CycleDuration;

   del=(time-del)%CycleDuration;
   int xStep=del;
   if (xStep>300)
      xStep-=CycleDuration;
   if (del){
     DBGINFO("calib:");
     DBGINFO(del);

   }
   DeviationPlus=del;
   if (del==0)         //xStep==0
     return;
   if ((xCycle>6*syncRate)&& (xCycle<1500*syncRate)&& (xStep>-200)&&(xStep<200))
   {
       if ((xCycle>48*syncRate)&&(xStep>80))  //bypass then sync >0066
          return;
       if ((xStep==1)) {
         --SynchronizeCycle;
         return;
       }
       if ((xStep==-1)) {
         ++SynchronizeCycle;
         return;
       }
//       if ((xStep==0)){
//         return;
//       }
       _synchronizeStart=0;
      if (SynchronizeCycle>0){
         int xc=xCycle/SynchronizeCycle;
         if (SynchronizeStep>0) xStep-=xc;
         if (SynchronizeStep<0) xStep+=xc;
      }
      if (xStep>0)
      {
        SynchronizeCycle=xCycle/xStep;
        SynchronizeStep=-1;
      } else if (xStep==0){
        SynchronizeCycle=0xffff;
        SynchronizeStep=0;

      } else{
        SynchronizeCycle=xCycle/(-xStep);
        SynchronizeStep=1;
      }
   }



   if (current!=LISTENBROADCAST){
     DBGINFO("BROADCAST");
     DBGINFO(current);
   }
}

void IMTimer::ResetDeviation()
{
  DeviationPlus=0;
  DeviationMinus=0;
}


t_Time IMTimer::getTime()
{
 // return getTime(millisT2());
   return ((millisTNow()-start) %stages[PERIOD]);
}
 /*
t_Time IMTimer::getTime(t_Time time)
{

//  return ((time-start) %stages[PERIOD]);
 return ((time-start) %CycleDuration);
}
*/

void IMTimer::printTime()
{
   DBGINFO(millisT2());
          DBGINFO(":");
   DBGINFO(millis());
          DBGINFO(":");
//   DBGINFO(millis()-millisT2());
//          DBGINFO(":");
   DBGINFO(getTime());
          DBGINFO(">");
}


long IMTimer::CycleHour()
{
 // return long(3600000L / stages[PERIOD]);
 return 1200;
}

long IMTimer::Cycle()
{
  return cycle;
}

void IMTimer::Watchdog()
{
  watchdog=0;
}

bool IMTimer::Watchdog(uint16_t dog)
{
  watchdog++;
  return watchdog>dog;
}

void IMTimer::Setup(byte stage, unsigned long waittime)
{
  stages[stage]=waittime;
  compute();
}


void IMTimer::compute()
{
 // t_Time last=nearTime;
  t_Time last=getTime();
  if (last>=(stages[PERIOD]-50))
      last=1;
  nearTime=stages[PERIOD]-50;
  nearStage=PERIOD;
  for(byte i=1 ;i<maxStages;i++)
  {
    if ((nearTime>stages[i])&&(stages[i]>last))
      {
        nearTime=stages[i];
        nearStage=i;
      }
  }

}



void IMTimer::setStage(byte stage)
{
  current=stage;
}

t_Time IMTimer::setNextTime()
{
  t_Time nextTT1=millisTNow()-start;
  if (nextTT1>0xFFFFFFF)
     nextTT1+= stages[PERIOD];
  if (nextTT1>0xFFFFFFF)
     nextTT1+= stages[PERIOD];

  t_Time nextTT=nextTT1-getTime()+nearTime;
  if (nextTT>0xFFFFFFF)
     nextTT+= stages[PERIOD];

  if (nextTT<nextTT1)  {
     nextTT+= stages[PERIOD];
  }

  return nextTT+start;

}
byte IMTimer::WaitStage()
{
  setSleepModeT2();
  sei();
  t_Time dTT=millisTNow()-nearTimeTT;
  if ((dTT>30)){
  t_Time nextTT=setNextTime();
  stopTimer2(nextTT);
  nearTimeTT=nextTT;
  while ((millisTNow())<nextTT)
  {
//     if (!_listen && !_measure)
     if (_measure==0)
        delayT2();
     if (_listen){
       _listen=0;
      // stopTimer2(start);
       return current;
     }
     if (_measure){
       _measure=0;
    //   stopTimer2(start);
       _measure=0;
       return MEASUREDATA;
     }
  }
  }

  sei();

  byte r= nearStage;
  if (r==PERIOD) {
//  DBGPINHIGH();
     cycle++;
     _synchronizeStart+=syncRate;
     if (_synchronizeStart >SynchronizeCycle){
        _synchronizeStart-=SynchronizeCycle;
        syncTimer2(SynchronizeStep);
     }
//     _synchronizeStart+=syncRate;
//     watchdog++;
//     if (((_synchronizeStart) % SynchronizeCycle)==0)
  //     syncTimer2(SynchronizeStep);
//     delaySleepT2(30);
       if ((cycle % CycleHour()) ==0){
         r=CRONHOUR;
     }
  }
  compute();
  nearTimeTT=setNextTime();
//  setNextTime();        hand after 1min
  return r;
}

 void IMTimer::doneReceived(byte count)
{
    ptrr->doneListen();
}

 void IMTimer::doneMeasure()
{
   ptrr->_measure++;
}

void IMTimer::doneListen()
{
   _listen++;
//   waiting=0;
}

void IMTimer::doneWrite()
{
}

void IMTimer::StepTimerNull(){
    toggle = ~toggle;
    digitalWrite(DBGCLOCK,toggle);
}
short IMTimer::ClassTest()
{
    IMTimer t;
    t.Setup(t.PERIOD,10000);
    t.Setup(1,3000);
    if ( t.nearTime!=t.stages[1])
       return 1;
    t.Setup(2,1000);
    if ( t.nearTime!=t.stages[2])
       return 2;
    t.Setup(2,6000);
    if ( t.nearTime!=t.stages[1])
       return 3;
    return 0;
}



#if defined(__AVR_ATmega328P__)
ISR(TIMER2_COMPA_vect) {
  #ifdef CRYSTAL32K
  OCR2A=addTimer2(OCR2A);
//  OCR2A=nextTimer2();
  #else
  incTimer2();
  #endif
  #ifdef DBGCLOCK
   IMTimer::funStep();
//    toggle = ~toggle;
 //   digitalWrite(DBGCLOCK,HIGH);
//    digitalWrite(DBGCLOCK,LOW);
//    digitalWrite(DBGCLOCK,toggle);
  #endif
}
#endif

//http://www.engblaze.com/microcontroller-tutorial-avr-and-arduino-timer-interrupts/


/*
ISR(TIMER2_OVF_vect) {
//  TCNT2 = tcnt2;
//  toggle = ~toggle;
//  counterTimer2++;               //Increments the interrupt counter
  TCNT2 = counterTCNT2;           //Reset Timer to 130 out of 255
  TIFR2 = 0x00;
  incTimer2();

}
*/



/*
void pin2_isr()
{

sleep_disable();
detachInterrupt(0);
pin2_interrupt_flag = 1;
}
*/
