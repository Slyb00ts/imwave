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


#define RECEIVE_TO 1000  //Wait at max this long in ms for packet arrival

//Buff for radio packet handling
// #. define HEADERSIZE 8
// #. define PAKETSIZE 61  //CC1101 adds LEN, LQI, RSSI -- stay under fifo size of 64 byte (CC1101 buggy)




#define header_t IMFrameHeader
#define packet_t IMFrame
//.#.define IMCC1101 CC1101

//Packet format delivered by the CC1101 RX

#define TransceiverIddle  0
#define TransceiverRead  1
#define TransceiverWrite  2
/*
typedef struct {
    uint32_t MAC;
    uint32_t salt;
} IMFrameSetup;
*/

typedef struct
{
  uint8_t len;
  IMFrame packet;
  uint16_t appended;
} transfer_t;


class Transceiver
{
private:
    IMCC1101 * cc1101;  //The CC1101 device
    IMFrame * pPacket;
    IMQueue queue;
    IMRouting routing;
    header_t * pHeader;
    TableACK  ack;
    float rssi;
    unsigned short rSize;
    unsigned short crc;
    void setRssi();
    void Prepare(IMFrame & frame );
    unsigned short crcCheck();
    uint8_t CRC(IMFrame & p);
    bool Send();
    void PrepareTransmit();

public:
    volatile  byte state;
    byte ksequence;

    transfer_t RX_buffer ;
    transfer_t TX_buffer ;
    unsigned short myID;
    void Init(IMCC1101 & cc);
    void StartReceive();
    bool GetFrame(IMFrame&frame);
    uint8_t GetData();
    float Rssi();


    bool Transmit();
    void Push(IMFrame & frame);
    void ReceiveACK(IMFrame & frame);
    void SendACK(IMFrame & frame);
    bool Send(IMFrame & frame);

    bool Retry();
    bool Knock();
    bool ResponseKnock(IMFrame & frame);
    bool Routing(IMFrame & frame);
    void printReceive();
    static short ClassTest();

private:
};


#endif
//
// END OF FILE
//
