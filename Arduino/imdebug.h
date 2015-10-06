//Write messages on dbgSerial
//0:none
//1:Errors
//2:Information

/***************************** Debug messages ***********************/
#ifndef imDebug_h
#define imDebug_h

#define DBGLVL 0


#include "Arduino.h"

//Write out errors on dbgSerial
#if DBGLVL>=1
  #define dbgSerial Serial
  #define dbgSerialSpeed 57600
  #define DBGERR(x) dbgSerial.print(x)
  #define DBGERR2(x,y) dbgSerial.print(x,y)
  #define DBGERRWRITE(x) dbgSerial.write(x)
#else
  #define DBGERR(x) do{}while(0)
  #define DBGERR2(x,y) do{}while(0)
  #define DBGERRWRITE(x) do{}while(0)
  #define INITDBG() do{}while(0)
#endif

//Write out information on dbgSerial
#if DBGLVL>=2
  #define DBGINFO(x) dbgSerial.print(x)
  #define DBGINFO2(x,y) dbgSerial.print(x,y)
  #define DBGWRITE(x) dbgSerial.write(x)
#else
  #define DBGINFO(x) do{}while(0)
  #define DBGINFO2(x,y) do{}while(0)
  #define DBGWRITE(x) do{}while(0)
#endif




#if DBGLVL>=1
#define INITDBG()      dbgSerial.begin(dbgSerialSpeed);\
                     dbgSerial.print("\r\n\r\nHello\r\n\r\n")


#endif


#endif

