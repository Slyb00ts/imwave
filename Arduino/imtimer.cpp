#include <imtimer.h>

#ifdef DBGCLOCK
byte toggle;
#endif

IMTimer* IMTimer::ptrr = 0;
//volatile byte IMTimer::State = 0;

IMTimer::IMTimer()
{
	ptrr = this;	//the ptr points to this object
        noInterrupts();
  #if defined(__sleepT2)
        setupTimer2();
        void disableADCB();
  #endif
        DeviationPlus=0;
        DeviationMinus=0;
        watchdog=0;
        cycle=0;
        Setup(IMTimer::PERIOD,CycleDuration);
}

void IMTimer::Calibrate(t_Time time)
{
  t_Time del=start;
  start=time;
  if (time> 10000)
   start=time-stages[PERIOD];

   del=(time-del)%1000;
   if (del){
     DBGINFO("calib:");
     DBGINFO(del);
   }
   if (del<500)
     DeviationPlus+=del;
   else
   {
     DeviationMinus-=del;
     DeviationMinus+=1000;
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
   return ((millisT2()-start) %stages[PERIOD]);
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
  t_Time last=nearTime;
  if (last>=(stages[PERIOD]-10))
      last=1;
  nearTime=stages[PERIOD]-10;
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
  t_Time nextTT1=millisT2()-start;
  if (nextTT1>0xFFFFFFF)
     nextTT1+= stages[PERIOD];
  if (nextTT1>0xFFFFFFF)
     nextTT1+= stages[PERIOD];

  t_Time nextTT=nextTT1-getTime()+nearTime;
  if (nextTT>0xFFFFFFF)
     nextTT+= stages[PERIOD];

  if (nextTT<nextTT1)
     nextTT+= stages[PERIOD];
  stopTimer2(nextTT+start);
  return nextTT;

}
byte IMTimer::WaitStage()
{
/*  DBGINFO("\r\n{{");
  DBGINFO(nearTime);
  DBGINFO('%');
  DBGINFO(millis());
  DBGINFO('%');
  DBGINFO(getTime());
  DBGINFO('%');
  DBGINFO(cycle);
  Serial.flush();
*/
  setSleepModeT2();
  sei();
  t_Time nextTT=setNextTime();
  while ((millisT2()-start)<nextTT)
  {
     waiting++;

 //    if (waiting %4==0)
     delayT2();
     if (_listen){
       _listen=0;
       stopTimer2(start);
  //     DBGPINHIGH();
       return current;
     }
  }

  sei();

  byte r= nearStage;
  if (r==PERIOD) {
//  DBGPINHIGH();
     cycle++;
//     watchdog++;
     delaySleepT2(30);
     if ((cycle % CycleHour()) ==0){
       r=CRONHOUR;
     }
     DBGINFO(waiting);
     waiting=0;
//     DBGPINLOW();
  }
  compute();
//  setNextTime();        hand after 1min
  return r;
}

 void IMTimer::doneReceived(byte count)
{
//       DBGINFO("RECEIVED");
//  _listen++;
    ptrr->doneListen();
}
void IMTimer::doneListen()
{
   _listen++;
   waiting=0;
}

void IMTimer::doneWrite()
{
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
    toggle = ~toggle;
 //   digitalWrite(DBGCLOCK,HIGH);
//    digitalWrite(DBGCLOCK,LOW);
    digitalWrite(DBGCLOCK,toggle);
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
