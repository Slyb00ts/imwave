
//

//
// HISTORY:
//
#ifndef imBuffer_h
#define imBuffer_h


#include <imframe.h>
//#.i.nclude <Arduino.h>



typedef struct
{
  uint8_t len;
  IMFrame packet;
  uint16_t appended;
} transfer_t;


class  IMBuffer
{
  private:
      uint8_t tail;
      uint8_t head;
      uint8_t temp;

    volatile byte ruptures[3];
//    void setRssi();

   public:
    volatile  byte state;
     byte _IM;
    IMBuffer() {
      _IM=99;
    };

    transfer_t RX_buffer ;
    transfer_t TX_buffer ;
    unsigned short rSize;
    unsigned short crc;
    unsigned short rssiH;  //from last receive frame
    void Init(byte channel);
    bool Send();
    void Read();
    bool TestFrame();
    uint8_t GetData();
    bool Received();
    void StartReceive();
    void Sleep();
    void Wakeup();
    void Reboot();
    bool Rupture();
    void printReceive();
    void printSend();
    void setChannel(byte channel);
    void setFunction(funTransceiver fun);

} ;


#endif
