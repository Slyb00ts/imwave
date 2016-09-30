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


void IMBuffer::Init()
{
//  cc1101=&cc;
  bool promiscuousMode = true; //set to 'true' to sniff all packets on the same network
    radio.initialize(FREQUENCY,NODEID,NETWORKID);
    radio.promiscuous(promiscuousMode);
  radio.readAllRegs();

}
bool IMBuffer::Send()
{
  if (ruptures[TransceiverRead]){
    DBGERR("?? READ ??");
  }
  state=TransceiverWrite;
  TX_buffer.len=sizeof(TX_buffer.packet);
  if (radio.send(GATEWAYID, (const void*)(&TX_buffer.packet), sizeof(IMFrame)),false) {

//  if  (cc1101.SendData((uint8_t*)&(TX_buffer.packet),TX_buffer.len)) {

    return true;
  } else  {
//    DBGERR("! SEND");
//    DBGERR(radio.errState);
//    state=TransceiverIdle;
//    cc1101.Reinit();
    return false;
  }

}

void IMBuffer::setFunction(funTransceiver fun)
{
  radio.receivedData=fun;
}

bool IMBuffer::Received()
{
//      DBGINFO("[=]");

  bool b= radio.canRead();
///  bool b=(rSize>0);
      if (b) {
         b= TestFrame();
      }
//  rSize=0;
  if (!b){
    DBGINFO("[");
    DBGINFO(state);
    DBGINFO(":EE:");
    DBGINFO(millis());
    DBGINFO("] ");
  }

  return b;
}

void IMBuffer::Read()
{
 rssiH=radio.RSSI;
}



bool IMBuffer::TestFrame()
{
//      bool io= ((RX_buffer.len>=sizeof(IMFrameHeader)) && (RX_buffer.len<=sizeof(IMFrame)));
      rssiH=radio.RSSI;
      rSize=radio.PAYLOADLEN;
      radio.PAYLOADLEN=0;
        DBGINFO("IRQ ");
        DBGINFO(radio.IRQ2);
        DBGINFO("=");
      bool io=1;
      if (io) {
          io=(rSize==sizeof(IMFrame));
      } else {
        DBGERR("!Size");

      }
      if (io)
      {

      } else {
        DBGERR("!LEN");
        DBGERR(rSize);
        return io;
      }
//      memcpy(&(radio.DATA),&(RX_buffer.packet),sizeof(IMFrame));

            for(unsigned short i=0 ; i<(sizeof(IMFrame)) ; i++)
            {
              ((uint8_t*)&RX_buffer.packet)[i]=radio.DATA[i];
            }



      return io;

}
/*

void IMBuffer::setRssi()
{
//            crc = RX_buffer.packet.Body[_frameBodySize+1];
//            rssiH = RX_buffer.packet.Body[_frameBodySize];
//            rssiH=c;
//             DBGINFO(c);
//            DBGINFO("&");
//            rssi = c;
//            if (c&0x80) rssi -= 256;
//            rssi /= 2;
//            rssi -= 74;

}

*/


void IMBuffer::StartReceive()
{
  state=TransceiverRead;
  radio.receiveBegin();
//  radio.listenMode();
}

void IMBuffer::Sleep()
{
  radio.sleep();
  state=TransceiverSleep;
//   DBGINFO("{{");
//   DBGINFO(millisT2());
}

void IMBuffer::Wakeup()
{
  radio.idle();
  state=TransceiverIdle;
   DBGINFO("}}");
//   DBGINFO(millisT2());
}

void IMBuffer::Reboot()
{
   radio.reset();
}


void IMBuffer::setChannel(byte channel)
{
//  DBGINFO("CHN");  DBGINFO(channel);  DBGINFO("_");
//  cc1101.SetChannel(channel);
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

