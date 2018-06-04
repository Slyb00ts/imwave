//
//    FILE: transceiver.cpp
// VERSION: 0.7.00
// PURPOSE: Transceiver library for Arduino
//
// DATASHEET:
//
// HISTORY:
// 0.7 by Dariusz Mazur (12/09/2017)     noSync
// 0.6 by Dariusz Mazur (01/03/2016)     imEConfig
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
  #define knockShift 30
#endif

#define IMVERSION 39

#define DBGSLEEP 0

#if DBGSLEEP>0
   #define DBGPINWAKEUP() PORTD|=(B00000010)
   #define DBGPINSLEEP()  PORTD&=~(B00000010)
#else
  #define DBGPINSLEEP(x) do{}while(0)
  #define DBGPINWAKEUP(x) do{}while(0)

#endif

Transceiver* Transceiver::ptr = 0;
t_Time lastWelcome;
t_Time lastWakeup;
Transceiver::Transceiver()
{
  noInterrupts();
  randomSeed(internalrandom());
  ptr = this;	//the ptr points to this object
  myChannel=0;
  ksequence=0;
  hsequence=0;
//  invalidSequence=0;
//  _calibrate=0;
  _calibrated=false;
  _doSleep=false;
  _rateData=1;
  _rateHello=20;
  funOrder=NULL;
  _broadcastshift=0;
  NoSleep=false;
  TimerSetupAll();
}

void Transceiver::Init(IMBuffer & buf)
{
  buffer=&buf;
//  _inSleep=true;
  power_spi_enable();
  if(!NoRadio)
    buffer->Init(myChannel);
  buffer->setFunction(&timer.doneReceived);
  TimerSetup(0);
  LoadSetup();
  hostRssiListen=31;
  Deconnect();
  LoadSetup();
  PrepareTransmission();
  _noSync=true;
  TimerSetupKnock();
  hostRssiListen=0;
}

void Transceiver::setPower(byte power)
{
   buffer->setPower(power);
}

void Transceiver::TimerSetupAll()
{
    timer.Setup(STARTBROADCAST,BroadcastDelay);
}

void Transceiver::TimerSetup(t_Time cal)
{
   // _calibrate=cal;
    timer.Setup(STARTDATA,DataDelay+cal);
    timer.Setup(STOPDATA,DataDelay+DataDuration+cal);
 //   TimerSetupKnock();
    timer.Setup(STOPBROADCAST,BroadcastDelay+BroadcastDuration); //when shift knock
}

void Transceiver::TimerSetupKnock()
{
  return ;
   t_Time cal;
   cal=0;
   if (_noSync)
     cal=50;
    timer.Setup(STARTBROADCAST,BroadcastDelay+cal);
    timer.Setup(STOPBROADCAST,BroadcastDelay+BroadcastDuration); //when shift knock
}



uint8_t Transceiver::GetData()
{
  if (buffer->Received())
  {
    buffer->printReceive();
    return 1;
  } else{
    return 0;
  }
}



bool Transceiver::GetFrame(IMFrame& frame)
{
       frame=buffer->RX_buffer.packet;
       bool io =( (frame.Header.ReceiverId==myId) ||  (frame.Header.ReceiverId==0)||  (frame.Header.DestinationId==shadowId));
       if (!io) {
          DBGERR("Address");
          DBGERR(frame.Header.ReceiverId);
      };
       if (io)
       {
               io= frame.checkCRC();
       }
       if (!io) {
          DBGERR("!!CRC ");
          DBGERR(frame.CRC());
       };
    return io;
}




void Transceiver::LoadSetup()
{
  IMEprom::ReadConfig();
  myId=imEConfig.Id;
  hostId=imEConfig.HostId;
  serverId=imEConfig.ServerId;
  myMode=imEConfig.Mode;
  uint16_t xMacLo=imEConfig.MacLo;
  if (startMAC!=0)
  {
    myMAC=(startMAC & 0xffff0000L )  | xMacLo;
  }
  if (funOrder) {
        funOrder(imEConfig.Order);
  }
}

