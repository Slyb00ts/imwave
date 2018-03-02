//Write messages on dbgSerial
//0:none
//1:Errors
//2:Information

/***************************** Debug messages ***********************/
#ifndef imDebug_h
#define imDebug_h

#define DBGLVL 0
#define DBGLED 9
#define DBGPIN 0
#define DBGCLOCK 6


#ifndef DBGLED
  #define DBGLED 1
#endif

#define ERRLEDNO 13

#include "Arduino.h"


//Used for error signaling (ON when restransmitting, OFF on receive success)
#define ERRLEDINIT() pinMode(ERRLEDNO, OUTPUT)
#if DBGLED>=1
  #define DBGLEDON() digitalWrite(DBGLED,HIGH)
  #define DBGLEDOFF() digitalWrite(DBGLED,LOW)
  #define ERRFLASH() ERRLEDON(); delay(50);  ERRLEDOFF();
#else
  #define ERRFLASHA(x) do{}while(0)
  #define DBGLEDON(x) do{}while(0)
  #define DBGLEDOFF(x) do{}while(0)
#endif

#if DBGPIN>0
   #define DBGPINHIGH() PORTD|=(B00100000)
   #define DBGPINLOW()  PORTD&=~(B00100000)
#else
  #define DBGPINHIGH(x) do{}while(0)
  #define DBGPINLOW(x) do{}while(0)

#endif


 //  #define DBGPINHIGH() PORTD|=(B00100000);//digitalWrite(DBGPIN,HIGH)
//   #define DBGPINLOW()  PORTD&=(B00100000);~//digitalWrite(DBGPIN,LOW)


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
#define INITDBG()    delay(2000);\
                     dbgSerial.begin(dbgSerialSpeed);\
                     dbgSerial.print("\r\n\r\nHello\r\n\r\n")


#endif


#endif

