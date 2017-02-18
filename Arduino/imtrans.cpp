//
//    FILE: transceiver.cpp
// VERSION: 0.6.00
// PURPOSE: DTransceiver library for Arduino
//
// DATASHEET:
//
// HISTORY:
// 0.6 by Dariusz Mazur (01/03/2016)
// 0.5 by Dariusz Mazur (11/01/2016)
// 0.4 by Dariusz Mazur (01/11/2015)
// 0.2 by Dariusz Mazur (01/09/2015)
//

#include "imtrans.h"


/////////////////////////////////////////////////////
//
// PUBLIC
//

#ifndef TimerKnockCycle
#define TimerKnockCycle 10
#define TimerHelloCycle 16
#endif

#if DBGLVL>=1
  #define TimerHelloCycle 44
#endif


#if DBGLVL>=1
  #define knockShift 40
#else
  #define knockShift 10
#endif

#define IMVERSION 11


Transceiver* Transceiver::ptr = 0;

Transceiver::Transceiver()
{
  noInterrupts();
  randomSeed(internalrandom());
  ptr = this;	//the ptr points to this object
  HostChannel=0;
  myChannel=0;
  BroadcastChannel=0;
//  _calibrate=0;
  _calibrated=false;
  _rateData=3;
  _rateHello=20;
  _calibrateshift=0;
  TimerSetupAll();
}

void Transceiver::Init(IMBuffer & buf)
{
  buffer=&buf;
//  _inSleep=true;
  power_spi_enable();
  if(!NoRadio)
    buffer->Init();
  buffer->setFunction(&timer.doneReceived);
  TimerSetup(0);
  Deconnect();
  LoadSetup();
  PrepareTransmission();
}

void Transceiver::TimerSetupAll()
{
    timer.Setup(STARTBROADCAST,BroadcastDelay);
   // timer.Setup(IMTimer::PERIOD,CycleDuration);
}

void Transceiver::TimerSetup(t_Time cal)
{
   // _calibrate=cal;
    timer.Setup(STARTDATA,DataDelay+cal-_calibrateshift);
    timer.Setup(STOPDATA,DataDelay+DataDuration+cal-_calibrateshift);
    timer.Setup(STOPBROADCAST,BroadcastDelay+BroadcastDuration+cal-_calibrateshift); //when shift knock
}


