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


#include "imtrans.h"
#include "imtimer.h"
#include "uart.h"



/******************************** Configuration *************************************/

#define MID  0x02  //My ID
#define MMAC 0x102030  // My MAC




#define RADIO_BUFFSIZE 725  //Buffer for radio

#define RESEND_TIMEOUT1 75  //Wait this time in ms for an radio ack

//Used for error signaling (ON when restransmitting, OFF on receive success)
#define ERRLEDON() digitalWrite(13,HIGH)
#define ERRLEDOFF() digitalWrite(13,LOW)
#define ERRLEDINIT() pinMode(13, OUTPUT)



/******************************* Module specific settings **********************/




  





#define INDEX_NWID 0  //Network ID
#define INDEX_SRC  1  //Source ID
#define INDEX_DEST 2  //Target ID
#define INDEX_SEQ  3  //Senders current sequence number


#define RadioDelay 2900  //Time between calls - Let hardware serial write out async
#define BroadcastDelay 200
#define BroadcastDuration 400
#define BroadcastCallibrate 300


#define SlaveDelay 700
#define DataDelay 1000
#define DataDuration 300
#define SlaveDuration 300
#define CycleDuration 3000
/****************** Vars *******************************/



char radioBuf[RADIO_BUFFSIZE] = {0,};
unsigned short radioBufLen = 0;
unsigned short radio_writeout = 0xFFFF;


IMCC1101  cc1101;
IMTimer  Timer;
Transceiver trx;
//volatile byte ReadState = 0;
//volatile byte WriteState= 0;

#define PRINTRADIO 1
#define STARTBROADCAST 2
#define STOPBROADCAST 3
#define STARTDATA 4
#define STOPDATA 5
#define STARTSLAVE 7

#define LISTENBROADCAST 101
#define LISTENDATA 102
#define LISTENSLAVE 103

/************************* Functions **********************/


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
  Timer.doneListen();
}


void ListenBroadcast()
{
      trx.setChannel(trx.BroadcastChannel);
      Timer.setStage(LISTENBROADCAST);
      trx.StartReceive();

}

void SendSlave()
{
   if (trx.Connected())
   {
      trx.setChannel(trx.SlaveChannel);
      Timer.setStage(LISTENSLAVE);
      trx.StartReceive();
   }

}

void SendData()
{
   if (trx.Connected())
   {
      static IMFrame frame;
      frame.Reset();
      UartPrepareData(frame);

      trx.SendData(frame);
      DBGINFO("PUSH ");
      if (trx.Retry())
      {
         DBGINFO("Retry");
      }
      if (trx.Transmit())
      {
         DBGINFO("transmit:");  DBGINFO(millis());    DBGINFO(" ");
         DBGINFO(trx.TX_buffer.len);    DBGINFO(",");
      }
      Timer.setStage(LISTENDATA);
      trx.StartReceive();
   } else {
     ListenBroadcast();
   }

}

void SendKnock()
{
   if (trx.Connected())
   {
     if ((Timer.Cycle() %5)==0)
     {
       trx.Knock();
     }

   };
   ListenBroadcast();


}

void CheckSlave()
{
   if (trx.Connected())
   {
       DBGINFO("Slave");
       trx.setChannel(trx.SlaveChannel);
       Timer.setStage(LISTENSLAVE);
       trx.StartReceive();

   } else{
     ListenBroadcast();
   }

}


byte GetData()
{
  static IMFrame rxFrame;
  if (trx.GetData()  )
    {
//      trx.printReceive();
      if (trx.GetFrame(rxFrame))
      {
        DBGINFO(" RSSI: ");           DBGINFO(trx.Rssi());            DBGINFO("dBm\r\n");
        if (rxFrame.Knock())
        {
           DBGINFO(" receiveKnock ");
           if (trx.myHost(rxFrame)){
             Timer.Calibrate(millis()-BroadcastCallibrate);
             if (trx.ResponseHello(rxFrame)){
                 DBGINFO(" rHELLO ");
                 return 1;
              }

           } else {
             DBGINFO(" alien host ");
           }
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


        } else if (trx.Onward(rxFrame)){

           DBGINFO(" Onward ");
        }
        else if (rxFrame.ACK())
        {
              trx.ReceiveACK(rxFrame);
           DBGINFO(" ACK ");
        } else{
          if (rxFrame.NeedACK())
             trx.SendACK(rxFrame);
            radioBufLen+=rxFrame.Get((uint8_t*)&(radioBuf[radioBufLen]));
            if (radio_writeout == 0xFFFF) radio_writeout = 0;  //signal the uart handler to start writing out
            if (rxFrame.Repeat())
               return 1;
        }
      }
      else
      {
        DBGERR("!VALID");
      }
//      DBGINFO(millis());
//      DBGINFO("\r\n");
  }
  return 0;

}

void ListenData()
{
       DBGINFO('*');

        while (GetData()){
          trx.StartReceive();
        }
        trx.StartReceive();
}

void StopListen()
{
 
  if (trx.Connected())
  {
    DBGINFO("stop listen");
//    trx.Idle();
//    Timer.setStage(Timer.IDDLESTAGE);
  }

}

void stageloop(byte stage)
{
//  DBGINFO("stageloop=");  DBGINFO(millis());
//  DBGINFO(":");  DBGINFO(stage);
  switch (stage)
  {
    case STARTBROADCAST:  SendKnock();      break;
    case STOPBROADCAST:  StopListen();      break;
    case STARTSLAVE:  SendSlave();      break;
    case STARTDATA: SendData();break;
    case STOPDATA:   StopListen();      break;

    case PRINTRADIO:     printRadio(); break;

    case LISTENDATA : ListenData();break;
    case LISTENBROADCAST : ListenData();break;
    case LISTENSLAVE : ListenData();break;

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
  Timer.onStage=stageloop;
  pciSetup(9);
//  DBGINFO("classtest TRX");  DBGINFO(Transceiver::ClassTest());
  DBGINFO("classtest Timer");  DBGINFO(IMTimer::ClassTest());
    Timer.Setup(Timer.PERIOD,CycleDuration);
    Timer.Setup(PRINTRADIO,RadioDelay);

    Timer.Setup(STARTSLAVE,SlaveDelay);
    Timer.Setup(STARTDATA,DataDelay);
    Timer.Setup(STOPDATA,DataDelay+DataDuration);

    Timer.Setup(STARTBROADCAST,BroadcastDelay);
    Timer.Setup(STOPBROADCAST,BroadcastDelay+BroadcastDuration);

}




void loop()
{
  ERRLEDON();         delay(50);         ERRLEDOFF();

  /************** radio to UART ************************/
  byte xstage;
  do{

     xstage=Timer.WaitStage();
     stageloop(xstage);
  }while( xstage==Timer.PERIOD);

//  DBGINFO(")\r\n");

}

