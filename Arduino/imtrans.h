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



//Buff for radio packet handling
// #. define PAKETSIZE 61  //CC1101 adds LEN, LQI, RSSI -- stay under fifo size of 64 byte (CC1101 buggy)




#define header_t IMFrameHeader
#define packet_t IMFrame
//.#.define IMCC1101 CC1101

//Packet format delivered by the CC1101 RX

#define TransceiverIdle  0
#define TransceiverRead  1
#define TransceiverWrite  2


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
    header_t * pHeader;
//    TableACK  ack;
    byte connected;
    byte _knocked;
    byte myHop;
    uint16_t _salt;
    byte seqnr;
    byte ksequence;
    volatile byte ruptures[2];
    float rssi;
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

public:
    Transceiver();
    volatile  byte state;

    transfer_t RX_buffer ;
    transfer_t TX_buffer ;
    IMAddress myId;
    IMAddress hostId;
    IMMAC myMAC;
    IMMAC hostMAC;
    IMMAC serverMAC;
    byte HostChannel;
    byte SlaveChannel;
    byte BroadcastChannel;
    funTransceiver onEvent;
    void Init(IMCC1101 & cc);
    friend void PCINT0_vect(void);
    void StartReceive();
    void setChannel(byte channel);
    bool GetFrame(IMFrame&frame);
    uint8_t GetData();
    float Rssi();


    bool Transmit();
    bool myHost(IMFrame & frame);
//    bool Local(IMFrame & frame);
    void ReceiveACK(IMFrame & frame);
    bool ReceiveWelcome(IMFrame & frame);
    bool ResponseHello(IMFrame & frame);
    bool ForwardHello(IMFrame & frame);
    void SendACK(IMFrame & frame);
    void Idle();
    bool Retry();
    bool Knock();
    bool SendData(IMFrame & frame);
//    bool Routing(IMFrame & frame);
    bool Connected();
    bool Onward(IMFrame & frame);
    void printReceive();
    void printSend();
    static short ClassTest();

private:
};


#endif
//
// END OF FILE
//
