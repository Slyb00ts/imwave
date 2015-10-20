;//
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

Transceiver* Transceiver::ptr = 0;

Transceiver::Transceiver()
{
  ptr = this;	//the ptr points to this object
  state = 0;
  HostChannel=0;
  BroadcastChannel=2;

}

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

void Transceiver::Idle()
{
  cc1101->FlushRxFifo();
  state=TransceiverIdle;

}

bool Transceiver::CheckReadState()
{
   DBGINFO(ruptures[TransceiverRead]);
   if (ruptures[TransceiverRead]>1){
      ruptures[TransceiverRead]=0;

      return true;
   }
   return false;

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
    rSize=cc1101->ReceiveData((uint8_t*)&RX_buffer);
//    rSize=cc1101->GetData((uint8_t*)&RX_buffer);
    printReceive();
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
      frame.Header.ReceiverId=a;
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
         io =( (pHeader->ReceiverId==myId) || myId==0 || (pHeader->ReceiverId==0));
      } else {
          DBGERR("!CRC");
          return io;
      };

      if (io) {
        frame=RX_buffer.packet;
        setRssi();
      } else {
          DBGERR("Address");
          DBGERR(RX_buffer.packet.Header.ReceiverId);
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

void Transceiver::setChannel(byte channel)
{
  cc1101->SetChannel(channel);
}


bool Transceiver::Connected()
{
  return connected;
}

void Transceiver::Prepare(IMFrame & frame)
{
  frame.Header.SourceId=myId;
  byte dst=frame.Header.DestinationId;
  frame.Header.pseq = ack.Answer(dst);
  frame.Header.SenderId=myId;
  frame.Header.ReceiverId=routing.Repeater(frame.Header.DestinationId);
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
  state=TransceiverIdle;

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
  printSend();
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
   setChannel(BroadcastChannel);
   IMFrame _frame;
   IMFrameSetup setup;
   setup=EmptyIMFrameSetup;
   _frame.Reset();
   _frame.Header.Function=IMF_KNOCK;
   _frame.Header.DestinationId=0;
   _frame.Header.Sequence=ksequence++;
   setup.salt=0x77;
   setup.MAC= 0x11111;
   setup.device1= 5;
   _frame.Put(&setup);
   DBGINFO("[Knock:");
   DBGINFO(_frame.Header.Sequence);
   return Send(_frame);
}

bool Transceiver::ResponseKnock(IMFrame & frame)
{
   IMFrame _frame;
   IMFrameSetup setup;
//    frame.Get(&setup);
//   DBGINFO("#");
//   DBGERR2(setup.address,HEX);

   setup=EmptyIMFrameSetup;
   _frame.Reset();
//   _frame.Header.SourceId=myId;
   _frame.Header.Function=IMF_HELLO+IMF_REPEAT ;
   _frame.Header.DestinationId=frame.Header.SourceId;
//   _frame.Header.ReceiverId=frame.Header.SenderId;
   _frame.Header.Sequence=ksequence++;
//   _frame.Header.
   setup.salt=0x82;
   setup.MAC= myMAC;
   setup.device1= 6;
   _frame.Put(&setup);
   DBGINFO("%");
   DBGINFO(_frame.Header.Sequence);
   DBGERR2(setup.MAC,HEX);
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


bool Transceiver::Forward(IMFrame & frame)
{
   IMFrame _frame;
   _frame=frame;
   DBGINFO("FORWARD");
   _frame.Header.Function=frame.Header.Function | IMF_FORWARD;
   _frame.Header.DestinationId=frame.Header.DestinationId;
   _frame.Header.SourceId=frame.Header.SourceId;
//   _frame.Header.SenderId=myId;
//   _frame.Header.ReceiverId=hostId;
   _frame.Header.Sequence=ksequence++;
   setChannel(HostChannel);
   return Send(_frame);

}

bool Transceiver::ReceiveHello(IMFrame & frame)
{
//   IMFrameSetup setup;
   IMFrameSetup setup_recv=EmptyIMFrameSetup;
    frame.Get(&setup_recv);
    routing.addMAC(setup_recv.MAC);
    return Forward(frame);
}


bool Transceiver::ReceiveWelcome(IMFrame & frame)
{
   IMFrameSetup setup;

   setup=EmptyIMFrameSetup;
   frame.Get(&setup);
   DBGINFO("%MAC");
   DBGINFO(setup.MAC);
   if  (setup.MAC!=myMAC)
     return false;

   myId=setup.address;
   HostChannel=setup.hostchannel;
   SlaveChannel=setup.slavechannel;
   connected=1;
   DBGINFO("CONNECT%");
   return true;

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
 _f.Header.SourceId=myId;
 Send(_f);

}

void Transceiver::printReceive()
{
      DBGINFO("Receive(");
      DBGINFO(millis());
      DBGINFO("): ");
      for (unsigned short i=0;i<rSize ;i++)
      {
        DBGINFO2(((uint8_t*)&RX_buffer)[i],HEX);
        DBGWRITE(' ');
      }
      DBGINFO("-> ");
}

void Transceiver::printSend()
{
      DBGINFO("Send(");
      DBGINFO(millis());
      DBGINFO("): ");
      for (unsigned short i=0;i<TX_buffer.len ;i++)
      {
        DBGINFO2(((uint8_t*)&TX_buffer)[i],HEX);
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


void Transceiver::Rupture()
{
  ruptures[state]++;
  if (ruptures[state]>1)
  {
     ruptures[state]=0;
     if (onEvent)
       onEvent(state);
  }

}

ISR(PCINT0_vect) // handle pin change interrupt for D8 to D13 here
{
   Transceiver::ptr->Rupture();

}


//
// END OF FILE
//