void Transceiver::StoreSetup()
{
  imEConfig.Mode=myMode;
  imEConfig.Id=myId;
  imEConfig.ServerId=serverId;
  imEConfig.HostId=hostId;
 // imEConfig.MacLo=myMacLo;
  IMEprom::WriteConfig();
}

void Transceiver::Deconnect()
{
  _connected=false;
  _doSleep=false;
  hostMAC=0;
  _salt=1;
  _cycleshift=0;
  _calibrated=false;
  _noSync=true;
  TimerSetupKnock();

//  _helloCycle=0;   //on Deconnect reset skipping
  _KnockCycle=timer.Cycle();
  _helloCycle=_KnockCycle;
  _inSleep=true;
  tube.Reset();
  router.reset();
  router.addMAC(myMAC,0xFF);
  BroadcastEnable=false;
  timer.Watchdog();
  SendKnock(true);
  hostRssiListen=2;
  delaySleepT2(20); //if too short wait : error on serial yyyyy***yyyy
  DoListenBroadcast();
}

bool Transceiver::Connected()
{
  return _connected;
}


bool Transceiver::myHost(IMFrame & frame)
{
     return (hostId== frame.Header.SenderId)||(  hostMAC==frame.Setup()->MAC);
}


void Transceiver::Prepare(IMFrame & frame)
{
  frame.Header.SenderId=myId;
  frame.Header.crc=0;
  frame.Header.crc=frame.CRC();
}


void Transceiver::Push(IMFrame & frame)
{
//   queue.push(frame);
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
}

bool Transceiver::Send(IMFrame & frame)
{
  if(NoRadio) return false;
  if (_inSleep) return false;

  Prepare(frame);
  buffer->TX_buffer.packet=frame;
  return buffer->Send();
}

bool Transceiver::SendData(IMFrame & frame)
{
   frame.Header.Function = IMF_DATA;
   frame.Header.SourceId=myId;
   frame.Header.ReceiverId=hostId;
   frame.Header.Sequence = seqnr++;
   frame.Header.DestinationId=serverId;
   if (myId==0){
    frame.Data()->w[8]=myMAC >> 16;
    frame.Data()->w[9]=myMAC;
   }
   return Send(frame);
}


void Transceiver::Transmit()
{
      if (RetryData())
      {
         DBGINFO("Retry");
      }
//      delaySleepT2(1);
//      ListenData();//stop listen when no broadcast
   if (BroadcastEnable || NoSleep ){
      Wakeup();
      timer.setStage(LISTENDATA);
      buffer->StartReceive();
   } else {
      StopListenBroadcast();
   }
}


void Transceiver::Idle()
{
   if (NoSleep)
       return;
   if (!_inSleep)
   {
       _inSleep=true;
       buffer->Sleep();
       timer.setStage(IMTimer::IDDLESTAGE);
       power_spi_disable();
       DBGPINSLEEP();
   }
}

void Transceiver::Wakeup()
{
  if (_inSleep)
  {
    _inSleep=false;
    power_spi_enable();
    buffer->Wakeup();
    DBGPINWAKEUP();
    DBGINFO("wakeup");
  }
}


bool Transceiver::CheckListenBroadcast()
{
   if (timer.Watchdog(3600+_rateHello))  //3hours
   {
      hostRssiListen=17;
      SendKnock(true);
      hostRssiListen=98;

      Deconnect();
      buffer->Reboot();
      return true;
   }
   if (NoSleep)
   {
     return true;
   }
   if (Connected())
   {
     if (timer.Cycle()<_helloCycle)
       return false;
     if (timer.Cycle()>(_helloCycle+4)){
        _calibrated=false;
        _KnockCycle=timer.Cycle();
        byte xs=24;
        if (_doSleep)
           xs=25;
        SendStatus(xs);
          _doSleep=false;
          if (_noSync) {
            _doSleep=true;
            _helloCycle+=1200;
          }
     }
   } else {

     if (timer.Cycle()>(_KnockCycle+7))   {
         _noSync=true;
         if (!_doSleep)
         {
              _helloCycle=timer.Cycle()+1200;
              SendStatus(23);
              _doSleep=true;
              TimerSetupKnock();
          }
         _calibrated=false;
         return false;
     }
   }
   if (_noSync) return false;
   return true;
}

