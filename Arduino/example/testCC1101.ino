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
#define MMAC 0x100001; // My MAC




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


#define RadioDelay 100  //Time between calls - Let hardware serial write out async
#define DataDelay 500
#define KnockDelay 1200
#define KnockDuration 200
#define DataDuration 300
#define CycleDuration 5000
/****************** Vars *******************************/



char radioBuf[RADIO_BUFFSIZE] = {0,};
unsigned short radioBufLen = 0;
unsigned short radio_writeout = 0xFFFF;
//unsigned long radioOut_delay = 0;
unsigned long knockTimeOut =0;
//unsigned long listenTimeOut = 0;


IMCC1101  cc1101;
IMTimer  Timer;
Transceiver trx;
volatile byte ReadState = 0;
volatile byte WriteState= 0;

#define SENDKNOCK 2
#define PRINTRADIO 3
#define SENDDATA 4
#define STOPLISTENKNOCK 5
#define STOPLISTENDATA 6

#define LISTENKNOCK 101
#define LISTENDATA 102
#define LISTENHELLO 103

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

/*ISR (PCINT0_vect) // handle pin change interrupt for D8 to D13 here
 {
   if (trx.state==TransceiverRead)
   {
     ReadState++;
   } else {
     WriteState++;
   }

 }*/

 bool CheckReadState()
 {
   if (ReadState>1){
//     DBGINFO(">");
      ReadState=0;

      return true;
   }
   return false;
 }

 bool CheckWriteState()
 {
   if (WriteState>0){
      WriteState=0;
      return true;
   }
   return false;
 }

 
// ISR (PCINT2_vect) // handle pin change interrupt for D8 to D13 here
// { 
// ReadState=3;  
// } 


void OnRead()
{
 ReadState=1;
}



//Main loop is called over and over again

void SendData()
{
   if (trx.Connected())
   {
    static IMFrame frame;
        frame.Reset();
        UartPrepareData(frame);
        trx.Send(frame);
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
  } else {
    Timer.setStage(LISTENKNOCK);
  }
}

void SendKnock()
{
   if (trx.Connected())
   {
     if ((Timer.Cycle() %10)==0)
     {
       trx.Knock();
       Timer.setStage(LISTENHELLO);
     }

   } else{
      Timer.setStage(LISTENKNOCK);
   }
}

byte GetData()
{
  static IMFrame rxFrame;
  if (trx.GetData()  )
    {
      trx.printReceive();
      if (trx.GetFrame(rxFrame))
      {
        DBGINFO(" RSSI: ");           DBGINFO(trx.Rssi());            DBGINFO("dBm");
        if (rxFrame.Knock())
        {
           DBGINFO(" ResponseKnock ");
           if (trx.ResponseKnock(rxFrame))
             return 1;
        }
        else if (rxFrame.Welcome())
        {
           if (trx.ReceiveWelcome(rxFrame))
              DBGINFO(" Welcome ");
        }
        else if (rxFrame.Hello())
           DBGINFO(" Hello ");
//          trx.parseKnock()
        else if (rxFrame.ACK())
        {
           trx.ReceiveACK(rxFrame);

           DBGINFO(" ACK ");
        } else{
          if (rxFrame.NeedACK())
             trx.SendACK(rxFrame);
          if  (rxFrame.Destination()!=MID)
          {
             if (trx.Routing(rxFrame))
             DBGINFO(" Route ");
          }
          else
          {
            radioBufLen+=rxFrame.Get((uint8_t*)&(radioBuf[radioBufLen]));
            if (radio_writeout == 0xFFFF) radio_writeout = 0;  //signal the uart handler to start writing out
          }
          if (rxFrame.Repeat())
              return 1;
        }
      }
      else
      {
        DBGERR("!VALID");
      }
      DBGINFO(millis());
      DBGINFO("\r\n");
  }
  return 0;

}

void ListenData()
{
     trx.StartReceive();
     if (CheckReadState())
     {

        while (GetData()){
          trx.StartReceive();
        }
        trx.StartReceive();
     };
}

void StopListen()
{
  trx.Idle();
  Timer.setStage(Timer.IDDLESTAGE);

}

void stageloop(byte stage)
{
  DBGINFO("stageloop=");  DBGINFO(millis());
  DBGINFO(":");  DBGINFO(stage);
  switch (stage)
  {
    case SENDKNOCK:  SendKnock();      break;
    case STOPLISTENKNOCK:  StopListen();      break;
    case STOPLISTENDATA:   StopListen();      break;
    case PRINTRADIO:     printRadio(); break;
    case SENDDATA: SendData();break;
    case LISTENDATA : ListenData();break;
    case LISTENKNOCK : ListenData();break;
    case LISTENHELLO : ListenData();break;

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
  trx.myID= MID;
  trx.myMAC=MMAC;
  Timer.onStage=stageloop;
//  pciSetup(7);
  pciSetup(9);
//  DBGINFO("classtest TRX");  DBGINFO(Transceiver::ClassTest());
  DBGINFO("classtest Timer");  DBGINFO(IMTimer::ClassTest());
    Timer.Setup(Timer.PERIOD,CycleDuration);
    Timer.Setup(PRINTRADIO,RadioDelay);
    Timer.Setup(SENDDATA,DataDelay);
    Timer.Setup(STOPLISTENDATA,DataDelay+DataDuration);
    Timer.Setup(SENDKNOCK,KnockDelay);
    Timer.Setup(STOPLISTENKNOCK,KnockDelay+KnockDuration);

}




void loop()
{
  ERRLEDON();         delay(50);         ERRLEDOFF();

//  trx.StartReceive();
  /************** radio to UART ************************/
  byte xstage;
  do{

     xstage=Timer.WaitStage();
     stageloop(xstage);
  }while( xstage==Timer.PERIOD);

  DBGINFO(")\r\n");

}

