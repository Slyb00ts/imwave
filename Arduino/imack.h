
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
#include "imframe.h"

#define MAXRETRIES 2  //Try so much retransmissions

#define MAXTableACK 8
class TableACK
{
  private:
//  int count;
  IMFrame tab[MAXTableACK];
  byte lastsentseq;
  byte partnerseqnr;
  byte empty();
  byte find();
  void Accept( uint8_t Seq);

  public:
  byte retrycnt;  //counter for retries
  uint8_t addr[MAXTableACK];

  byte Send(IMFrame & frame);
  byte Receive(IMFrame & frame);

  uint8_t Answer(uint8_t Addr);
  bool noack(uint8_t Addr);
  IMFrame * toRetry();
};

#endif
