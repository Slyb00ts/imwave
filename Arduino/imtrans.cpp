//
//    FILE: transceiver.cpp
// VERSION: 0.1.00
// PURPOSE: DTransceiver library for Arduino
//
// DATASHEET: 
//
// HISTORY:
// 0.2 by Dariusz Mazur (01/09/201)
// inspired by DHT11 library
//

#include "imtrans.h"


/////////////////////////////////////////////////////
//
// PUBLIC
//


void Transceiver::Init(CC1101 & cc)
{
  cc1101=&cc;
  cc1101->Init();
  cc1101->StartReceive(RECEIVE_TO);
        pPacket = &RX_buffer.packet;
        pHeader = &pPacket->Header;
        txPacket = &TX_buffer.packet;
        txHeader = &txPacket->Header;

}

void Transceiver::StartReceive()
{
  cc1101->StartReceive(RECEIVE_TO);
}

uint8_t Transceiver::GetData()
{
  if (cc1101->GetState() == CCGOTPACKET)
  {
    Serial.print("G");
    rSize=cc1101->GetData((uint8_t*)&RX_buffer);
    return rSize;
//  packet_t * pPacket;

  } else{
    return 0;
  }
}

bool Transceiver::Valid()
{
        pPacket = &RX_buffer.packet;
        pHeader = &pPacket->Header;

      bool io= ((RX_buffer.len>=sizeof(header_t)) && (RX_buffer.len<=sizeof(packet_t)));
      if (io)
        io =( (pHeader->DestinationId==myID));
      return io;
}


unsigned short Transceiver::crcCheck()
{
          unsigned short cnt = pHeader->crc;
          pHeader->crc = 0;
//          unsigned short c=42;
//          for(unsigned short i=0 ; i<RX_buffer.len ; i++) c+=((uint8_t*)pPacket)[i];

          //valid packet crc

          bool io= (CRC(*pPacket)-cnt);
          if (io) {
            ack.Recive(pHeader->SourceId,  pHeader->Sequence);
            ack.Accept(pHeader->pseq);
          };
          return io;

}

uint8_t Transceiver::GetLen(packet_t & p)
{
  return (sizeof(header_t)+p.Header.Len);
}

uint8_t Transceiver::CRC(packet_t & p)
{
    unsigned short c=42;
    for(unsigned short i=0 ; i<(sizeof(header_t)+p.Header.Len) ; i++)
    {
      c+=((uint8_t*)&p)[i];
    }
    return c;
 
}  


float Transceiver::Rssi()
{
            crc = pPacket->Body[pHeader->Len+1];
            unsigned short c = pPacket->Body[pHeader->Len];
            rssi = c;
            if (c&0x80) rssi -= 256;
            rssi /= 2;
            rssi -= 74;
            return rssi;

}

void Transceiver::PrepareTransmit(uint8_t dst)
{
   TX_buffer.len=GetLen(TX_buffer.packet);
   
//       txHeader->src = MID;
//    txHeader->dest = TID;
//   TX_buffer.packet.Header.Sequence=sequence;
   TX_buffer.packet.Header.SourceId=myID;
   TX_buffer.packet.Header.DestinationId=dst;
//   sizeof(header_t)+txHeader->len;
   TX_buffer.packet.Header.crc=0;
   TX_buffer.packet.Header.crc=CRC(TX_buffer.packet);
   ack.Send(dst,TX_buffer.packet.Header.Sequence);
   TX_buffer.packet.Header.pseq = ack.Answer(dst);

}   



unsigned char Transceiver::Transmit()
{
   return cc1101->Transmit((uint8_t*)&(TX_buffer.packet),TX_buffer.len); 
}

uint8_t Transceiver::Get(uint8_t* buf)
{
              uint8_t i;
              for(i=0; i<pHeader->Len  ;i++)  //fill uart buffer
              {
                buf[i] = pPacket->Body[i];
              }
              return i;

}
uint8_t Transceiver::Put(uint8_t* buf,uint8_t len)
{
              uint8_t i;
              txHeader->Len = len<_frameBodySize ? len : _frameBodySize;  //length
              for ( i=0 ; i<txHeader->Len ; i++ )
              {
                    txPacket->Body[i] = buf[i];
              }
              return i;      

}





//
// END OF FILE
//
