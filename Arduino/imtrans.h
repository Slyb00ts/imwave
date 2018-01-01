//
//    FILE: dht.h
// VERSION: 0.1.00
// PURPOSE: Transiver for Arduino
//
//
// HISTORY:
//

#ifndef imTransceiver_h
#define imTransceiver_h

#if ARDUINO >= 100
// #include "Arduino.h"
#else
// #include "WProgram.h"
#endif

#include "imdebug.h"
//#include "imcc1101.h"
#include "imframe.h"
//#include "imack.h"
//#include "imqueue.h"
#include "imrouting.h"
#include "imtube.h"
#include "imtimer.h"
#include "imatmega.h"
#include "imbuffer.h"
#include "imeprom.h"








//Packet format delivered by the CC1101 RX




#define BroadcastDelay 200
//#define BroadcastDuration 100

#define DataDelay 600
#define DataDuration 200
#if DBGLVL>=1
  #define BroadcastDuration 200
#else
  #define BroadcastDuration 150
#endif







extern "C" void PCINT0_vect(void)__attribute__ ((signal)); // handle pin change interrupt for D8 to D13 here
typedef byte( * funOrderTransceiver )(uint16_t );


class Transceiver
{
private:
    static Transceiver* ptr; //static ptr to Sleep class for the ISR
//    IMQueue queue;
    IMRouting router;
    IMBuffer * buffer;
    IMTube tube;

//    TableACK  ack;
    byte _connected;
    bool _calibrated;
    bool _inSleep;
    bool _doSleep;
    bool _noSync;

    long _helloCycle;
    long _KnockCycle;
    byte myHop;
    IMAddress myId;
    uint16_t myMode;
  //  uint16_t mySynchronize;
    uint16_t _salt;
  //  uint16_t _calibrateshift;
    uint16_t _broadcastshift;
    long _rateData;
    long _rateHello;
    uint16_t _cycleshift;
    byte seqnr;
    byte ksequence;
    byte hsequence;

//    volatile byte ruptures[3];
    byte hostRssiSend;     //from hello
    byte hostRssiListen;  //from welcome
    void Prepare(IMFrame & frame );
//    void PrepareTransmit();
    bool Forward(IMFrame & frame);
    bool Backward(IMFrame & frame);
    bool Send(IMFrame & frame);
//    bool Send();
    void Push(IMFrame & frame);
    bool BackwardWelcome(IMFrame & frame);
    void PrepareSetup(IMFrameSetup &se);
    bool SendKnock(bool invalid);
    void SendHello();
    bool myHost(IMFrame & frame);
    void StartReceive();
//    void setChannel(byte channel);
//    bool SendQueue();
    bool RetryData();
    bool Onward(IMFrame & frame);
    void TimerSetupAll();
    void TimerSetup(t_Time cal);
    void setupMode(uint16_t aMode);
//    void ContinueListen();
    void DoListenBroadcast();
    bool CheckListenBroadcast();
    void Idle();
    void LoadSetup();
    void StoreSetup();
    void PrepareTransmission();
public:
    Transceiver();
    funOrderTransceiver funOrder;
    IMTimer  timer;

    IMAddress hostId;
    IMAddress serverId;
    IMMAC startMAC;
    IMMAC myMAC;

    IMMAC hostMAC;
    IMMAC serverMAC;
 //   uint16_t myMacLo;
    byte myChannel;
    byte myDevice;
 //   byte HostChannel;
 //   byte BroadcastChannel;
    bool NoRadio;
    bool NoSleep;
    bool BroadcastEnable;
    bool SteeringEnable;
    void setTimerFunction(funStepTimer fun);
//    funTransceiver onEvent;
//    void Init(IMCC1101 & cc);
    void Init(IMBuffer & buf);
    friend void PCINT0_vect(void);
    uint8_t GetData();
    bool GetFrame(IMFrame&frame);
    void Rupture();
    bool Parse();
    void Wakeup();

//    bool TestLow();
    void Deconnect();
    void Knock();
    bool ContinueListen();
   // void ListenData();
    void ListenBroadcast();
    void StopListen();
    void StopListenBroadcast();
    bool ParseFrame(IMFrame & rxFrame);
    void Transmit();
    void setPower(byte power);
    uint16_t Deviation();


    bool ReceiveKnock(IMFrame & frame);
    void ReceiveACK(IMFrame & frame);
    bool ReceiveWelcome(IMFrame & frame);
    bool ReceiveConfig(IMFrame & frame);
    bool ReceiveOrder(IMFrame & frame);
    bool ResponseHello(IMFrame & frame);
    bool ForwardHello(IMFrame & frame);
    void SendACK(IMFrame & frame);
    bool SendData(IMFrame & frame);
//    bool Routing(IMFrame & frame);
    bool Connected();
    void printStatus();
    void printTime();
    void printCycle();
    bool CycleData();
    bool CycleDataPrev();
//    float Rssi(byte h); //compute Rssi from byte
//    float Rssi(); //return last Rssi
    void DisableWatchdog();
    static short ClassTest();

private:
};


#endif
//
// END OF FILE
//