void Transceiver::DoListenBroadcast()
{
   timer.setStage(LISTENBROADCAST);
   buffer->StartReceive();
}


void Transceiver::StopListen()
{
   if (NoSleep) {
     return;
   }
   if (Connected()){
//      if       (timer.Cycle()>(_helloCycle+10))  //check 30s
//        _doSleep=false;
//      if  (_doSleep )
//          Idle();

      if (timer.Cycle()>_helloCycle+7) { //after 45s without knock
         _KnockCycle=timer.Cycle();
         _helloCycle=_KnockCycle+1;  // next 10min waiting
         _connected=false;
         _doSleep=false;
         hostMAC=0;
         _calibrated=false;
         _noSync=true;
         TimerSetupKnock();
     }

   } else {
     if (timer.Cycle() >(_KnockCycle+1200)){   // after  15 min    check Knock
        DBGINFO("StopListen");
        _KnockCycle=timer.Cycle();
        _helloCycle=_KnockCycle+1;

        _doSleep=false;
        _noSync=true;

     }
   }
   StopListenBroadcast();
}

void Transceiver::StopListenBroadcast()
{
   if (!NoSleep) {
   if (_connected){
     if ( _doSleep && _calibrated ) //check 3min
     {
        Idle();
        return;
     }
   };
   if (_noSync){
        Idle();
        return;
   };
   };
   Wakeup();
   DoListenBroadcast();
}


bool Transceiver::ReceiveKnock(IMFrame & frame)
{
           IMFrameSetup *sp=frame.Setup();
           if (Connected()) {
              if (myHost(frame)){
                if (sp->salt!=_salt){   //host reboot
                  // Deconnect();
                    _helloCycle=timer.Cycle();
                   // _doSleep=false;
                    _calibrated=false;
                    _noSync=true;
                    TimerSetupKnock();
                     _KnockCycle=timer.Cycle();
                     if (sp->salt==0)
                       {    //received invalid knock
                         return true;
                       }
                    _salt=sp->salt; //accept new value
                    DBGINFO("HOST REBOOT");
                } else {
                }
              } else {
                        DBGINFO(" alien host ");
                        return false;
              }
                timer.Calibrate(millisTNow()-BroadcastDelay-knockShift-_broadcastshift);
                SendStatus(77);
                lastWelcome=timer.getTime();
                int x=(lastWelcome % 100) -frame.Header.Sequence;
                if (x<0){
                  _cycleshift=x+100;
                } else{
                  _cycleshift=x;
                }
                _calibrated=true;
                hsequence-=10;
           }
           if (_noSync) {
             return true; //knock between sendHello and receiveWelcome
           }


           if (sp->salt==0) {    //received invalid knock
                      DBGINFO(" invalid ");
                      return true;
           }
           _salt=sp->salt; //accept new value
               hostRssiListen=buffer->rssiH;

     //      hostRssiListen++;   //for debug  sake
           if (ResponseHello(frame)){
                 DoListenBroadcast();      //return to broadcas channel (wait to WELCOME)
                 return true;
           }
    //       StopListenBroadcast(); // no listen until data stage
           return false;
}

bool Transceiver::SendKnock(bool invalid)
{
   Wakeup();
   IMFrame _frame;
   IMFrameSetup *setup=_frame.Setup();
   _frame.Reset();
   _frame.Header.Function=IMF_KNOCK;
   ksequence++;
   _frame.Header.Sequence=ksequence;
   _frame.Header.SourceId=myId;
   PrepareSetup(*setup);
   setup->address=myHop;
   if (invalid){
     tube.PrepareInvalid(*setup);
     setup->salt=0;
     setup->mode=myMode;
     setup->mode=(uint16_t)timer.DeviationMinus;
     setup->MAC2=hostMAC;

     setup->hostchannel=IMVERSION;
   if (!Connected())
     setup->hostchannel=0;
     setup->rssi=hostRssiListen;
     if (tube.invalidSequence>30){
        SendStatus(19);
        hostRssiListen=97;
        Deconnect();
        buffer->Reboot();
        Send(_frame);
        return Send(_frame);
     }
   }
   return Send(_frame);
}

