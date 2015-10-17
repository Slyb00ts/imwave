#include <imtimer.h>

IMTimer* IMTimer::ptrr = 0;

IMTimer::IMTimer()
{
	ptrr = this;	//the ptr points to this object

}

void IMTimer::Next(byte stage, unsigned long waittime)
{
  delays[stage]=waittime;
  stages[stage]=millis()+waittime;
  compute();
}

void IMTimer::Next(byte stage)
{
  if (delays[stage])
  {
    stages[stage]=millis()+delays[stage];
  } else {
    stages[stage]=0;
  }
  compute();
}

bool IMTimer::Arrived(byte stage)
{
 return (stages[stage]>millis());
}

void IMTimer::compute()
{
  for(byte i=0 ;i<maxStages;i++)
  {
    if (nearStage>stages[i])
      {
        nearStage=stages[i];
      }
  }

}
byte IMTimer::Find(unsigned long _next)
{
  for(byte i=0 ;i<maxStages;i++)
  {
    if  (_next=stages[i])
       return i;
  }
  return 0;

}

byte IMTimer::WaitStage()
{
  while(nearStage <millis())
  {
   waiting++;

  }
  byte r= Find(nearStage);
  if (r){
    Next(r);


  }
  return r;

}

void IMTimer::doRupture(byte state)
{
  ruptures[state]++;
}

short IMTimer::ClassTest()
{
    IMTimer t;
    t.Next(1,3000);
    if ( t.nearStage!=t.stages[1])
       return 1;
    t.Next(2,1000);
    if ( t.nearStage!=t.stages[2])
       return 2;
    t.Next(2,6000);
    if ( t.nearStage!=t.stages[1])
       return 3;
    return 0;
}





//http://www.engblaze.com/microcontroller-tutorial-avr-and-arduino-timer-interrupts/

  /*
ISR(PCINT0_vect) // handle pin change interrupt for D8 to D13 here
{
   IMTimer::ptrr->doRupture(IMTimer::State);

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
