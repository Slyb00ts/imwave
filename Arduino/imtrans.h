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

#include "imcc1101.h"
#include "imframe.h"
#include "imack.h"

#define TRANSCIEVER_LIB_VERSION "0.1.00"

#define RECEIVE_TO 1000  //Wait at max this long in ms for packet arrival

//Buff for radio packet handling
// #. define HEADERSIZE 8
// #. define PAKETSIZE 61  //CC1101 adds LEN, LQI, RSSI -- stay under fifo size of 64 byte (CC1101 buggy)
// #. define MAXDATALEN PAKETSIZE-HEADERSIZE

/*
typedef struct
{
  uint8_t nwid;
  uint8_t src;
  uint8_t dest;
  uint8_t seq;
  uint8_t pseq;
  uint8_t hopc;
  uint8_t len;
  uint8_t crc;
} header_t;
*/

/*
typedef struct
{
  header_t header;
  uint8_t data[MAXDATALEN];
} packet_t;
*/

#define header_t IMFrameHeader
#define packet_t IMFrame

//Packet format delivered by the CC1101 RX
typedef struct
{
  uint8_t len;
  packet_t packet;
  uint16_t appended;
} transfer_t;


class Transceiver
{
private:
    IMCC1101 * cc1101;  //The CC1101 device
    packet_t * pPacket;
    packet_t * txPacket;
    float rssi;
    void setRssi();

public:
    header_t * pHeader;
    header_t * txHeader;

    transfer_t RX_buffer ;
    transfer_t TX_buffer ;
    TableACK  ack;
    unsigned short netID;
    unsigned short myID;
    unsigned short rSize;
    unsigned short crc;
    void Init(IMCC1101 & cc);
    void StartReceive();
    bool Valid();
    uint8_t GetData();
    float Rssi();
    unsigned short crcCheck();

    uint8_t CRC(packet_t & p);
    uint8_t GetLen(packet_t & p);
    
    unsigned char Transmit(uint8_t dst);
    uint8_t Get(uint8_t* buf);
    uint8_t Put(uint8_t*buf,uint8_t len);

private:
	int read(uint8_t pin);
    void PrepareTransmit(uint8_t dst);
};


#endif
//
// END OF FILE
//
