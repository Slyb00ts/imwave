
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
  IMAddress Source[MAXTableRouting];
  IMAddress Destination[MAXTableRouting];

  public:
  uint32_t time[MAXTableRouting];

  byte Send(IMFrame & frame);
  IMAddress Forward(IMAddress addr);

};

#endif
