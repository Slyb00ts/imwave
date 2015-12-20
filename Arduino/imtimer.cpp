#include <imtimer.h>

IMTimer* IMTimer::ptrr = 0;
//volatile byte IMTimer::State = 0;

IMTimer::IMTimer()
{
	ptrr = this;	//the ptr points to this object
}

void IMTimer::Calibrate(unsigned long time)
{
  unsigned long del=start;
   start=time;
   del=time-del- stages[PERIOD]+50000;
   DBGINFO("cl");
   DBGINFO(del);
   DBGINFO("%");
   DBGINFO(time);

}

unsigned long IMTimer::getTime()
{
  return getTime(millis());
}

unsigned long IMTimer::getTime(unsigned long time)
{

  return ((time-start) %stages[PERIOD]);
}

void IMTimer::sleep(unsigned long time)
{
  period_t xtime;
  if (time==15)  xtime=SLEEP_15Ms;
  else if (time==30) xtime=SLEEP_30MS;
  else if (time==60) xtime=SLEEP_60MS;
  else {
    delay(time);
//    return;
  }
//   delay(time);
 // LowPower.powerDown(xtime, ADC_OFF, BOD_OFF);
 LowPower.idle(xtime, ADC_OFF, TIMER4_OFF,TIMER3_OFF,TIMER1_ON,TIMER0_ON, SPI_ON,USART1_OFF,TWI_ON, USB_ON);



}

long IMTimer::Cycle()
{
  return cycle;
  DBGINFO("CYC(");
  DBGINFO(cycle);

}

void IMTimer::Watchdog()
{
  watchdog=0;
}

bool IMTimer::Watchdog(byte dog)
{
  return watchdog>dog;
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



void IMTimer::setStage(byte stage)
{
  current=stage;
}



byte IMTimer::WaitStage()
{
/*  DBGINFO("{\r\n");
  DBGINFO(nearTime);
  DBGINFO('%');
  DBGINFO(millis());
  DBGINFO('%');
  DBGINFO(getTime());
  DBGINFO('%');
  DBGINFO(cycle);
  */
  while(nearTime >getTime())
  {
     waiting++;
     long next=nearTime-getTime();
     if (next > 3) {
//       if (next>202)
//          delay(200);
//       else
       if (next>72)
          sleep(60);
       else
         sleep(2);
     }
     if (_listen){
       _listen=0;
       DBGINFO(">>");
       DBGINFO(millis());
       return current;
     }
     if ((waiting % 2000)==2)
     {
       return LAP;
     }

  }


  byte r= nearStage;
  if (r==PERIOD) {
     cycle++;
     watchdog++;
     delay(20);
  }
  compute();


  return r;

}
void IMTimer::doneListen()
{
   _listen++;
   DBGINFO("??");
   DBGINFO(millis());

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
