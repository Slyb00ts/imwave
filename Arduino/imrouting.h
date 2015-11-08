
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
  IMAddress WARD[MAXTableRouting];          //hop address
  IMAddress ORIGIN[MAXTableRouting];        //device address
  IMMAC MACARRAY[MAXTableRouting];
  byte CHANNEL[MAXTableRouting];           //device channel listen
  byte Send(IMFrame & frame);
//  IMAddress Forward(IMAddress addr);
  byte find(IMMAC  mac);

  public:
//  uint32_t time[MAXTableRouting];
  IMAddress myId;
  IMAddress Repeater(IMAddress addr);
  void addMAC(IMMAC mac,IMAddress ward);
  byte addAddress(IMMAC mac,IMAddress addr, byte channel);
  byte getChannel(IMAddress addr);
  void reset();

};

#endif
