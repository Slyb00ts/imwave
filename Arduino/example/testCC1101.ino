/*
        mesh CC1101 example
        for more information: www.imwave

	Copyright (c) 2015 Dariusz Mazur
        
        v0.1 - 2015.09.01
	    Basic implementation - functions and comments are subject to change
        
	Permission is hereby granted, free of charge, to any person obtaining a copy of this software
	and associated documentation files (the "Software"), to deal in the Software without restriction,
	including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
	and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
	LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
	WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

//#.define DBGLVL 2
#include "imdebug.h"





/******************************** Configuration *************************************/

#define MMAC 0x102040  // My MAC

#define RADIO_BUFFSIZE 63  //Buffer for radio




#define RadioDelay 2900  //Time between calls - Let hardware serial write out async
#define BroadcastDelay 200
#define BroadcastDuration 400
#define BroadcastCallibrate 300


#define DataDelay 1000
#define DataDuration 300
#define CycleDuration 3000


/************************* Module specyfic functions **********************/


#include "imtrans.h"
#include "imtimer.h"
#include "uart.h"



char radioBuf[RADIO_BUFFSIZE] = {0,};
unsigned short radioBufLen = 0;
unsigned short radio_writeout = 0xFFFF;

/******************************* Objects **********************/

IMCC1101  cc1101;
Transceiver trx;

int freeRam ()
{
      extern int __heap_start, *__brkval;
      int v;
      return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void printRadio()
{
    DBGINFO("Radio:");
    DBGINFO(radioBufLen);
    for (int i=0 ; (radio_writeout<radioBufLen) ; radio_writeout++, i++ )
    {
      DBGINFO(radioBuf[radio_writeout]);
    }
    DBGINFO(";\r\n");
    radio_writeout = 0xFFFF;  //signal write complete to radio reception
    radioBufLen = 0;
    for(byte x= ((millis() %3) );(x>0);x--)
    {
      generatorUart();
    }
}

void pciSetup(byte pin)
{
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}





void OnRead(byte state)
{
  DBGINFO(state);
  if (state==2)
  {
    return ;
  }
  trx.timer.doneListen();
}





void SendData()
{
   if (trx.Connected())
   {
      static IMFrame frame;
      frame.Reset();
      UartPrepareData(frame);

      DBGINFO("SendData ");
      trx.SendData(frame);
      if (trx.Retry())
      {
         DBGINFO("Retry");
      }
      if (trx.Transmit())
      {
         DBGINFO("transmit:");  DBGINFO(millis());    DBGINFO(" ");
         DBGINFO(trx.TX_buffer.len);    DBGINFO(",");
      }
      DBGINFO("\r\n");
      trx.ListenData();

   } else {
     trx.ListenBroadcast();
   }

}



void ReceiveData()
{
  static IMFrame rxFrame;
  if (trx.GetData()  )
  {
//    trx.printReceive();
      if (trx.GetFrame(rxFrame))
      {
        DBGINFO(" RSSI: ");           DBGINFO(trx.Rssi());            DBGINFO("dBm  ");
        if (rxFrame.Knock())
        {
           DBGINFO("\r\n receiveKnock ");
           if (trx.ReceiveKnock(rxFrame))
           {
              DBGINFO(" sendHello ");
           }
           DBGINFO(" \r\n");
        }
        else if (rxFrame.Hello())
        {
           if (trx.ForwardHello(rxFrame))
              DBGINFO(" FORWARDHello ");
        }
        else if (rxFrame.Welcome())
        {
           if (trx.ReceiveWelcome(rxFrame))
              DBGINFO(" Welcome ");

        }
        else if (trx.Onward(rxFrame))
        {
              DBGINFO(" Onward ");
        }
        else if (rxFrame.ACK())
        {
              trx.ReceiveACK(rxFrame);
              DBGINFO(" ACK ");
        }
        else
        {
          if (rxFrame.NeedACK())
             trx.SendACK(rxFrame);
          DBGINFO(" rxGET ");
          radioBufLen+=rxFrame.Get((uint8_t*)&(radioBuf[radioBufLen]));
          if (radio_writeout == 0xFFFF) radio_writeout = 0;  //signal the uart handler to start writing out
        }
      }
      else
      {
        DBGERR("!VALID");
      }
//      DBGINFO(millis());
      DBGINFO("\r\n");
  }

}



void stageloop(byte stage)
{
//  DBGINFO("stageloop=");  DBGINFO(millis());
//  DBGINFO(":");  DBGINFO(stage);
  switch (stage)
  {
    case STARTBROADCAST:  trx.ListenBroadcast();      break;
    case STOPBROADCAST:  trx.Knock();      break;
    case STARTDATA: SendData();break;
    case STOPDATA:   trx.StopListen();      break;
    case PRINTRADIO:     printRadio(); break;
    case LISTENDATA : ReceiveData();break;
    case LISTENBROADCAST : ReceiveData();break;

    default:
    break;
  }
}


//Initialize the system
void setup()
{
  INITDBG();
  interrupts ();
//  attachInterrupt(4, OnRead, FALLING );
  ERRLEDINIT(); ERRLEDOFF();
  trx.Init(cc1101);
  trx.myMAC=MMAC;
  trx.onEvent=OnRead;
  trx.timer.onStage=stageloop;
  pciSetup(9);
//  DBGINFO("classtest TRX");  DBGINFO(Transceiver::ClassTest());
   DBGINFO("classtest Timer");  DBGINFO(IMTimer::ClassTest());
    trx.timer.Setup(trx.timer.PERIOD,CycleDuration);
    trx.timer.Setup(PRINTRADIO,RadioDelay);
    trx.timer.Setup(STARTDATA,DataDelay);
    trx.timer.Setup(STOPDATA,DataDelay+DataDuration);
    trx.timer.Setup(STARTBROADCAST,BroadcastDelay);
    trx.timer.Setup(STOPBROADCAST,BroadcastDelay+BroadcastDuration);
}




void loop()
{
//  DBGINFO(freeRam());
//  DBGINFO("FREE\r\n");
  ERRFLASH();
  byte xstage;
  do{

     xstage=trx.timer.WaitStage();
     stageloop(xstage);
  }while( xstage==trx.timer.PERIOD);
//  DBGINFO(")\r\n");

}

