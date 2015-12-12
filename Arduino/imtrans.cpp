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

#ifndef TimerKnockCycle
#define TimerKnockCycle 5
#endif

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


bool Transceiver::TestFrame()
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

      if (!io) {
          DBGERR("Address");
          DBGERR(RX_buffer.packet.Header.ReceiverId);
      };

      return io;

}

bool Transceiver::GetFrame(IMFrame& frame)
{
  if (GetData()) {
    bool io = TestFrame();
    if (io) {
        frame=RX_buffer.packet;
        setRssi();
        DBGINFO(" RSSI: ");           DBGINFO(Rssi());            DBGINFO("dBm  ");
    }
    return io;

  } else {
     return false;
  }
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
            rssiH=c;
            rssi = c;
            if (c&0x80) rssi -= 256;
            rssi /= 2;
            rssi -= 74;

}

void Transceiver::setChannel(byte channel)
{
  DBGINFO("CHN");
  DBGINFO(channel);
  DBGINFO("_");
  cc1101->SetChannel(channel);
}

void Transceiver::Deconnect()
{
  connected=false;
  _salt=1;
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
//  if (Connected()){
//     IMFrameSetup setup;
//     frame.Get(&setup);
     return hostMAC==frame.Setup()->MAC;
//  } else
//     return true;
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

bool Transceiver::SendQueue()
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

bool Transceiver::RetryData()
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
   se.device1= myDevice;
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


void Transceiver::Transmit()
{
      if (RetryData())
      {
         DBGINFO("Retry");
      }
      if (SendQueue())
      {
         DBGINFO("transmit:");  DBGINFO(millis());    DBGINFO(" ");
         DBGINFO(TX_buffer.len);    DBGINFO(",");
      }
      DBGINFO("\r\n");
      ListenData();
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
//       DBGINFO("stop listen");
//     trx.Idle();
//     Timer.setStage(Timer.IDDLESTAGE);
   }
}


bool Transceiver::ReceiveKnock(IMFrame & frame)
{
           IMFrameSetup *sp=frame.Setup();
           if (Connected()) {
              if (myHost(frame)){
                if (sp->salt!=_salt){   //host reboot
                   Deconnect();
                   DBGINFO("HOST REBBOT");
                } else {
                   timer.Calibrate(millis()-callibrate);
                }
              } else {
                        DBGINFO(" alien host ");
                        return false;
              }

           }

           if (sp->salt==0) {    //send invalid knock
                        DBGINFO(" inv ");
                      return false;
             }

//           if (myHost(frame)){
//             if (sp->salt!=_salt){   //host reboot
//                 Deconnect();
//                 DBGINFO("HOST REBBOT");

               _salt=sp->salt; //accept new value

             if (ResponseHello(frame)){
                 ListenBroadcast();      //return to broadcas channel (wait to WELCOME)
                 DBGINFO(" rHELLO ");
                 return true;
              }

           return false;


}

bool Transceiver::SendKnock(bool invalid)
{
   setChannel(BroadcastChannel);
   IMFrame _frame;
   IMFrameSetup *setup=_frame.Setup();
//   setup=EmptyIMFrameSetup;
   _frame.Reset();
   _frame.Header.Function=IMF_KNOCK;
   _frame.Header.Sequence=ksequence++;
   _frame.Header.SourceId=myId;
   PrepareSetup(*setup);
   if (invalid){
     setup->salt=0;
   }
//   _frame.Put(&setup);
   return Send(_frame);
}


void Transceiver::Knock()
{
   if (timer.Watchdog(90+TimerKnockCycle*3))
   {
      DBGINFO("WATCHDOG");
      Deconnect();
   }
   if (Connected())
   {

      if ((timer.Cycle() %TimerKnockCycle)==0){
          DBGINFO("Knock ");
          SendKnock(false);
          DBGINFO("\r\n");
          ListenData();
       }

   } else {
       if ((timer.Cycle() % 3)==0){
          DBGINFO("InvalidKnock ");
          SendKnock(true);
          DBGINFO("\r\n");
       }
       ListenBroadcast();
   }

}


