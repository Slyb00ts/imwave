//
//    FILE: transceiver.cpp
// VERSION: 0.4.00
// PURPOSE: DTransceiver library for Arduino
//
// DATASHEET: 
//
// HISTORY:
// 0.4 by Dariusz Mazur (01/11/2015)
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
  myChannel=0;
  BroadcastChannel=0;
  callibrate=300;
  Deconnect();
}

void Transceiver::Init(IMCC1101 & cc)
{
  cc1101=&cc;
  cc1101->Init();
  cc1101->StartReceive();
        pPacket = &RX_buffer.packet;
//        pHeader = &pPacket->Header;
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

/*
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
*/


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
         io =( (pPacket->Header.ReceiverId==myId) ||  (pPacket->Header.ReceiverId==0));
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
//          unsigned short cne=CRC(RX_buffer.packet);
          unsigned short cnf=RX_buffer.packet.CRC();
//          DBGINFO(cne);
//         DBGINFO('?');
//          DBGINFO(cnf);
//          DBGINFO('?');
//          DBGINFO(cnt);

          return (cnf==cnt);
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
            crc = pPacket->Body[pPacket->Header.Len+1];
            unsigned short c = pPacket->Body[pPacket->Header.Len];
            rssi = c;
            if (c&0x80) rssi -= 256;
            rssi /= 2;
            rssi -= 74;

}

void Transceiver::setChannel(byte channel)
{
  DBGINFO("CHN");
  DBGINFO(channel);
  cc1101->SetChannel(channel);
}

void Transceiver::Deconnect()
{
  connected=false;
  myId=0;
  router.reset();
  router.addMAC(myMAC,0xFF);
  DBGINFO("Deconnect");
}

bool Transceiver::Connected()
{
  return connected;
}


bool Transceiver::myHost(IMFrame & frame)
{
  if (Connected()){
//     IMFrameSetup setup;
//     frame.Get(&setup);
     return hostMAC==frame.Setup()->MAC;
  } else
     return true;
}


void Transceiver::Prepare(IMFrame & frame)
{
 // byte dst=frame.Header.DestinationId;
//  frame.Header.pseq = ack.Answer(dst);
  frame.Header.SenderId=myId;
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
/*   while (queue.pop(TX_buffer.packet))  {
     PrepareTransmit();
     if (Send())
     {
        ack.Send(TX_buffer.packet);
        io++;
     }
   }
   */
   return io;

}



void Transceiver::Push(IMFrame & frame)
{
//   queue.push(frame);
}

bool Transceiver::Retry()
{
  /*   IMFrame * pf;
     pf=ack.toRetry();
     if (pf)
     {
        Push(*pf);
        return true;
     }
     */
     return false;
}

void Transceiver::PrepareSetup(IMFrameSetup &se)
{
   se.MAC= myMAC;
   se.MAC2=serverMAC;
   se.salt=_salt;
   se.device1= 6;
   se.hostchannel=myChannel;
   se.hop=myHop;

}


bool Transceiver::SendData(IMFrame & frame)
{
   setChannel(HostChannel);
   frame.Header.Function = IMF_DATA;
   frame.Header.SourceId=myId;
   frame.Header.ReceiverId=hostId;
   frame.Header.Sequence = seqnr++;
   frame.Header.DestinationId=serverId;

   return Send(frame);

}


void Transceiver::ListenBroadcast()
{
      setChannel(BroadcastChannel);
      timer.setStage(LISTENBROADCAST);
      StartReceive();
}

void Transceiver::ListenData()
{
      setChannel(myChannel);
      timer.setStage(LISTENDATA);
      StartReceive();
}

void Transceiver::StopListen()
{
    if (Connected())
    {
       DBGINFO("stop listen");
//     trx.Idle();
//     Timer.setStage(Timer.IDDLESTAGE);
   }
}


bool Transceiver::ReceiveKnock(IMFrame & frame)
{
           if (myHost(frame)){
             timer.Calibrate(millis()+callibrate);
             if (ResponseHello(frame)){
                 ListenData();      //return to listen channel
                 DBGINFO(" rHELLO ");
                 return true;
              }

           } else {
                        DBGINFO(" alien host ");
           }
           return false;


}

bool Transceiver::SendKnock()
{
   setChannel(BroadcastChannel);
   IMFrame _frame;
   IMFrameSetup setup;
   setup=EmptyIMFrameSetup;
   _frame.Reset();
   _frame.Header.Function=IMF_KNOCK;
   _frame.Header.Sequence=ksequence++;
   _frame.Header.SourceId=myId;
   PrepareSetup(setup);
   //setup.device1= 5;
   _frame.Put(&setup);
   return Send(_frame);
}


