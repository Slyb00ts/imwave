#include <imbufcc1101.h>


IMCC1101  cc1101;  //The CC1101 device

void IMBuffer::Init()
{
//  cc1101=&cc;
  cc1101.Init();
  cc1101.StartReceive();
}
bool IMBuffer::Send()
{
  if (ruptures[TransceiverRead]){
    DBGERR("?? READ ??");
  }
  state=TransceiverWrite;
  TX_buffer.len=sizeof(TX_buffer.packet);
  if  (cc1101.SendData((uint8_t*)&(TX_buffer.packet),TX_buffer.len)) {
//    while (state==TransceiverWrite)	// Wait for GDO0 to be cleared -> end of packet
//       DBGINFO("?");
//       DBGINFO(state);

    return true;
  } else  {
    DBGERR("! SEND");
    DBGERR(cc1101.errState);
    state=TransceiverIdle;
    cc1101.Reinit();
    return false;
  }

}



bool IMBuffer::Received()
{
  bool b=(rSize>0);
      if (b) {
         b= TestFrame();
      }
  rSize=0;
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
  rSize=cc1101.ReceiveData((uint8_t*)&RX_buffer);
  crc = RX_buffer.packet.Body[_frameBodySize+1];
  rssiH = RX_buffer.packet.Body[_frameBodySize];

//  setRssi();
}


bool IMBuffer::TestFrame()
{
      bool io= ((RX_buffer.len>=sizeof(IMFrameHeader)) && (RX_buffer.len<=sizeof(IMFrame)));
      if (io) {
          io=(rSize==sizeof(IMFrame)+3);
      } else {
        DBGERR("!Size");

      }
      if (io)
      {
//        io=crcCheck();
        return io;
      } else {
        DBGERR("!LEN");
        DBGERR(rSize);
        return io;
      }
      if (!io){
//         io =( (RX_buffer.packet.Header.ReceiverId==myId) ||  (RX_buffer.packet.Header.ReceiverId==0));
//      } else {
          DBGERR("!CRC");
          return io;
      };



      return io;

}

/*
unsigned short IMBuffer::crcCheck()
{
         return RX_buffer.packet.checkCRC();
}

*/

/*
void IMBuffer::setRssi()
{
            crc = RX_buffer.packet.Body[_frameBodySize+1];
            rssiH = RX_buffer.packet.Body[_frameBodySize];
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
  cc1101.StartReceive();
  state=TransceiverRead;
}


void IMBuffer::Sleep()
{
  cc1101.Sleep();
  state=TransceiverIdle;

}

void IMBuffer::setChannel(byte channel)
{
//  DBGINFO("CHN");  DBGINFO(channel);  DBGINFO("_");
  cc1101.SetChannel(channel);
}



bool IMBuffer::Rupture()
{
  ruptures[state]++;
  if (ruptures[state]>1)
  {
     ruptures[state]=0;

     if (state!=TransceiverWrite)
     {
       Read();
       return true;
     } else {
       state=TransceiverIdle;
     }

  }
  return false;
}



void IMBuffer::printReceive()
{
      for (unsigned short i=0;i<RX_buffer.len ;i++)
      {
        DBGINFO2(((uint8_t*)&RX_buffer)[i],HEX);
        DBGWRITE(" ");
      }
      DBGINFO("-> ");
}

void IMBuffer::printSend()
{
      for (unsigned short i=0;i<TX_buffer.len ;i++)
      {
        DBGINFO2(((uint8_t*)&TX_buffer)[i],HEX);
        DBGWRITE(' ');
      }
}