bool Transceiver::SendStatus(uint16_t v1)
{
   Wakeup();
   IMFrame _frame;
   IMFrameSetup *setup=_frame.Setup();
   _frame.Reset();
   _frame.Header.Function=IMF_STATUS;
   tube.PrepareStatus(*setup);
   ksequence++;
   _frame.Header.Sequence=ksequence;
   _frame.Header.SourceId=myId;
   PrepareSetup(*setup);
   setup->address=myHop;
   setup->salt=0;
   setup->mode=timer.getTime()-lastWakeup;
   setup->MAC2=hostMAC;

   setup->hostchannel=IMVERSION;
   if (!Connected()) {
      setup->hostchannel=0;
   }
   setup->rssi=v1;

   return Send(_frame);
}


void Transceiver::Knock()
{
      bool bb=CheckListenBroadcast();
      if (BroadcastEnable){
          if ((timer.Cycle() % TimerKnockCycle)==0){
            DBGINFO("Knock ");
            SendKnock(false);
            bb=true;
            DBGINFO("\r\n");
          }
      } else {
         if (bb)
          hsequence+=10;
         if (_noSync){
              if (timer.Cycle()>_helloCycle){
                    SendHello();
                    bb=true;
              }
         }
      }
      if (bb) {
        lastWakeup=timer.getTime();

        Wakeup();
        DoListenBroadcast();
      }
}


bool Transceiver::ResponseHello(IMFrame & frame)
{
   IMFrameSetup *sp=frame.Setup();
   IMFrame _frame;
   hostMAC=sp->MAC;
  // serverMAC=sp->MAC2;
   myHop=sp->address;
   myHop++;
   hostId=frame.Header.SenderId;

   Wakeup();

   IMFrameSetup * setup=_frame.Setup();

   _frame.Reset();
   _frame.Header.SourceId=myId;   //if not registerred then myId==0
   _frame.Header.Function=IMF_HELLO;
   _frame.Header.ReceiverId=frame.Header.SenderId;
   _frame.Header.DestinationId=frame.Header.SourceId;
   _frame.Header.Sequence=frame.Header.Sequence;
   PrepareSetup(*setup);
    hsequence++;
   setup->rssi=hostRssiListen;
 //   setup->mode=myMode;
  //  setup->mode=Deviation();
  //  t_Time  xcc=timer.getTime()leDuration;
    setup->device2=0;
    setup->mode=lastWelcome-lastWakeup;
    setup->hostchannel=IMVERSION;
    if (!Connected())
      setup->hostchannel=0;
    setup->slavechannel=hsequence;
 //   setup->address=wsequence;//debug
  //   setup->rssi =hsequence;   //debug sake

   Send(_frame);
   return true;   //changed channel
}

void Transceiver::SendHello()
{
  IMFrame _frame;
  Wakeup();
   _frame.Reset();
   _frame.Header.SourceId=myId;   //if not registerred then myId==0
   _frame.Header.Function=IMF_HELLO;
   _frame.Header.ReceiverId=hostId;
   _frame.Header.DestinationId=serverId;
   _frame.Header.Sequence=hsequence;
  IMFrameSetup * setup=_frame.Setup();
    PrepareSetup(*setup);
    hsequence++;
   setup->rssi=hostRssiListen;
    setup->mode=myMode;
 //   setup->mode=Deviation();
    setup->mode=0xC88;
    setup->hostchannel=IMVERSION;
    if (!Connected())
      setup->hostchannel=0;
    setup->slavechannel=hsequence++;
//    setup->slavechannel=timer.SynchronizeCycle / 10;

   Send(_frame);
 //  return true;
}


bool Transceiver::Backward(IMFrame & frame)
{
   Wakeup();
   IMFrame _frame;
   _frame=frame;
   DBGINFO("BACKWARD");
   _frame.Header.Function=frame.Header.Function | IMF_FORWARD;
   _frame.Header.ReceiverId=router.Repeater(frame.Header.DestinationId);
   return Send(_frame);
}