bool Transceiver::ResponseHello(IMFrame & frame)
{


   timer.Watchdog();
   _knocked++;
    if (Connected() && (_knocked % 10))  {
       DBGINFO("notHL ");

     return false;
    }

   IMFrameSetup *sp=frame.Setup();
   IMFrame _frame;
   hostMAC=sp->MAC;
   serverMAC=sp->MAC2;
   myHop=sp->hop;
   myHop++;
   hostId=frame.Header.SourceId;
   HostChannel=sp->hostchannel;

   setChannel(HostChannel);

   IMFrameSetup * setup=_frame.Setup();

//   (*setup) =EmptyIMFrameSetup;
   _frame.Reset();
   _frame.Header.SourceId=myId;   //if not registerred then myId==0
   _frame.Header.Function=IMF_HELLO+IMF_REPEAT ;
   _frame.Header.ReceiverId=frame.Header.SenderId;
   _frame.Header.DestinationId=frame.Header.SourceId;
   _frame.Header.Sequence=frame.Header.Sequence;
   PrepareSetup(*setup);
    setup->rssi=rssiH;

//   _frame.Put(&setup);
   DBGINFO("%");
   DBGINFO(_frame.Header.Sequence);
   DBGERR2(setup->MAC,HEX);
   Send(_frame);
   return true;   //changed channel
}



bool Transceiver::Backward(IMFrame & frame)
{
   IMFrame _frame;
   _frame=frame;
   DBGINFO("BACKWARD");
   _frame.Header.Function=frame.Header.Function | IMF_FORWARD;
   _frame.Header.ReceiverId=router.Repeater(frame.Header.DestinationId);
   setChannel(router.getChannel(_frame.Header.ReceiverId));  //get proper channel form router
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
           if (!Connected())
           {
              DBGERR("&NOTCNT");
              return true;
           }

           if (frame.Header.ReceiverId==myId)
           {
             if (frame.Forward())
                Forward(frame);
             else {
                Backward(frame);
             }
           } else {
              DBGERR("&NOTMY");
           }
           return true;
        }

}


bool Transceiver::ForwardHello(IMFrame & frame)
{
    IMFrameSetup setup_recv;
    frame.Get(&setup_recv);
    IMAddress a=frame.Header.SenderId;
    if (frame.Onward())   // first hop
    {
      frame.Header.DestinationId=serverId;
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
    } else {
      frame.Header.ReceiverId=x;          //transmiter hop
      setChannel(router.getChannel(x));  // set to channel of hop
      setChannel(BroadcastChannel);
    }
    return Send(frame);
}



bool Transceiver::ReceiveWelcome(IMFrame & frame)
{
   IMFrameSetup * setup =frame.Setup();

//   setup=EmptyIMFrameSetup;
//   frame.Get(&setup);

   DBGINFO("\r\n%MAC");
   DBGINFO(setup->MAC);
   DBGINFO(":");
   DBGINFO(setup->MAC2);
   if  (setup->MAC!=myMAC) {
     DBGINFO("*****NOT FORME ");
     DBGINFO(myMAC);
     return BackwardWelcome(frame);
   }
   serverId=frame.Header.SourceId;
   myId=setup->address;
   myChannel=setup->slavechannel;
   router.myId=myId;
   HostChannel=0;
   myChannel=0;
   connected=1;
   callibrate=300+myId*30;
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


bool Transceiver::ParseFrame(IMFrame & rxFrame)
{
        if (rxFrame.Knock())
        {
           DBGINFO("\r\n receiveKnock ");
           if (ReceiveKnock(rxFrame))
           {
              DBGINFO(" sendHello ");
           }
           DBGINFO(" \r\n");
           return true;
        }
        else if (rxFrame.Hello())
        {
           if (ForwardHello(rxFrame))
              DBGINFO(" FORWHello ");
        }
        else if (rxFrame.Welcome())
        {
           if (ReceiveWelcome(rxFrame))
              DBGINFO(" Welcome ");

        }
        else if (Onward(rxFrame))
        {
              DBGINFO(" Onward ");
        }
        else if (rxFrame.ACK())
        {
              ReceiveACK(rxFrame);
              DBGINFO(" ACK ");
        }
        else
        {
              DBGINFO(" tue ");
              if (rxFrame.NeedACK())
                       SendACK(rxFrame);

              return false;
        }

     if (Connected())
        ListenData();
     else
        ListenBroadcast();

     return true;

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
