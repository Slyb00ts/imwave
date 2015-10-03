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
//        txPacket = &TX_buffer.packet;
//        txHeader = &txPacket->Header;

}

void Transceiver::StartReceive()
{
  cc1101->StartReceive();
}

uint8_t Transceiver::GetData()
{

  if (cc1101->GetState() == CCGOTPACKET)
//  if (1)
  {
    DBGINFO("G");
//    rSize=cc1101->ReceiveData((uint8_t*)&RX_buffer);
    rSize=cc1101->GetData((uint8_t*)&RX_buffer);
    return rSize;
  } else{
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
        DBGINFO("!LEN");
        return io;
      }
      if (io){
         io =( (pHeader->RepeaterId==myID));
      } else {
          DBGERR("!CRC");
          return io;
      };

      if (io) {
        frame=RX_buffer.packet;
        setRssi();
      } else {
          DBGERR("Address");
          return io;
      };

      return io;

}


unsigned short Transceiver::crcCheck()
{
          unsigned short cnt = pHeader->crc;
          pHeader->crc = 0;
          //valid packet crc

          return (CRC(RX_buffer.packet)-cnt);

}

uint8_t Transceiver::GetLen(packet_t & p)
{
  return (sizeof(header_t)+p.Header.Len);
}

uint8_t Transceiver::CRC(packet_t & p)
{
    unsigned short c=42;
    for(unsigned short i=0 ; i<(sizeof(header_t)+p.Header.Len) ; i++)
    {
      c+=((uint8_t*)&p)[i];
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

void Transceiver::Prepare(IMFrame & frame)
{
  frame.Header.SourceId=myID;
  frame.Header.crc=0;
  frame.Header.crc=CRC(frame);
//  frame.Header.DestinationId=dst;
}
void Transceiver::PrepareTransmit()
{
//   TX_buffer.packet.Header.SourceId=myID;
//   sizeof(header_t)+txHeader->len;

   TX_buffer.len=GetLen(TX_buffer.packet);
   byte dst=TX_buffer.packet.Header.DestinationId;
   TX_buffer.packet.Header.pseq = ack.Answer(dst);

}   

byte Transceiver::Send()
{
//  return (cc1101->SendData((uint8_t*)&(TX_buffer.packet),TX_buffer.len)) ;
  if (cc1101->StopReceive())
    return cc1101->Transmit((uint8_t*)&(TX_buffer.packet),TX_buffer.len);
  else{
    DBGERR("error stopreceive");
    return 0;
  }
}

byte Transceiver::Send(IMFrame & frame)
{
  TX_buffer.packet=frame;
  PrepareTransmit();
  return Send();

}
byte Transceiver::Transmit()
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

void Transceiver::ReceiveACK(IMFrame & frame)
{
  ack.Receive(frame);
}

void Transceiver::SendACK(IMFrame & frame)
{
 IMFrame f =frame;
 f.Header.Function=IMF_ACK  ;
 f.Header.DestinationId=frame.Header.SourceId;
 f.Header.SourceId=myID;

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
