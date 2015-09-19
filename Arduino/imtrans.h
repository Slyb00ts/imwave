// 
//    FILE: dht.h
// VERSION: 0.1.00
// PURPOSE: Transiver for Arduino
//
//
// HISTORY:
//

#ifndef transceiver_h
#define transceiver_h

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include "CC1101_lib.h"

#define TRANSCIEVER_LIB_VERSION "0.1.00"

#define RECEIVE_TO 1000  //Wait at max this long in ms for packet arrival

//Buff for radio packet handling
#define HEADERSIZE 8
#define PAKETSIZE 61  //CC1101 adds LEN, LQI, RSSI -- stay under fifo size of 64 byte (CC1101 buggy)
#define MAXDATALEN PAKETSIZE-HEADERSIZE


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

typedef struct
{
  header_t header;
  uint8_t data[MAXDATALEN];
} packet_t;

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
    CC1101 * cc1101;  //The CC1101 device
    packet_t * pPacket;
    packet_t * txPacket;

public:
    header_t * pHeader;
    header_t * txHeader;

    transfer_t RX_buffer ;
    transfer_t TX_buffer ;
    unsigned short netID;
    unsigned short myID;
    float rssi;
    unsigned short rSize;
    unsigned short crc;
    void Init(CC1101 & cc);
    void StartReceive();
    bool Valid();
    uint8_t GetData();
    unsigned short crcCheck();

    float Rssi();
    uint8_t CRC(packet_t & p);
    uint8_t GetLen(packet_t & p);
    
    void PrepareTransmit(uint8_t src,uint8_t dst);
    unsigned char Transmit();
    int Get(uint8_t* buf);
    int Put(uint8_t*buf,uint8_t len);

private:
	int read(uint8_t pin);
};

#define MAXTableACK 16
class TableACK
{
  private:
  int count;
  uint8_t lastsentseq;
  uint8_t partnerseqnr;
  public:

  uint8_t addr[MAXTableACK];
  uint8_t seq[MAXTableACK];
  int Send(uint8_t Addr, uint8_t Seq);
  int Recive(uint8_t Addr, uint8_t Seq);
  
  uint8_t Answer(uint8_t Addr);
  void Accept( uint8_t Seq);
  bool noack(uint8_t Addr);
};

#endif
//
// END OF FILE
//