void Transceiver::Knock()
{
   if (Connected())
   {
     if ((timer.Cycle() %5)==0)
     {
        DBGINFO("Knock ");
        SendKnock();
        DBGINFO("\r\n");
     }
     ListenData();

   } else
     ListenBroadcast();
}


bool Transceiver::ResponseHello(IMFrame & frame)
{

   IMFrameSetup *sp;
   if (sp->salt!=_salt){   //host reboot
     Deconnect();
    _salt=sp->salt;
   }


   _knocked++;
    if (Connected() && !(_knocked % 5))
     return false;

   IMFrameSetup setup;
   IMFrame _frame;
   frame.Get(&setup);
   hostMAC=setup.MAC;
   serverMAC=setup.MAC2;
   myHop=setup.hop;
   myHop++;
   hostId=frame.Header.SourceId;
   HostChannel=setup.hostchannel;

   setChannel(HostChannel);

   setup=EmptyIMFrameSetup;
   _frame.Reset();
   _frame.Header.SourceId=myId;   //if not registerred then myId==0
   _frame.Header.Function=IMF_HELLO+IMF_REPEAT ;
   _frame.Header.DestinationId=frame.Header.SourceId;
   _frame.Header.Sequence=frame.Header.Sequence;
//   setup.salt=0x82;
   PrepareSetup(setup);
   _frame.Put(&setup);
   DBGINFO("%");
   DBGINFO(_frame.Header.Sequence);
   DBGERR2(setup.MAC,HEX);
   Send(_frame);
   return true;   //changed channel
}



bool Transceiver::Backward(IMFrame & frame)
{
   IMFrame _frame;
   _frame=frame;
   DBGINFO("BACKWARD");
//          setChannel(SlaveChannel);   //get proper channel form router
   _frame.Header.Function=frame.Header.Function | IMF_FORWARD;
   _frame.Header.ReceiverId=router.Repeater(frame.Header.DestinationId);
   setChannel(router.getChannel(_frame.Header.ReceiverId));
   return Send(_frame);
}

bool Transceiver::Forward(IMFrame & frame)
{
   IMFrame _frame;
   _frame=frame;
   DBGINFO("FORWARD");
   setChannel(HostChannel);
   _frame.Header.Function=frame.Header.Function | IMF_FORWARD;
   _frame.Header.ReceiverId=hostId;
   return Send(_frame);

}

bool Transceiver::Onward(IMFrame & frame)
{
        if ((frame.Header.DestinationId==myId) || (myId==0))
        {
          return false;    //this frame is for me
        }
        else
        {

           if (frame.Header.ReceiverId==myId)
           {
             if (frame.Forward())
               return Forward(frame);
             else {
               return Backward(frame);
             }
           } else {
              DBGERR("&NOTMY");
              return true;
           }
        }

}


bool Transceiver::ForwardHello(IMFrame & frame)
{
    IMFrameSetup setup_recv;
    frame.Get(&setup_recv);
    IMAddress a=frame.Header.SenderId;
    if (frame.Onward())   // first hop
    {
      frame.Header.SourceId=myId; // server should know where send welcome
      a=0;                        // register MAC with no addres (no bypass)
    }
    router.addMAC(setup_recv.MAC,a);
    return Forward(frame);
}

bool Transceiver::BackwardWelcome(IMFrame & frame)
{
    IMFrameSetup setup_recv;
    frame.Get(&setup_recv);
    byte x=router.addAddress(setup_recv.MAC,setup_recv.address,setup_recv.slavechannel);
    if (x==0xFF)
       return false;
    setup_recv.hostchannel=myChannel;
    frame.Put(&setup_recv);
    if (x==0) {
      setChannel(BroadcastChannel);        // source hop - listen on broadcast
      frame.Header.ReceiverId=0;
    } else{
      frame.Header.ReceiverId=x;          //transmiter hop
      setChannel(router.getChannel(x));  // set to channel of hop
      setChannel(BroadcastChannel);
    }
    return Send(frame);
}



bool Transceiver::ReceiveWelcome(IMFrame & frame)
{
   IMFrameSetup setup;

   setup=EmptyIMFrameSetup;
   frame.Get(&setup);

   DBGINFO("%MAC");
   DBGINFO(setup.MAC);
   DBGINFO(":");
   DBGINFO(setup.MAC2);
   if  (setup.MAC!=myMAC) {
     DBGINFO("NOT FOR ME");
     return BackwardWelcome(frame);

   }
   serverId=frame.Header.SourceId;
   myId=setup.address;
   myChannel=setup.slavechannel;
   router.myId=myId;
   HostChannel=0;
   myChannel=0;
   connected=1;
   DBGINFO(myId);
   DBGINFO("CONNECT%");
   return true;

}




void Transceiver::ReceiveACK(IMFrame & frame)
{
 // ack.Receive(frame);
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
