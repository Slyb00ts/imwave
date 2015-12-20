#ifndef _imFrame_h
#define _imFrame_h

#include <Arduino.h>


#define IMF_KNOCK     0x04        // Packet automation control
#define IMF_HELLO     0x05
#define IMF_WELCOME   0x06
#define IMF_ACK       0x08
#define IMF_DATA      0x09
#define IMF_ORDER     0x0A
#define IMF_REPEAT    0x80
#define IMF_FORWARD   0x20
#define _frameSize  32

typedef uint8_t IMAddress;
typedef uint32_t IMMAC;

typedef struct
{
	byte Function;
	IMAddress SourceId;
	IMAddress SenderId;
	IMAddress ReceiverId;
	IMAddress DestinationId;
        byte Sequence;
        byte Len;
        byte Retry;
        byte crc;
        byte pseq;
} IMFrameHeader;

typedef struct {
	IMMAC MAC;
        IMMAC MAC2;
	uint16_t device1;
        uint16_t device2;
        uint16_t salt;
        uint8_t address;
        uint8_t shift;
        uint8_t hostchannel;
        uint8_t slavechannel;
        uint8_t hop;
        uint8_t rssi;

} IMFrameSetup;

typedef struct {
      uint16_t w[15];
} IMFrameData;

static const  IMFrameSetup EmptyIMFrameSetup={0};
#define _frameBodySize _frameSize - sizeof(IMFrameHeader)
typedef struct
{
	IMFrameHeader Header;
	byte Body[_frameBodySize];

        byte CRC()
        {
            unsigned short c=42;
            for(unsigned short i=0 ; i<(sizeof(IMFrameHeader )+Header.Len) ; i++)
            {
              c+=((uint8_t*)&Header)[i];
            }
            return c;

        }


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

        byte Put(IMFrameSetup*buf)
        {
                      byte i;
                      Header.Len = sizeof(IMFrameSetup);  //length
                      for ( i=0 ; i<Header.Len ; i++ )
                      {
                            Body[i] = ((byte *)buf)[i];
                      }
                      return i;

        }
        byte Get(IMFrameSetup* buf)
        {
                      if (Header.Len!=sizeof(IMFrameSetup))
                          return 0;
                      memcpy(buf,&Body,sizeof(IMFrameSetup));
                      return 1;

        }
        IMFrameSetup * Setup()
        {
         return (IMFrameSetup *) &(Body);
        }

        IMFrameData * Data()
        {
         return (IMFrameData *) &(Body);
        }

        void Reset()
        {
                      Header.Len = 0;//length
                      Header.Retry = 0;
                      memset(&Header,0,_frameSize);

//                      for (byte i=0 ; i<_frameBodySize ; i++ )
//                      {
//                            Body[i] = 0;
//                      }

        }

        bool Knock()
        {
          return (Header.Function & 0x0F)==IMF_KNOCK;
        }

        bool Hello()
        {
          return Header.Function==IMF_HELLO;
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
        bool Forward()
        {
          byte x=(Header.Function & 0xC0);
          return (x==IMF_HELLO)||(IMF_DATA);
        }
        bool Backward()
        {
          byte x=(Header.Function & 0xC0);
          return (x==IMF_WELCOME)||(IMF_ACK);
        }

        bool Repeat()
        {
          return (Header.Function & 0x80) !=0;
        }

        bool Onward()
        {
          return (Header.Function & IMF_FORWARD) !=0;
        }

} IMFrame;

#endif
