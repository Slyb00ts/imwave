
//

//
// HISTORY:
//
#ifndef imBufCC1101_h
#define imBufCC1101_h


#include <imframe.h>
#include <Arduino.h>
#include "imcc1101.h"
#include "imdebug.h"

#define TransceiverIdle  0
#define TransceiverRead  1
#define TransceiverWrite  2

typedef struct
{
  uint8_t len;
  IMFrame packet;
  uint16_t appended;
} transfer_t;


class  IMBuffer
{
//  private:
  public:
     IMCC1101  cc1101;  //The CC1101 device
      uint8_t tail;
      uint8_t head;
      uint8_t temp;

    volatile byte ruptures[3];
    volatile  byte state;
    unsigned short crcCheck();
    void setRssi();

   public:
    transfer_t RX_buffer ;
    transfer_t TX_buffer ;
    unsigned short rSize;
    unsigned short crc;
    unsigned short rssiH;  //from last receive frame
    void Init();
    bool Send();
    void Read();
    bool TestFrame();
    uint8_t GetData();
    bool Received();
    void StartReceive();
    void Sleep();
    bool Rupture();
    void printReceive();
    void printSend();
    void setChannel(byte channel);

} ;


#endif
