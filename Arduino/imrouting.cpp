#include <imrouting.h>

byte IMRouting::find()
{
  return 0;
}

byte IMRouting::Send(IMFrame & frame)
{
   lastsentseq=Seq;
//   byte i = empty();

}
void IMRouting::Accept( uint8_t Seq)
{
   lastsentseq=0;
}


byte IMRouting::Recive(uint8_t Addr, uint8_t Seq)
{
  partnerseqnr=Seq;
  return 0;
}

