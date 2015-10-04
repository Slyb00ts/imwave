#ifndef _imFrame_h
#define _imFrame_h

#include <Arduino.h>


#define IMF_KNOCK     0x04        // Packet automation control
#define IMF_HELLO     0x05
#define IMF_WELCOME   0x06
#define IMF_ACK       0x07
#define IMF_DATA      0x10
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

        byte Put(byte*buf,byte len)
        {
                      byte i;
                      Header.Len = len<_frameBodySize ? len : _frameBodySize;  //length
                      for ( i=0 ; i<Header.Len ; i++ )
                      {
                            Body[i] = buf[i];
                      }
                      return i;

        }
        byte Get(byte* buf)
        {
                      byte i;
                      for(i=0; i<Header.Len  ;i++)  //fill uart buffer
                      {
                        buf[i] = Body[i];
                      }
                      return i;

        }
        void Reset()
        {
                      Header.Len = 0;//length
                      Header.Retry = 0;
                      for (byte i=0 ; i<_frameBodySize ; i++ )
                      {
                            Body[i] = 0;
                      }

        }

        bool Knock()
        {
          return Header.Function==IMF_KNOCK;
        }

        bool Welcome()
        {
          return Header.Function==IMF_WELCOME;
        }
        bool ACK()
        {
          return Header.Function==IMF_ACK;
        }
        bool NeedACK()
        {
          return (Header.Function & 0xC0) !=0;
        }
        bool Repeat()
        {
          return (Header.Function & 0x80) !=0;
        }

        IMAddress Destination()
        {
          return Header.DestinationId;
        }

} IMFrame;

#endif
