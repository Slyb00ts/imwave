#include <imtimer.h>

IMTimer* IMTimer::ptrr = 0;
//volatile byte IMTimer::State = 0;

IMTimer::IMTimer()
{
	ptrr = this;	//the ptr points to this object
        setupTimer2();
}

void IMTimer::Calibrate(unsigned long time)
{
  unsigned long del=start;
   start=time;
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


unsigned long IMTimer::getTime()
{
  return getTime(millisT2());
}

unsigned long IMTimer::getTime(unsigned long time)
{

  return ((time-start) %stages[PERIOD]);
}

void IMTimer::printTime()
{
   DBGINFO(millis());
          DBGINFO(":");
   DBGINFO(getTime());
          DBGINFO(">");
}
void IMTimer::sleep(unsigned long time)
{
/*  period_t xtime;
  if (time==15)  xtime=SLEEP_15Ms;
  else if (time==30) xtime=SLEEP_30MS;
  else if (time==60) xtime=SLEEP_60MS;
  else if (time>=120) xtime=SLEEP_120MS;
  else {
    delay(5);
    return;
  }
  */
   delay(3);
// unsigned long xstart=millis();
// LowPower.idle(xtime, ADC_OFF, TIMER4_OFF,TIMER3_OFF,TIMER1_ON,TIMER0_ON, SPI_ON,USART1_OFF,TWI_ON, USB_ON);
/*      DBGINFO("\r\n<<");
       DBGINFO(millis()-xstart);
       DBGINFO(" ");
       DBGINFO(time);
*/
//  LowPower.powerSave(xtime, ADC_OFF, BOD_ON,TIMER2_ON);

//  LowPower.powerStandby(xtime, ADC_OFF, BOD_ON);

}

uint16_t IMTimer::CycleHour()
{
  return uint16_t(3600000 / stages[PERIOD]);
}

long IMTimer::Cycle()
{
  return cycle;
//  DBGINFO("CYC(");
//  DBGINFO(cycle);

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

/*  DBGINFO("\r\n{{");
  DBGINFO(nearTime);
  DBGINFO('%');
  DBGINFO(millis());
  DBGINFO('%');
  DBGINFO(getTime());
  DBGINFO('%');
  DBGINFO(cycle);
  */
  Serial.flush();
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
//  set_sleep_mode(SLEEP_MODE_IDLE);
  cli();

  while(nearTime >getTime())
  {
    goSleep();
     /*
     long next=nearTime-getTime();
     if (next > 3) {
//       waiting++;
       if (next>202)
          sleep(120);
       else
       if (next>72)
          sleep(60);
       else
         sleep(2);
     }
     */
     if (_listen){
       _listen=0;
//       DBGINFO("<()>");
//       printTime();
//       DBGINFO(millis());
       return current;
     }

    cli();

  }
  sei();

  byte r= nearStage;
  if (r==PERIOD) {
     cycle++;
//     watchdog++;
     delaySleep(30);
     if ((cycle % CycleHour()) ==0){
       r=CRONHOUR;

     }
  }
  compute();


  return r;

}

 void IMTimer::doneReceived(byte count)
{
//       DBGINFO("RECEIVED");
//  _listen++;
//  if (count>10)
    ptrr->doneListen();
}
void IMTimer::doneListen()
{
   _listen++;
   waiting=0;
//   DBGINFO("??");
//   DBGINFO(millis());

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

ISR(TIMER2_OVF_vect) {
  /* Reload the timer */
//  TCNT2 = tcnt2;
  /* Write to a digital pin so that we can confirm our timer */
//  toggle = ~toggle;
  incTimer2();
//  counterTimer2++;               //Increments the interrupt counter
  TCNT2 = counterTCNT2;           //Reset Timer to 130 out of 255
  TIFR2 = 0x00;
}



/*
void pin2_isr()
{

sleep_disable();
detachInterrupt(0);
pin2_interrupt_flag = 1;
}
*/
