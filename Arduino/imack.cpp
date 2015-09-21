#include <imack.h>


byte TableACK::Send(uint8_t Addr, uint8_t Seq)
{
   lastsentseq=Seq;
   for(byte i =0 ;i<MAXTableACK;i++)
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


byte TableACK::Recive(uint8_t Addr, uint8_t Seq)
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
  if (lastsentseq==0){
    retrycnt=0;
    return true;
  } else{
    return lastsentseq;
  }

}


bool TableACK::retry()
{
      bool io= (retrycnt>=MAXRETRIES);
      if (io){
        retrycnt = 0;  //reset retries
      } else{
        retrycnt++;
      }
      return io;
}

