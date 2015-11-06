
//
//    FILE: imrouting.h
// VERSION: 0.1.00
// PURPOSE: routing information base

#ifndef imRouting_h
#define imRouting_h


#include <Arduino.h>
#include "imframe.h"


#define MAXTableRouting 127
class IMRouting
{
  private:
  int count;
  IMAddress BYPASS[MAXTableRouting];
  IMAddress ORIGIN[MAXTableRouting];
  IMMAC MMAC [MAXTableRouting];
  byte Send(IMFrame & frame);
  IMAddress Forward(IMAddress addr);
  byte find(IMMAC  mac);

  public:
//  uint32_t time[MAXTableRouting];

  IMAddress Repeater(IMAddress addr);
  void addMAC(IMMAC mac,IMAddress bypass);
  void addAddress(IMMAC mac,IMAddress addr);
  void reset();

};

#endif
