#ifndef _imFrame_h
#define _imFrame_h

#include <Arduino.h>

#define _frameSize  32
// .#.define byte unsigned short

typedef struct
{
	byte Function;
	byte SourceId;
	byte DestinationId;
	byte ReapeterId;
        byte Sequence;
        byte Len;
        byte crc;
        byte pseq;
} IMFrameHeader;

#define _frameBodySize _frameSize - sizeof(IMFrameHeader)
typedef struct
{
	IMFrameHeader Header;
	byte Body[_frameBodySize];
} IMFrame;


#endif