bool Transceiver::Forward(IMFrame & frame)
{
   Wakeup();
   IMFrame _frame;
   _frame=frame;
   DBGINFO("FORWARD");
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
    if (!BroadcastEnable){

              DBGERR("&NOTBCE");
              return false;
    }
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
              hostRssiListen=18;
              SendKnock(true);
               hostRssiListen=96;

              Deconnect();
              buffer->Reboot();
    }
    return Forward(frame);
}

bool Transceiver::BackwardWelcome(IMFrame & frame)
{
    if (!BroadcastEnable){
              DBGERR("&NOTBCE");
              return false;
    }
    IMFrameSetup setup_recv;
    frame.Get(&setup_recv);
    byte x=router.addAddress(setup_recv.MAC,setup_recv.address,setup_recv.slavechannel);
    if (x==0xFF)
       return false;
  //  setup_recv.hostchannel=myChannel;
    setup_recv.MAC2=myMAC;
    frame.Put(&setup_recv);
    if (x==0) {
      frame.Header.ReceiverId=0;
    } else {
      frame.Header.ReceiverId=x;          //transmiter hop
    }
    return Send(frame);
}


void Transceiver::setupMode(uint16_t aMode)
{
  BroadcastEnable=(aMode & IMS_TRANSCEIVER)!=0;
  SteeringEnable=(aMode & IMS_STEERING)!=0;
  if (SteeringEnable)
    NoSleep=true;

  uint8_t xCycle= aMode & 0xFF;
  _noSync=false;
  _broadcastshift=0;
  if (xCycle==1) {
    _rateData=4;
  } else if (xCycle==2)   {
    _rateData=20;            //1min
  } else if (xCycle==3)   {
    _rateData=100;           //5min
  } else if (xCycle==4)   {
    _rateData=300;          //15min
  } else if (xCycle==5)   {
    _rateData=1200;
  } else {
    _rateData=1;
  }
  if (xCycle==1) {
    _rateHello=200;             //10min
  } else if (xCycle==2)   {
    _rateHello=1200*6;           //6h
    _noSync=true;
 //   _broadcastshift=-20;
  } else if (xCycle==3)   {
    _rateHello=1200*12;              //12h
    _noSync=true;
  //  _broadcastshift=-20;
  } else if (xCycle==4)   {
    _rateHello=1199*24;         //24h
//    DisableWatchdog();
 //   _broadcastshift=-20;
    _noSync=true;
  } else if (xCycle==5)   {
    _rateHello=1199*24;         //24h
 //   DisableWatchdog();
//    _broadcastshift=-20;
    _noSync=true;
  } else {
    _rateHello=100;
  }
//  if ((timer.SynchronizeCycle==0) &&(_rateHello <360))
//     _rateHello=29;                                   // cycle>1h -> no sync
  if (BroadcastEnable)_broadcastshift=100;
  //if (_noSync) _calibrated=true;
}

void Transceiver::PrepareTransmission()
{
  t_Time t=myId;
  t*=32;
  t =t %2248; //70 *32+8
   setupMode(myMode);
   TimerSetup(t);
}

bool Transceiver::ReceiveWelcome(IMFrame & frame)
{
   IMFrameSetup * setup =frame.Setup();
   if (setup->MAC!=myMAC) {
     DBGINFO("*****NOT FORME ");
     BackwardWelcome(frame);
     return false;
   }
   timer.Watchdog();
   hsequence--;
   hostRssiListen=buffer->rssiH;

   serverId=frame.Header.SourceId;
   hostId=frame.Header.SenderId;
   if (myId!=setup->address)
     _connected=0;
   myId=setup->address;
   if (myMode!=setup->mode)
     _connected=0;
   myMode=setup->mode;
  // hostMAC=setup->MAC2;

   hostRssiSend=setup->rssi;
   PrepareTransmission();
//   if (!_connected)
//      _cycleshift=(timer.Cycle()+myId) % _rateData; //only on first connection

   _connected=1;
   _doSleep=true;
   tube.OnWelcome();
   _helloCycle=timer.Cycle()+_rateHello;//setup next Hello
   if (myHop==2) {
//     _calibrateshift=200;
     DBGINFO("C200SHIFT");
   }
   DBGINFO(myId);
   DBGINFO("CONNECT%");
   StopListenBroadcast(); // no listen until data stage
   StoreSetup();
   SendStatus(66);
   StopListenBroadcast();
   return true;
}