uint8_t Transceiver::GetData()
{

  if (buffer->Received())
  {

  //  ReceiveTime=buffer->RXTime();
//    DBGINFO("Receive*<");
 //   printTime();
   buffer->printReceive();
    return 1;
  } else{
 //   DBGINFO("+++++");
//    DBGINFO(buffer->state);

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



bool Transceiver::GetFrame(IMFrame& frame)
{
 // if (GetData()) {

       frame=buffer->RX_buffer.packet;
       bool io =( (frame.Header.ReceiverId==myId) ||  (frame.Header.ReceiverId==0));
       if (!io) {
          DBGERR("Address");
          DBGERR(frame.Header.ReceiverId);
      };
//       if (io){
               io= frame.checkCRC();
//       }
       if (!io) {
          DBGERR("!!CRC ");
          DBGERR(frame.CRC());
       };

        DBGINFO(" RSSI: ");           DBGINFO(Rssi());            DBGINFO("dBm  ");
    return io;
}




float Transceiver::Rssi(byte h  )
{
   float rssi = h;
   return rssi;
}
float Transceiver::Rssi()
{
   return Rssi(buffer->rssiH);
}
float Transceiver::RssiListen()
{
   return Rssi(hostRssiListen);
}
float Transceiver::RssiSend()
{
   return Rssi(hostRssiSend);
}


void Transceiver::LoadSetup()
{
  eprom.ReadConfig();
  myId=imEConfig.myId;
  hostId=imEConfig.hostId;
  serverId=imEConfig.serverId;
  myMode=imEConfig.myMode;
  myChannel=imEConfig.myChannel;
}

void Transceiver::StoreSetup(){
  imEConfig.myMode=myMode;
  imEConfig.myId=myId;
  imEConfig.myChannel=myChannel;
  imEConfig.serverId=serverId;
  imEConfig.hostId=hostId;
  eprom.WriteConfig();
}

void Transceiver::Deconnect()
{
  _connected=false;
  _salt=1;
//  myId=0;
//  hostId=0;
  _rateData=3;
  _rateHello=20;
  _cycleshift=0;
  _calibrated=false;

//  _knocked=0;
  _helloCycle=0;   //on Deconnect reset skipping
  _KnockCycle=timer.Cycle();
  myChannel=0;
  _inSleep=true;
  router.reset();
  router.addMAC(myMAC,0xFF);
//  _KnockCycle=0;
  BroadcastEnable=false;

  //_knockCycle=timer.clock()-100;
  timer.Watchdog();
  SendKnock(true);
  delaySleepT2(20); //if too short wait : error on serial yyyyy***yyyy
  DoListenBroadcast();
}

bool Transceiver::Connected()
{
  return _connected;
}


bool Transceiver::myHost(IMFrame & frame)
{
     return hostMAC==frame.Setup()->MAC;
}


void Transceiver::Prepare(IMFrame & frame)
{
  frame.Header.SenderId=myId;
  frame.Header.crc=0;
  frame.Header.crc=frame.CRC();
}


bool Transceiver::TestLow()
{
/*   IMFrame _frame;
   IMFrameSetup *setup=_frame.Setup();
   _frame.Reset();

  TX_buffer.packet=_frame;
  PrepareTransmit();
  printSend();
  state=TransceiverWrite;
  if  (cc1101->SendData((uint8_t*)&(TX_buffer.packet),TX_buffer.len)) {
    return true;
  } else  {
    DBGERR("! SEND");
    return false;
  }
  */
  return false;
}

void Transceiver::Push(IMFrame & frame)
{
//   queue.push(frame);
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


bool Transceiver::RetryData()
{
     return false;
}

void Transceiver::PrepareSetup(IMFrameSetup &se)
{
   se.MAC= myMAC;
   se.MAC2=serverMAC;
   se.salt=_salt;
//   se.device1= myDevice;
   se.hostchannel=myChannel;
}

bool Transceiver::Send(IMFrame & frame)
{
  if(NoRadio) return false;
  Prepare(frame);
  buffer->TX_buffer.packet=frame;
//  PrepareTransmit();
  DBGINFO("Send<");
//  printTime();
//  buffer->printSend();
  return buffer->Send();
}

bool Transceiver::SendData(IMFrame & frame)
{
   buffer->setChannel(HostChannel);
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
      delaySleepT2(1);
      ListenData();
}


void Transceiver::Idle()
{
   if (!_inSleep){
     _inSleep=true;
     buffer->Sleep();
     timer.setStage(IMTimer::IDDLESTAGE);
     power_spi_disable();
  //   DBGPINLOW();
     DBGINFO("idle");
   }
}

void Transceiver::Wakeup()
{
  if (_inSleep){
    _inSleep=false;
    power_spi_enable();
    buffer->Wakeup();
//    DBGPINHIGH();
    DBGINFO("wakeup");
  }
}


void Transceiver::ContinueListen()
{
     if (Connected())
        ListenData();
     else
        DoListenBroadcast();

}
void Transceiver::ListenBroadcast()
{
   if (timer.Watchdog(1200))  //1hour
   {
      DBGINFO("WATCHDOG");
      Deconnect();
      buffer->Reboot();
   }

   if (Connected()){
     if (timer.Cycle()<_helloCycle)
       return;
     if (timer.Cycle()==(_helloCycle+10)){
        SendKnock(true);
        DoListenBroadcast();
        return;
     }
   } else {
       if ((timer.Cycle() % (TimerKnockCycle))==0){
          if (_cycleshift){  //hello sended
            _cycleshift=0;
          }else{
            DBGINFO("InvalidKnock ");
            SendKnock(true);
            DoListenBroadcast();
            DBGINFO("\r\n");
            return;
          }
       }

     if (timer.Cycle()>(_KnockCycle+60))   {
         _calibrated=true;
        Idle();
         return;
     }
     if ((timer.Cycle() & 0x4) ==0)
       return;
   }
   DBGINFO("listenBroad ");
   DBGINFO(_KnockCycle);
   Wakeup();
   DoListenBroadcast();
}

void Transceiver::DoListenBroadcast()
{
   timer.setStage(LISTENBROADCAST);
   buffer->setChannel(BroadcastChannel);
   buffer->StartReceive();
}

void Transceiver::ListenData()
{
   if (BroadcastEnable){
      Wakeup();
      buffer->setChannel(myChannel);
      timer.setStage(LISTENDATA);
      buffer->StartReceive();
   } else {
      StopListen();
   }
}

void Transceiver::StopListen()
{
   if (Connected()){
      if       (timer.Cycle()>(_helloCycle+10))  //check 30s
        _calibrated=false;
      if  (_calibrated )
          Idle();
      if  (timer.Cycle()>_helloCycle+20) { //after 1min without knock
         _helloCycle=timer.Cycle()+300;  // next 15min waiting
         _calibrated=true;
      }

   } else {
     if (timer.Cycle() >(_KnockCycle+300)){   // check 15 min
        DBGINFO("StopListen");
        _KnockCycle=timer.Cycle();
        _calibrated=false;
//        Idle();
     }
   }
}

void Transceiver::StopListenBroadcast()
{
   if ( _calibrated) //check 3min
    {
     Idle();
//     timer.setStage(IMTimer::IDDLESTAGE);
   }
}


bool Transceiver::ReceiveKnock(IMFrame & frame)
{
           IMFrameSetup *sp=frame.Setup();
           if (Connected()) {
              if (myHost(frame)){
                if (sp->salt!=_salt){   //host reboot
                  // Deconnect();
                    _helloCycle=timer.Cycle();
                   DBGINFO("HOST REBOOT");
                } else {
 //                  timer.Calibrate(millis()-BroadcastDelay-100);
                }
              } else {
                        DBGINFO(" alien host ");
                        if (sp->salt==0) {    //received invalid knock
                           DBGINFO(" invalid ");
                        }
                        return false;
              }
              DBGPINHIGH();
          //     if ((timer.Cycle() %6)==0)
                timer.Calibrate(millisTNow()-BroadcastDelay-knockShift);
                _calibrated=true;
              DBGPINLOW();
             //  return false;
           }

           if (sp->salt==0) {    //received invalid knock
                      DBGINFO(" invalid ");
                      return false;
           }


           _KnockCycle=timer.Cycle();
           _salt=sp->salt; //accept new value
           hostRssiListen=buffer->rssiH;

           if (ResponseHello(frame)){
                 DoListenBroadcast();      //return to broadcas channel (wait to WELCOME)
                 return true;
           }
           StopListenBroadcast(); // no listen until data stage
           return false;
}

bool Transceiver::SendKnock(bool invalid)
{
   Wakeup();
   buffer->setChannel(BroadcastChannel);
   IMFrame _frame;
   IMFrameSetup *setup=_frame.Setup();
   _frame.Reset();
   _frame.Header.Function=IMF_KNOCK;
   _frame.Header.Sequence=ksequence++;
   _frame.Header.SourceId=myId;
   PrepareSetup(*setup);
   setup->address=myHop;
   if (invalid){
     setup->salt=0;
     setup->mode=IMVERSION;
   }
   return Send(_frame);
}


void Transceiver::Knock()
{
      if (BroadcastEnable){
          if ((timer.Cycle() % TimerKnockCycle)==0){
            DBGINFO("Knock ");
            SendKnock(false);
            DBGINFO("\r\n");
            DoListenBroadcast();
//            return;
          }
  //        ListenData();
      }
//      StopListenBroadcast();
//   } else {
/*       if ((timer.Cycle() % (TimerKnockCycle))==0){
          if (_cycleshift){  //hello sended
            _cycleshift=0;
          }else{
            DBGINFO("InvalidKnock ");
            SendKnock(true);
            DoListenBroadcast();
            DBGINFO("\r\n");
          }
       }
       */
 //  }

}


bool Transceiver::ResponseHello(IMFrame & frame)
{
   DBGINFO("((");
   DBGINFO(_helloCycle);
   DBGINFO(")) ");
//   _knocked++;
   byte xr;
   if (Connected()){
     //if (_knocked % (TimerHelloCycle*_cycledata))  {
         if (timer.Cycle()<_helloCycle) {    //last call hasn't success

           DBGINFO("notsendHello ");
           return false;
         }
     //}
     _helloCycle=timer.Cycle() +3;  //if not success bypass cycle
     xr=random(100)+60;
   } else {
     if (timer.Cycle()<_helloCycle)
       return false;
     xr=random(60);
     _helloCycle=timer.Cycle() +(xr & 1);

   }
   DBGINFO("[[");
   DBGINFO(xr);
   DBGINFO("]] ");
   delaySleepT2(xr);


   IMFrameSetup *sp=frame.Setup();
   IMFrame _frame;
   hostMAC=sp->MAC;
   serverMAC=sp->MAC2;
   myHop=sp->address;
   myHop++;
  // _cycleshift=1;
   hostId=frame.Header.SourceId;
   HostChannel=sp->hostchannel;

   buffer->setChannel(HostChannel);

   IMFrameSetup * setup=_frame.Setup();

   _frame.Reset();
   _frame.Header.SourceId=myId;   //if not registerred then myId==0
   _frame.Header.Function=IMF_HELLO ;
   _frame.Header.ReceiverId=frame.Header.SenderId;
   _frame.Header.DestinationId=frame.Header.SourceId;
   _frame.Header.Sequence=frame.Header.Sequence;
   PrepareSetup(*setup);
    setup->rssi=hostRssiListen;
    setup->mode=IMVERSION;
    setup->slavechannel=hsequence++;

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
   buffer->setChannel(router.getChannel(_frame.Header.ReceiverId));  //get proper channel form router
   return Send(_frame);
}

bool Transceiver::Forward(IMFrame & frame)
{
   IMFrame _frame;
   _frame=frame;
   DBGINFO("FORWARD");
   buffer->setChannel(HostChannel);
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
     //      if (!Connected())
     //      {
     //         DBGERR("&NOTCNT");
     //         return true;
     //      }

        if (!BroadcastEnable){

              DBGERR("&NOTBCE");
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
      frame.Setup()->rssi=buffer->rssiH;  // send RSSI to server
    }
    if (!router.addMAC(setup_recv.MAC,a)){
              DBGERR("ERRADDMAC");
              Deconnect();
              buffer->Reboot();
      ;
    }
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
      buffer->setChannel(BroadcastChannel);        // source hop - listen on broadcast
      frame.Header.ReceiverId=0;
    } else {
      frame.Header.ReceiverId=x;          //transmiter hop
      buffer->setChannel(router.getChannel(x));  // set to channel of hop
      buffer->setChannel(BroadcastChannel);
    }
    return Send(frame);
}


void Transceiver::setupMode(uint16_t aMode)
{
  BroadcastEnable=(aMode & IMS_TRANSCEIVER);
  uint8_t xCycle= aMode & 0xFF;
  if (xCycle==1) {
    _rateData=3;
  } else if (xCycle==2)   {
    _rateData=20;
  } else if (xCycle==3)   {
    _rateData=100;
  } else if (xCycle==4)   {
    _rateData=300;
  } else if (xCycle==5)   {
    _rateData=1200;
  } else {
    _rateData=1;
  }
  if (xCycle==1) {
    _rateHello=120;
  } else if (xCycle==2)   {
    _rateHello=300;
  } else if (xCycle==3)   {
    _rateHello=600;
  } else if (xCycle==4)   {
    _rateHello=1200;
  } else if (xCycle==5)   {
    _rateHello=1200;
  } else {
    _rateHello=60;
  }
}

void Transceiver::PrepareTransmission()
{
   router.myId=myId;
   HostChannel=0;
   myChannel=0;
   _calibrateshift=0;
  t_Time t=myId;
  t*=8;
  t =t %1000;
   TimerSetup(t);
//   BroadcastEnable=(setup->mode && IMS_TRANSCEIVER);
   setupMode(myMode);
}

bool Transceiver::ReceiveWelcome(IMFrame & frame)
{
   IMFrameSetup * setup =frame.Setup();


//   DBGINFO("\r\n%MAC");
//   DBGINFO(setup->MAC);
//   DBGINFO(":");
//   DBGINFO(setup->MAC2);
   if  (setup->MAC!=myMAC) {
     DBGINFO("*****NOT FORME ");
     DBGINFO(myMAC);
     BackwardWelcome(frame);
     return false;
   }
   timer.Watchdog();
   serverId=frame.Header.SourceId;
   myId=setup->address;
   myChannel=setup->slavechannel;
   myMode=setup->mode;
   hostRssiSend=setup->rssi;
   PrepareTransmission();
   if (!_connected)
      _cycleshift=(timer.Cycle()+myId) % _rateData; //only on first connection

   _connected=1;
   _helloCycle=timer.Cycle()+_rateHello;//setup next Hello
  StoreSetup();
   if (myHop==2) {
     _calibrateshift=200;
     DBGINFO("C200SHIFT");
   }
   DBGINFO(myId);
   DBGINFO("CONNECT%");
   StopListenBroadcast(); // no listen until data stage

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


bool Transceiver::Parse()
{
  static IMFrame rxFrame;

  if(GetFrame(rxFrame))
      {
        if (!ParseFrame(rxFrame))
        {
   //       DBGINFO(" rxGET ");
        } else{
          return true;
        }
   }
   return false;
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
           DBGINFO("HELLO");
           if (ForwardHello(rxFrame))
              DBGINFO(" FORW ");
        }
        else if (rxFrame.Welcome())
        {
           if (ReceiveWelcome(rxFrame))
           {
              DBGINFO(" Welcome ");
               return true;
           }

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
        buffer->StartReceive();

        return true;

}




void Transceiver::printStatus()
{
          DBGINFO(" RSSI ");
          DBGINFO(RssiListen());
          DBGINFO("  ");
          DBGINFO(RssiSend());
          DBGINFO(" dBm ");
          DBGINFO(" Deviation ");
          DBGINFO(timer.DeviationPlus);
          DBGINFO("  ");
          DBGINFO(timer.DeviationMinus);
   //       DBGINFO("  ");
    //      DBGINFO(_calibrate);
          DBGINFO(" cykl: ");
          DBGINFO(timer.Cycle());
          DBGINFO("  ");
          DBGINFO(_cycleshift);
          DBGINFO(" > ");
          DBGINFO(_rateData);

}

void Transceiver::printTime()
{
  timer.printTime();
}

short Transceiver::ClassTest()
{
/*  short x=IMQueue::ClassTest();
  if (x)
  {
     DBGERR("IMQueue");
     DBGERR(x);
     return x+100;
  }
  return 0;
  */
  return 0;
}


bool Transceiver::CycleData()
{
  return       (timer.Cycle() % _rateData) ==_cycleshift;

}

void Transceiver::printCycle(){
        DBGINFO("[[cycle:");
        DBGINFO(timer.Cycle() % _rateData);
        DBGINFO("%");
        DBGINFO(_cycleshift);
        DBGINFO("]]");

}

bool Transceiver::CycleDataPrev()
{
  return       ((timer.Cycle()+1) % _rateData) ==_cycleshift;

}

void Transceiver::DisableWatchdog()
{
    _rateData=30000;
}


void Transceiver::Rupture()
{
  if (buffer->Rupture())
     timer.doneReceived(1);
//       timer.doneListen();
}



#if defined(__AVR_ATmega32U4__)

ISR(PCINT0_vect) // handle pin change interrupt for D8 to D13 here
{
   Transceiver::ptr->Rupture();
}

#endif

//
// END OF FILE
//
