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
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include "imdebug.h"
//#include "imcc1101.h"
#include "imframe.h"
//#include "imack.h"
//#include "imqueue.h"
#include "imrouting.h"
#include "imtimer.h"
#include "imatmega.h"
#include "imbuffer.h"








//Packet format delivered by the CC1101 RX



#define CycleDuration 3000
#define BroadcastDelay 200
#define BroadcastDuration 200

#define DataDelay 900
#define DataDuration 300






extern "C" void PCINT0_vect(void)__attribute__ ((signal)); // handle pin change interrupt for D8 to D13 here


class Transceiver
{
private:
    static Transceiver* ptr; //static ptr to Sleep class for the ISR
//    IMQueue queue;
    IMRouting router;
    IMBuffer * buffer;

//    TableACK  ack;
    byte _connected;

    int _knocked;
    int _helloed;
    long _KnockCycle;
    byte myHop;
    byte myChannel;
    IMAddress myId;
    uint16_t _salt;
    uint16_t _calibrateshift;
    uint16_t _calibrate;
    uint16_t _cycledata;
    uint16_t _cycleshift;
    byte seqnr;
    byte ksequence;

//    volatile byte ruptures[3];
    byte hostRssiSend;     //from hello
    byte hostRssiListen;  //from welcome
//    float rssi;
//    void setRssi();
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
    bool myHost(IMFrame & frame);
    void StartReceive();
//    void setChannel(byte channel);
    bool SendQueue();
    bool RetryData();
    bool Onward(IMFrame & frame);
    void TimerSetupAll();
    void TimerSetup(unsigned long cal);
    void setupMode(uint16_t aMode);

public:
    Transceiver();
    IMTimer  timer;

    IMAddress hostId;
    IMAddress serverId;
    IMMAC myMAC;

    IMMAC hostMAC;
    IMMAC serverMAC;
    byte myDevice;
    byte HostChannel;
    byte BroadcastChannel;
    bool NoRadio;
    bool BroadcastEnable;
//    funTransceiver onEvent;
//    void Init(IMCC1101 & cc);
    void Init(IMBuffer & buf);
    friend void PCINT0_vect(void);
    uint8_t GetData();
    bool GetFrame(IMFrame&frame);
    void Rupture();
    void Idle();
    bool Parse();
    void Wakeup();

    bool TestLow();
    void Deconnect();
    void Knock();
    void ListenData();
    void ListenBroadcast();
    void StopListen();
    void StopListenBroadcast();
    bool ParseFrame(IMFrame & rxFrame);
    void Transmit();


    bool ReceiveKnock(IMFrame & frame);
    void ReceiveACK(IMFrame & frame);
    bool ReceiveWelcome(IMFrame & frame);
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
    float Rssi(byte h); //compute Rssi from byte
    float Rssi(); //return last Rssi
    float RssiListen();
    float RssiSend();
    void DisableWatchdog();
    static short ClassTest();

private:
};


#endif
//
// END OF FILE
//