bool Transceiver::ReceiveConfig(IMFrame & frame)
{
   IMFrameSetup * setup =frame.Setup();

   if (setup->MAC!=myMAC) {
     DBGINFO("*****NOT FORME ");
     DBGINFO(myMAC);
      return false;
   }

   uint16_t xMacLo=setup->MAC2 & 0xffff;
   myMAC=(startMAC & 0xffff0000L )  | xMacLo;
      IMFrame _frame;
       _frame.Reset();
    IMFrameSetup * _setup=_frame.Setup();


   _frame.Header.Function = IMF_HELLO;
   _frame.Header.SourceId=myId;
   _frame.Header.ReceiverId=frame.Header.SenderId;
   _frame.Header.DestinationId=frame.Header.SourceId;
   _frame.Header.Sequence=frame.Header.Sequence;
   _setup->MAC= myMAC;
   _setup->MAC2=serverMAC;


   Send(_frame);
   myId=0;
   myMode=0;
   imEConfig.MacLo=xMacLo;
   hostRssiListen=32;
   StoreSetup();
   Deconnect();
   return true;
}

bool Transceiver::forMe(IMFrame & frame){
  return (  myMAC==frame.Setup()->MAC);
}


bool Transceiver::ReceiveOrder(IMFrame & frame)
{
    if (!forMe(frame)){
          return false;
   }
   IMFrameSetup * setup =frame.Setup();

   byte io;
   byte xOrder;
   xOrder=setup->device1;
   if (funOrder) {
      io=funOrder(xOrder);
   } else{
      io=255;
   }
    Wakeup();
      IMFrame _frame;
        IMFrameSetup * _setup=_frame.Setup();

   _frame.Reset();
   _frame.Header.Function = IMF_ORDERREP;
   _frame.Header.SourceId=myId;
   _frame.Header.ReceiverId=frame.Header.SenderId;
   _frame.Header.DestinationId=frame.Header.SourceId;
   _frame.Header.Sequence=frame.Header.Sequence;
    PrepareSetup(*_setup);
   _setup->MAC= myMAC;
   _setup->MAC2=serverMAC;
   _setup->device2=io;
   _setup->device1=xOrder;


   Send(_frame);
   imEConfig.Order=xOrder;
   StoreSetup();
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
   if (GetFrame(rxFrame)) {
        return ParseFrame(rxFrame);
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
              //return true;         //send hello and listening
           }  else{
             // hostRssiListen=81;
             // SendKnock(true);
            //  return true;
           }
        }
        else if (rxFrame.Hello())
        {
           ForwardHello(rxFrame);
        }
        else if (rxFrame.Welcome())
        {
           ReceiveWelcome(rxFrame);
        }
        else if (rxFrame.Order())
        {
           ReceiveOrder(rxFrame);
        }
        else if (rxFrame.CONFIG())
        {
           ReceiveConfig(rxFrame);
        }
        else if (Onward(rxFrame))
        {
              DBGINFO(" Onward ");    //dont stop listening
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
              return true;
        }
        return false;
}

bool Transceiver::ContinueListen(){
      if (CheckListenBroadcast()) {
        Wakeup();
        buffer->StartReceive();

        return true;
      }
      StopListenBroadcast();
      return false;
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
  return       ((timer.Cycle()+myId+_cycleshift) % _rateData) ==0;
}

bool Transceiver::CycleDataPrev()
{
  return       ((timer.Cycle()+myId+_cycleshift+1) % _rateData) ==0;
}

void Transceiver::DisableWatchdog()
{
 //   _rateData=30000;
}


void Transceiver::Rupture()
{
  if (buffer->Rupture())
     timer.doneReceived(1);
//       timer.doneListen();
}

uint16_t Transceiver::Deviation()
{
  return timer.DeviationPlus;
}

void Transceiver::setTimerFunction(funStepTimer fun)
{
  timer.funStep=fun;
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
