#include <imack.h>

byte TableACK::find()
{
  return 0;
}

byte TableACK::Send(IMFrame & frame)
{
   byte i = empty();
   tab[i]=frame;
   return i;
//   retry[i]=0;

}
void TableACK::Accept( uint8_t Seq)
{
}


byte TableACK::Receive(IMFrame & frame)
{
  partnerseqnr=frame.Header.Sequence;
  Accept(partnerseqnr);
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

byte TableACK::empty()
{
  byte x=0xFF;
  for (byte i=0; i<MAXTableACK;i++)
  {
    if (tab[i].Header.Function==0)
      return i;
    x=  (x <tab[i].Header.Sequence) ? x : tab[i].Header.Sequence;
  }
  for (byte i=0; i<MAXTableACK;i++)
  {
    if (tab[i].Header.Sequence==x)
        return i;
  }
  return 0;
}

IMFrame * TableACK::toRetry()
{
  for (byte i=0; i<MAXTableACK;i++)
  {
    if (tab[i].Header.Function!=0)
     return &(tab[i]);
  }

  return NULL;
}

