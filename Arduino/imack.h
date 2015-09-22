
//
//    FILE: imqueue.h
// VERSION: 0.1.00
// PURPOSE: Ack array for Arduino

//
// HISTORY:
//
#ifndef imACK_h
#define imACK_h


#include <Arduino.h>

#define MAXRETRIES 2  //Try so much retransmissions

#define MAXTableACK 16
class TableACK
{
  private:
//  int count;
  byte lastsentseq;
  byte partnerseqnr;
  public:
  byte retrycnt;  //counter for retries
  uint8_t addr[MAXTableACK];
  uint8_t seq[MAXTableACK];
  byte Send(uint8_t Addr, uint8_t Seq);
  byte Recive(uint8_t Addr, uint8_t Seq);

  uint8_t Answer(uint8_t Addr);
  void Accept( uint8_t Seq);
  bool noack(uint8_t Addr);
  bool retry();
};

#endif
