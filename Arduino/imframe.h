#ifndef _imFrame_h
#define _imFrame_h

#include <Arduino.h>

#define _frameSize  32

typedef uint8_t IMAddress;

typedef struct
{
	byte Function;
	IMAddress SourceId;
	IMAddress DestinationId;
	IMAddress RepeaterId;
        byte Sequence;
        byte Len;
        byte Retry;
        byte crc;
        byte pseq;
} IMFrameHeader;

#define _frameBodySize _frameSize - sizeof(IMFrameHeader)
typedef struct
{
	IMFrameHeader Header;
	byte Body[_frameBodySize];

byte Put(byte*buf,uint8_t len)
{
              byte i;
              Header.Len = len<_frameBodySize ? len : _frameBodySize;  //length
              for ( i=0 ; i<Header.Len ; i++ )
              {
                    Body[i] = buf[i];
              }
              return i;

}

} IMFrame;

#endif
