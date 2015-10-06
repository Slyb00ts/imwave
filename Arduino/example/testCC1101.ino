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
#include "uart.h"



/******************************** Configuration *************************************/

#define MID  0x00  //My ID
#define TID  0x00  //Default target ID



#define RADIO_BUFFSIZE 725  //Buffer for radio
#define RADIO_SENDTHRES 710  //Start writing when this full

#define RESEND_TIMEOUT 75  //Wait this time in ms for an radio ack

//Used for error signaling (ON when restransmitting, OFF on receive success)
#define ERRLEDON() digitalWrite(13,HIGH)
#define ERRLEDOFF() digitalWrite(13,LOW)
#define ERRLEDINIT() pinMode(13, OUTPUT)



/******************************* Module specific settings **********************/




  
  #define bridgeSpeed 9600
  #define bridgeBurst 10  //Chars written out per call
  #define radioDelay 3000  //Time between calls - Let hardware serial write out async





#define INDEX_NWID 0  //Network ID
#define INDEX_SRC  1  //Source ID
#define INDEX_DEST 2  //Target ID
#define INDEX_SEQ  3  //Senders current sequence number


#define knockDelay 10000
#define listenDelay 2000

/****************** Vars *******************************/



char radioBuf[RADIO_BUFFSIZE] = {0,};
unsigned short radioBufLen = 0;
unsigned short radio_writeout = 0xFFFF;
unsigned long radioOut_delay = 0;
unsigned long knockTimeOut =0;
unsigned long listenTimeOut = 0;


IMCC1101  cc1101;
Transceiver trx;
volatile byte ReadState = 0;
volatile byte WriteState= 0;

/************************* Functions **********************/


void printRadio()
{
    DBGINFO("Radio:");
    DBGINFO(radioBufLen);
    for (int i=0 ; i<bridgeBurst && (radio_writeout<radioBufLen) ; radio_writeout++, i++ )
    {
      DBGINFO(radioBuf[radio_writeout]);
    }
    DBGINFO(";\r\n");

}

void pciSetup(byte pin)
{
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

ISR (PCINT0_vect) // handle pin change interrupt for D8 to D13 here
 {
   if (trx.state==TransceiverRead)
   {
     ReadState++;
   } else {
     WriteState++;
   }

 }

 bool CheckReadState()
 {
   if (ReadState>1){
     DBGINFO("<");
     DBGINFO(ReadState);
     DBGINFO("+");
     DBGINFO(WriteState);
     DBGINFO(">");
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

//Initialize the system
void setup()
{
  INITDBG();
  interrupts ();
//  attachInterrupt(4, OnRead, FALLING );
  ERRLEDINIT(); ERRLEDOFF();
  trx.Init(cc1101);
  trx.myID= MID;
//  pciSetup(7);
  pciSetup(9);
//  DBGINFO("classtest");  DBGINFO(Transceiver::ClassTest());
}

//Main loop is called over and over again



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
           DBGINFO(" Knock ");
           if (trx.ResponseKnock(rxFrame))
             return 1;
        }
        else if (rxFrame.Welcome())
           DBGINFO(" Welcome ");
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


void loop()
{
  ERRLEDON();         delay(50);         ERRLEDOFF();
  static IMFrame frame;

  trx.StartReceive();
  /************** radio to UART ************************/
  if ( radioOut_delay<millis())
  {
    radioOut_delay = millis()+radioDelay;
    printRadio();
    radio_writeout = 0xFFFF;  //signal write complete to radio reception
    radioBufLen = 0;
  }

  /*delay(500);
  if (ReadState || WriteState)
  {
    DBGINFO(ReadState);
    DBGINFO("+");
    DBGINFO(WriteState);
    DBGINFO("++");
  }  
  ReadState=0;
  WriteState=0;
  */
  listenTimeOut=millis()+listenDelay;
   do{
     if (CheckReadState())
     {

        while (GetData()){
          trx.StartReceive();
        }
        trx.StartReceive();
     };
  }while (listenTimeOut>millis());
//  DBGINFO(millis());

  // prepare data
  generatorUart();    
//  DBGINFO(" R  (");

 
  
  if ((millis() %10) <7)
  {   
      frame.Reset();
      UartPrepareData(frame);
      trx.Send(frame);
      DBGINFO("PUSH ");
  }

  if (trx.Retry())
  {
      DBGINFO("Retry");
  }

   
  if (trx.Transmit())
  {
      DBGINFO("transmit:");  DBGINFO(millis());    DBGINFO(" ");
      DBGINFO(trx.TX_buffer.len);    DBGINFO(",");
  }
  if (millis()>knockTimeOut)
  {
    if (trx.Knock())
    {
      DBGINFO("Knock");
      knockTimeOut=knockTimeOut+knockDelay;
    }
  }

  DBGINFO(")\r\n");

}

