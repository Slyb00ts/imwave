//
//    FILE: transceiver.cpp
// VERSION: 0.1.00
// PURPOSE: DTransceiver library for Arduino
//
// DATASHEET: 
//
// HISTORY:
// 0.1.0 by Dariusz Mazur (01/09/201)
// inspired by DHT11 library
//

#include "transceiver.h"


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
        pHeader = &pPacket->header;
        txPacket = &TX_buffer.packet;
        txHeader = &txPacket->header;

}

void Transceiver::StartReceive()
{
  cc1101->StartReceive(RECEIVE_TO);
}

uint8_t Transceiver::GetData()
{
  if (cc1101->GetState() == CCGOTPACKET)
  {
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
        pHeader = &pPacket->header;

      bool io= ((RX_buffer.len>=sizeof(header_t)) && (RX_buffer.len<=sizeof(packet_t)));
      if (io)
        io =((pHeader->nwid==netID) && (pHeader->dest==myID));
      return io;
}


unsigned short Transceiver::crcCheck()
{
          unsigned short cnt = pHeader->crc;
          pHeader->crc = 0;
//          unsigned short c=42;
//          for(unsigned short i=0 ; i<RX_buffer.len ; i++) c+=((uint8_t*)pPacket)[i];

          //valid packet crc
          return (CRC(*pPacket)-cnt);

}

uint8_t Transceiver::GetLen(packet_t & p)
{
  return (sizeof(header_t)+p.header.len);
}

uint8_t Transceiver::CRC(packet_t & p)
{
    unsigned short c=42;
    for(unsigned short i=0 ; i<(sizeof(header_t)+p.header.len) ; i++)
    {
      c+=((uint8_t*)&p)[i];
    }
    return c;
 
}  


float Transceiver::Rssi()
{
            crc = pPacket->data[pHeader->len+1];
            unsigned short c = pPacket->data[pHeader->len];
            rssi = c;
            if (c&0x80) rssi -= 256;
            rssi /= 2;
            rssi -= 74;
            return rssi;

}

void Transceiver::PrepareTransmit(uint8_t src,uint8_t dst)
{
   TX_buffer.len=GetLen(TX_buffer.packet);
   
//       txHeader->src = MID;
//    txHeader->dest = TID;
   TX_buffer.packet.header.nwid=netID;
   TX_buffer.packet.header.src=myID;
   TX_buffer.packet.header.dest=dst;
//   sizeof(header_t)+txHeader->len;
   TX_buffer.packet.header.crc=0;
   TX_buffer.packet.header.crc=CRC(TX_buffer.packet);
}   



unsigned char Transceiver::Transmit()
{
   return cc1101->Transmit((uint8_t*)&(TX_buffer.packet),TX_buffer.len); 
}

int Transceiver::Get(uint8_t* buf)
{
              int i;
              for(i=0; i<pHeader->len  ;i++)  //fill uart buffer
              {
                buf[i] = pPacket->data[i];
              }
              return i;

}
int Transceiver::Put(uint8_t* buf,uint8_t len)
{
              int i;
              txHeader->len = len<MAXDATALEN ? len : MAXDATALEN;  //length
              for ( i=0 ; i<txHeader->len ; i++ )
              {
                    txPacket->data[i] = buf[i];
              }
              return i;      

}


int TableACK::Send(uint8_t Addr, uint8_t Seq)
{
   lastsentseq=Seq;
   for(int i =0 ;i<MAXTableACK;i++)
     if (addr[i]==0){
       addr[i]=Addr;
       seq[i]=Seq;
       return i;
     };
   return -1;

}
void TableACK::Accept( uint8_t Seq)
{
   lastsentseq=0;
}


int TableACK::Recive(uint8_t Addr, uint8_t Seq)
{
  partnerseqnr=Seq;
  return 0;
}
uint8_t TableACK::Answer(uint8_t Addr)
{
  return partnerseqnr;
}
bool TableACK::noack(uint8_t Addr)
{
  return lastsentseq;

}




//
// END OF FILE
//
