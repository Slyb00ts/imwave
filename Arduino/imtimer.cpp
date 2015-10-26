#include <imtimer.h>

IMTimer* IMTimer::ptrr = 0;
//volatile byte IMTimer::State = 0;

IMTimer::IMTimer()
{
	ptrr = this;	//the ptr points to this object
}

void IMTimer::Calibrate(unsigned long time)
{
   start=time;
   DBGINFO("cl%");
   DBGINFO(start);
}

unsigned long IMTimer::getTime()
{
  return getTime(millis());
}

unsigned long IMTimer::getTime(unsigned long time)
{

  return ((time-start) %stages[PERIOD]);
}


long IMTimer::Cycle()
{
  return cycle;
}


void IMTimer::Setup(byte stage, unsigned long waittime)
{
  stages[stage]=waittime;
  compute();
}


void IMTimer::compute()
{
  unsigned long last=nearTime;
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
/*
byte IMTimer::Find(unsigned long _next)
{
  for(byte i=0 ;i<maxStages;i++)
  {
    if  (_next=stages[i])
       return i;
  }
  return 0;

}
*/

void IMTimer::setStage(byte stage)
{
  current=stage;
}



byte IMTimer::WaitStage()
{
/*  DBGINFO("{\r\n");
  DBGINFO(nearTime);
  DBGINFO('%');
  DBGINFO(waiting);
  DBGINFO('%');
  DBGINFO(getTime());
  DBGINFO('%');
  DBGINFO(cycle);
*/
  while(nearTime >getTime())
  {
   waiting++;
   if ((nearTime+3) > getTime()) {
     delay(2);
   }
   if (_listen){
     _listen=0;
     return current;
   }
   if ((waiting % 2000)==2)
   {
     return 8;
   }

  }
  byte r= nearStage;
  if (r==PERIOD)
     cycle++;
  compute();


  return r;

}
void IMTimer::doneListen()
{
   _listen++;
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





//http://www.engblaze.com/microcontroller-tutorial-avr-and-arduino-timer-interrupts/



/*
void pin2_isr()
{

sleep_disable();
detachInterrupt(0);
pin2_interrupt_flag = 1;
}
*/
