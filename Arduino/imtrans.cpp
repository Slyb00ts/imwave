//
//    FILE: transceiver.cpp
// VERSION: 0.1.00
// PURPOSE: DTransceiver library for Arduino
//
// DATASHEET: 
//
// HISTORY:
// 0.2 by Dariusz Mazur (01/09/2015)
//

#include "imtrans.h"


/////////////////////////////////////////////////////
//
// PUBLIC
//


void Transceiver::Init(IMCC1101 & cc)
{
  cc1101=&cc;
  cc1101->Init();
  cc1101->StartReceive();
        pPacket = &RX_buffer.packet;
        pHeader = &pPacket->Header;

}

void Transceiver::StartReceive()
{
  cc1101->StartReceive();
  state=TransceiverRead;
}

uint8_t Transceiver::GetData()
{

//  if (cc1101->GetState() == CCGOTPACKET)
  if (cc1101->RXBytes()>10)
  {
    if (cc1101->CheckReceiveFlag())
    {
       DBGINFO("G");
    }
//    DBGINFO("rx");
    rSize=cc1101->ReceiveData((uint8_t*)&RX_buffer);
//    rSize=cc1101->GetData((uint8_t*)&RX_buffer);
    return rSize;
  } else{
//    DBGINFO("[");
//    DBGINFO(cc1101-> SpiReadStatus(CC1101_MARCSTATE));
    return 0;
  }
}

bool Transceiver::Routing(IMFrame & frame)
{
    IMAddress a=routing.Forward(frame.Header.DestinationId);
    if (a!=0xFF)
    {
      frame.Header.RepeaterId=a;
      Push(frame);
      return true;
    } else{
      DBGERR("ERR Routing");
      return false;
    }

}


bool Transceiver::GetFrame(IMFrame & frame)
{
      bool io= ((RX_buffer.len>=sizeof(header_t)) && (RX_buffer.len<=sizeof(packet_t)));
      if (io)
      {
        io=crcCheck();
      } else {
        DBGERR("!LEN");
        return io;
      }
      if (io){
         io =( (pHeader->RepeaterId==myID) || myID==0 || (pHeader->RepeaterId==0));
      } else {
          DBGERR("!CRC");
          return io;
      };

      if (io) {
        frame=RX_buffer.packet;
        setRssi();
      } else {
          DBGERR("Address");
          DBGERR(RX_buffer.packet.Header.RepeaterId);
          return io;
      };

      return io;

}


unsigned short Transceiver::crcCheck()
{
          unsigned short cnt = RX_buffer.packet.Header.crc;
          RX_buffer.packet.Header.crc = 0;
          unsigned short cne=CRC(RX_buffer.packet);
//          DBGINFO(cnt);
//          DBGINFO('?');
//          DBGINFO(cne);

          return (cne==cnt);
}


uint8_t Transceiver::CRC(IMFrame & f)
{
    unsigned short c=42;
    for(unsigned short i=0 ; i<(sizeof(header_t)+f.Header.Len) ; i++)
    {
      c+=((uint8_t*)&f)[i];
    }
    return c;
 
}  

float Transceiver::Rssi()
{
  return rssi;
}


void Transceiver::setRssi()
{
            crc = pPacket->Body[pHeader->Len+1];
            unsigned short c = pPacket->Body[pHeader->Len];
            rssi = c;
            if (c&0x80) rssi -= 256;
            rssi /= 2;
            rssi -= 74;

}

bool Transceiver::Connected()
{
  return connected;
}

void Transceiver::Prepare(IMFrame & frame)
{
  frame.Header.SourceId=myID;
  byte dst=frame.Header.DestinationId;
  frame.Header.pseq = ack.Answer(dst);
  frame.Header.RepeaterId=routing.Repeater(frame.Header.DestinationId);
  frame.Header.crc=0;
  frame.Header.crc=CRC(frame);
}
void Transceiver::PrepareTransmit()
{
   Prepare(TX_buffer.packet);
   TX_buffer.len=sizeof(TX_buffer.packet);

}   

