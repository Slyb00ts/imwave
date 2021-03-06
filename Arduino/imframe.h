#ifndef _imFrame_h
#define _imFrame_h

#include <Arduino.h>

#define IMF_KNOCK 0x04   // broadcast
#define IMF_HELLO 0x05   // response to broadcast
#define IMF_WELCOME 0x06 // register hop
#define IMF_ACK 0x08
#define IMF_DATA 0x09
#define IMF_ORDER 0x0A
#define IMF_STATUS 0x0B //internal status
#define IMF_CONFIG 0x0C // MAC setup
#define IMF_MESSAGE 0x0D // DATA without connect
#define IMF_REPEAT 0x80
#define IMF_FORWARD 0x20
#define IMF_ORDERREP 0x8A

#define IMS_TRANSCEIVER 0x8000
#define IMS_STEERING 0x4000
#define _frameSize 32

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
  byte crc;
  byte Len;
  //        byte Retry;
  //        byte pseq;
} IMFrameHeader;

typedef struct
{
  IMMAC MAC;
  IMMAC MAC2;
  uint16_t device1;
  uint16_t device2;
  uint16_t salt;
  uint8_t address; // hop in [knock, hello]
  uint8_t rssi;
  uint8_t hostchannel;
  uint8_t slavechannel;
  uint16_t mode;

} IMFrameSetup;

typedef struct
{
  uint16_t w[11];
} IMFrameData;

typedef struct
{
  uint16_t Version;
  uint16_t intVcc;
  uint16_t intTmp;
  uint16_t DeviationPlus;
  uint16_t DeviationMinus;

} IMFrameStatus;

typedef void (*funTransceiver)(byte);
static const IMFrameSetup EmptyIMFrameSetup = {0};
#define _frameBodySize _frameSize - sizeof(IMFrameHeader)
typedef struct
{
  IMFrameHeader Header;
  byte Body[_frameBodySize];

  byte CRC()
  {
    byte c = 42;
    uint8_t *pp = (uint8_t *)&Header;
    for (unsigned short i = 1; i <= _frameSize; i++)
    {
      c += *pp;
      pp++;
      //              c+=((uint8_t*)&Header)[i];
    }
    return 0x100 - c;
    //            return c;
  }
  byte checkCRC()
  {
    return CRC() == 0;
  }

  byte Put(byte *buf, byte len)
  {
    byte i;
    Header.Len = len < _frameBodySize ? len : _frameBodySize; //length
    for (i = 0; i < Header.Len; i++)
    {
      Body[i] = buf[i];
    }
    return i;
  }
  byte Get(byte *buf, byte Len)
  {
    byte i;
    for (i = 0; i < Len; i++) //fill uart buffer
    {
      buf[i] = Body[i];
    }
    return i;
  }

  byte Put(IMFrameSetup *buf)
  {
    byte i;
    for (i = 0; i < sizeof(IMFrameSetup); i++)
    {
      Body[i] = ((byte *)buf)[i];
    }
    return i;
  }
  byte Get(IMFrameSetup *buf)
  {
    memcpy(buf, &Body, sizeof(IMFrameSetup));
    return 1;
  }
  IMFrameSetup *Setup()
  {
    return (IMFrameSetup *)&(Body);
  }

  IMFrameData *Data()
  {
    return (IMFrameData *)&(Body);
  }
  IMFrameStatus *Status()
  {
    return (IMFrameStatus *)&(Body);
  }

  void Reset()
  {
    Header.Len = 0; //length
                    //                      Header.Retry = 0;
    memset(&Header, 0, _frameSize);

    //                      for (byte i=0 ; i<_frameBodySize ; i++ )
    //                      {
    //                            Body[i] = 0;
    //                      }
  }

  bool Knock()
  {
    return (Header.Function & 0x0F) == IMF_KNOCK;
  }

  bool Hello()
  {
    return Header.Function == IMF_HELLO;
  }
  bool Welcome()
  {
    return Header.Function == IMF_WELCOME;
  }
  bool ACK()
  {
    return Header.Function == IMF_ACK;
  }
  bool CONFIG()
  {
    return Header.Function == IMF_CONFIG;
  }
  bool DATA()
  {
    return Header.Function == IMF_DATA;
  }
  bool MESSAGE()
  {
    return Header.Function == IMF_MESSAGE;
  }
  bool Order()
  {
    return Header.Function == IMF_ORDER;
  }

  bool NeedACK()
  {
    return (Header.Function & 0xC0) != 0;
  }
  bool Forward()
  {
    byte x = (Header.Function & 0xC0);
    return (x == IMF_HELLO) || (IMF_DATA);
  }
  bool Backward()
  {
    byte x = (Header.Function & 0xC0);
    return (x == IMF_WELCOME) || (IMF_ACK);
  }

  bool Repeat()
  {
    return (Header.Function & 0x80) != 0;
  }

  bool Onward()
  {
    return (Header.Function & IMF_FORWARD) != 0;
  }

} IMFrame;

#endif
