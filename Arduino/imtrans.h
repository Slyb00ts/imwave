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

//Packet format delivered by the CC1101 RX
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
    packet_t * txPacket;
    IMQueue queue;
    IMRouting routing;
    header_t * pHeader;
    header_t * txHeader;
    float rssi;
    unsigned short rSize;
    unsigned short crc;
    void setRssi();
    void Prepare(IMFrame & frame );
    unsigned short netID;
    bool Routing(IMFrame & frame);
    bool broadcast();
    unsigned short crcCheck();

public:
    transfer_t RX_buffer ;
    transfer_t TX_buffer ;
    TableACK  ack;
    unsigned short myID;
    void Init(IMCC1101 & cc);
    void StartReceive();
    bool Valid();
    uint8_t GetData();
    float Rssi();

    uint8_t CRC(packet_t & p);
    uint8_t GetLen(IMFrame & p);
    
    byte Transmit();
    uint8_t Get(uint8_t* buf);
    uint8_t Put(uint8_t*buf,uint8_t len);
    void Push(IMFrame & frame);
    bool Retry();
    bool Routing();
    void printReceive();


private:
	int read(uint8_t pin);
    void PrepareTransmit();
};


#endif
//
// END OF FILE
//
