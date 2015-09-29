
//
//    FILE: imrouting.h
// VERSION: 0.1.00
// PURPOSE: routing information base

#ifndef imRouting_h
#define imRouting_h


#include <Arduino.h>
#include "imframe.h"


#define MAXTableRouting 8
class IMRouting
{
  private:
  int count;
  IMFrame tab[MAXTableRouting];
  public:
  byte retrycnt;  //counter for retries
  uint8_t addr[MAXTableRouting];
  uint32_t time[MAXTableRouting];

  byte Send(IMFrame & frame);
  byte Recive(IMFrame & frame);
  byte find();

};

#endif
