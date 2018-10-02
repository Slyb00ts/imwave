#include <imbufrfm69.h>
#include <imrfm69.h>
#include <imrfmregisters.h>
#include "imdebug.h"


RFM69  radio;  //The CC1101 device

#define NODEID        1    //unique for each node on same network
#define NETWORKID     100  //the same on all nodes that talk to each other
#define GATEWAYID   1

//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
//#define FREQUENCY     RF69_433MHZ
#define FREQUENCY     RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
//#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
//#define ENABLE_ATC    //comment out this line to disable AUTO TRANSMISSION CONTROL


void IMBuffer::Init(byte channel)
{
  bool promiscuousMode = true; //set to 'true' to sniff all packets on the same network
    radio.initialize(FREQUENCY,NODEID,NETWORKID);
    radio.promiscuous(promiscuousMode);
    radio.setChannel(channel);
}

bool IMBuffer::Send()
{
  if (ruptures[TransceiverRead]){
    DBGERR("?? READ ??");
  }
  state=TransceiverWrite;
  TX_buffer.len=sizeof(TX_buffer.packet);
  TX_buffer.packet.Header.crc=0;
  TX_buffer.packet.Header.crc=TX_buffer.packet.CRC();

  if (radio.send( (const void*)(&TX_buffer.packet), sizeof(IMFrame))) {

//  if  (cc1101.SendData((uint8_t*)&(TX_buffer.packet),TX_buffer.len)) {

    return true;
  } else  {
//    DBGERR(radio.errState);
    return false;
  }
}

void IMBuffer::setFunction(funTransceiver fun)
{
  radio.receivedData=fun;
}

bool IMBuffer::Received()
{
  bool b= radio.canRead();
      if (b) {
         b= TestFrame();
      }
  return b;
}

void IMBuffer::Read()
{
 rssiH=radio.RSSI;
}



bool IMBuffer::TestFrame()
{
      rssiH=radio.RSSI;
      rSize= sizeof(IMFrame);
         DBGINFO("IRQ ");
        DBGINFO(radio.IRQ2);
        DBGINFO("=");
      bool io=1;
      if (io) {
      } else {

      }
//      memcpy(&(radio.DATA),&(RX_buffer.packet),sizeof(IMFrame));
     uint8_t xx=(radio._tail  &0x03)* sizeof(IMFrame);
          radio._tail++;
            for(unsigned short i=0 ; i<(sizeof(IMFrame)) ; i++)
            {
              ((uint8_t*)&RX_buffer.packet)[i]=radio.DATA[xx++];
            }
      return io;
}


void IMBuffer::StartReceive()
{
  state=TransceiverRead;
  radio.receiveBegin();
}

void IMBuffer::Sleep()
{
  radio.sleep();
  state=TransceiverSleep;
}

void IMBuffer::Wakeup()
{
  radio.idle();
  state=TransceiverIdle;
}

void IMBuffer::Reboot()
{
   radio.reset();
}


void IMBuffer::setChannel(byte channel)
{
//  DBGINFO("CHN");  DBGINFO(channel);  DBGINFO("_");
}

void IMBuffer::setPower(byte power)
{
  if (power>1)

    radio.setPowerLevel(power);
  else
     radio.setHighPower(false);
}


bool IMBuffer::Rupture()
{
  ruptures[state]++;
  if (ruptures[state]>1)
  {
     ruptures[state]=0;

     if (state!=TransceiverWrite)
     {
//       Read();
       return true;
     } else {
       state=TransceiverIdle;
     }

  }
  return false;
}



void IMBuffer::printReceive()
{
      DBGINFO("[[");
      DBGINFO(RX_buffer.len);
      DBGINFO(":");
      for (unsigned short i=0;i<=sizeof(IMFrame)+2  ;i++)
      {
        DBGINFO2(((uint8_t*)&RX_buffer)[i],HEX);
        DBGWRITE(" ");
      }
        DBGWRITE(":");
//      DBGINFO(RX_buffer.packet.CRC());
      DBGINFO(RX_buffer.appended);

      DBGINFO("-> ");
}

void IMBuffer::printSend()
{
      for (unsigned short i=0;i<=sizeof(IMFrame) ;i++)
      {
        DBGINFO2(((uint8_t*)&TX_buffer)[i],HEX);
        DBGWRITE(" ");
      }
}
