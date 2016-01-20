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
#include "imcc1101.h"
// # include "CC1101_lib.h"
#include "imframe.h"
#include "imack.h"
#include "imqueue.h"
#include "imrouting.h"
#include "imtimer.h"



//Buff for radio packet handling
// #. define PAKETSIZE 61  //CC1101 adds LEN, LQI, RSSI -- stay under fifo size of 64 byte (CC1101 buggy)




#define header_t IMFrameHeader
#define packet_t IMFrame
//.#.define IMCC1101 CC1101

//Packet format delivered by the CC1101 RX

#define TransceiverIdle  0
#define TransceiverRead  1
#define TransceiverWrite  2


#define CycleDuration 3000
#define BroadcastDelay 200
#define BroadcastDuration 400

#define DataDelay 1200
#define DataDuration 300




typedef struct
{
  uint8_t len;
  IMFrame packet;
  uint16_t appended;
} transfer_t;


extern "C" void PCINT0_vect(void)__attribute__ ((signal)); // handle pin change interrupt for D8 to D13 here
typedef void( * funTransceiver )(byte );


class Transceiver
{
private:
    static Transceiver* ptr; //static ptr to Sleep class for the ISR
    IMCC1101 * cc1101;  //The CC1101 device
    IMFrame * pPacket;
//    IMQueue queue;
    IMRouting router;

//    header_t * pHeader;
//    TableACK  ack;
    byte connected;
    byte _knocked;
    byte _helloed;
    byte myHop;
    uint16_t _salt;
    uint16_t calibrate;
    uint16_t _cycledata;
    byte seqnr;
    byte ksequence;

    volatile byte ruptures[2];
    byte rssiH;  //from last receinve frame
    byte hostRssiSend;     //from hello
    byte hostRssiListen;  //from welcome
//    float rssi;
    unsigned short rSize;
    unsigned short crc;
    void setRssi();
    void Prepare(IMFrame & frame );
    unsigned short crcCheck();
    uint8_t CRC(IMFrame & p);
    void PrepareTransmit();
    void Rupture();
    bool Forward(IMFrame & frame);
    bool Backward(IMFrame & frame);
    bool Send(IMFrame & frame);
    bool Send();
    bool CheckReadState();
    void Deconnect();
    void Push(IMFrame & frame);
    bool BackwardWelcome(IMFrame & frame);
    void PrepareSetup(IMFrameSetup &se);
    bool SendKnock(bool invalid);
    bool myHost(IMFrame & frame);
    void StartReceive();
    void setChannel(byte channel);
    void Idle();
    bool SendQueue();
    bool RetryData();
    bool Onward(IMFrame & frame);
    bool TestFrame();
    uint8_t GetData();

public:
    Transceiver();
    volatile  byte state;
    IMTimer  timer;

    transfer_t RX_buffer ;
    transfer_t TX_buffer ;
    IMAddress myId;
    IMAddress hostId;
    IMAddress serverId;
    IMMAC myMAC;

    IMMAC hostMAC;
    IMMAC serverMAC;
    byte myDevice;
    byte HostChannel;
    byte myChannel;
    byte BroadcastChannel;
    funTransceiver onEvent;
    void Init(IMCC1101 & cc);
    friend void PCINT0_vect(void);
    bool GetFrame(IMFrame&frame);


    void Knock();
    void ListenData();
    void ListenBroadcast();
    void StopListen();
    bool ParseFrame(IMFrame & rxFrame);
    void Transmit();


//    bool Local(IMFrame & frame);
    bool ReceiveKnock(IMFrame & frame);
    void ReceiveACK(IMFrame & frame);
    bool ReceiveWelcome(IMFrame & frame);
    bool ResponseHello(IMFrame & frame);
    bool ForwardHello(IMFrame & frame);
    void SendACK(IMFrame & frame);
    bool SendData(IMFrame & frame);
//    bool Routing(IMFrame & frame);
    bool Connected();
    void printReceive();
    void printSend();
    void printStatus();
    bool CycleData();
    void TimerSetup();
    float Rssi(byte h); //compute Rssi from byte
    float Rssi(); //return last Rssi
    float RssiListen();
    float RssiSend();
    static short ClassTest();

private:
};


#endif
//
// END OF FILE
//