bool Transceiver::Send()
{
  state=TransceiverWrite;
  if  (cc1101->SendData((uint8_t*)&(TX_buffer.packet),TX_buffer.len))
    return true;
  else
  {
    DBGERR("! SEND");
    return false;
  }
  state=TransceiverIddle;

//  if (cc1101->StopReceive())
//    return cc1101->Transmit((uint8_t*)&(TX_buffer.packet),TX_buffer.len);
//  else{
//    DBGERR("error stopreceive");
//    return 0;
//  }
}

bool Transceiver::Send(IMFrame & frame)
{
  TX_buffer.packet=frame;
  PrepareTransmit();
  return Send();
}

bool Transceiver::Transmit()
{
   byte io=0;
   while (queue.pop(TX_buffer.packet))  {
     PrepareTransmit();
     if (Send())
     {
        ack.Send(TX_buffer.packet);
        io++;
     }
   }
   return io;
}


void Transceiver::Push(IMFrame & frame)
{
   queue.push(frame);
}

bool Transceiver::Retry()
{
     IMFrame * pf;
     pf=ack.toRetry();
     if (pf)
     {
        Push(*pf);
        return true;
     }
     return false;
}

bool Transceiver::Knock()
{
   IMFrame _frame;
   IMFrameSetup setup;
   _frame.Reset();
   _frame.Header.Function=IMF_KNOCK;
   _frame.Header.DestinationId=0;
   _frame.Header.Sequence=ksequence++;
   setup.salt=0x77;
   setup.MAC= 0x11111;
   setup.device1= 5;
   _frame.Put(&setup);
   DBGINFO("%");
   DBGINFO(_frame.Header.Sequence);
   return Send(_frame);
}

bool Transceiver::ResponseKnock(IMFrame & frame)
{
   IMFrame _frame;
   IMFrameSetup setup;
   _frame.Reset();
   _frame.Header.Function=IMF_HELLO;
   _frame.Header.DestinationId=frame.Header.SourceId;
   _frame.Header.Sequence=ksequence++;
   setup.salt=0x82;
   setup.MAC= myMAC;
   setup.device1= 6;
   _frame.Put(&setup);
   DBGINFO("%");
   DBGINFO(_frame.Header.Sequence);
   return Send(_frame);
}


/*bool Transceiver::ResponseKnock(IMFrame & frame)
{
   IMFrame _frame;
   _frame.Reset();
   IMFrameSetup *setup =(IMFrameSetup *)&_frame.Body;
   _frame.Header.Sequence=ksequence++;
   _frame.Header.DestinationId=frame.Header.SourceId;
   _frame.Header.Function=IMF_HELLO;
   _frame.Header.Len=sizeof(IMFrameSetup);

   setup->MAC1=0x77777;
   setup->salt=0xAA;
   DBGINFO("%");
   DBGINFO(_frame.Header.Sequence);
   DBGINFO("+");
   DBGINFO(_frame.Header.SourceId);

   return Send(_frame);
}

*/

bool Transceiver::ReceiveWelcome(IMFrame & frame)
{
   IMFrameSetup setup;
   frame.Get(&setup);
   DBGINFO("%");
   DBGINFO(setup.MAC);
   if  (setup.MAC!=myMAC)
     return false;

    myID=setup.address;
    connected=1;
}




void Transceiver::ReceiveACK(IMFrame & frame)
{
  ack.Receive(frame);
}

void Transceiver::SendACK(IMFrame & frame)
{
 IMFrame _f = frame;
 _f.Header.Function=IMF_ACK  ;
 _f.Header.DestinationId=frame.Header.SourceId;
 _f.Header.SourceId=myID;
 Send(_f);

}

void Transceiver::printReceive()
{
      DBGINFO("Receive(");
      DBGINFO(rSize);
      DBGINFO("): ");
      for (unsigned short i=0;i<rSize ;i++)
      {
        DBGINFO2(((uint8_t*)&RX_buffer)[i],HEX);
        DBGWRITE(' ');
      }
      DBGINFO("-> ");
}



short Transceiver::ClassTest()
{
  short x=IMQueue::ClassTest();
  if (x)
  {
     DBGERR("IMQueue");
     DBGERR(x);
     return x+100;
  }
  return 0;
}


//
// END OF FILE
//
