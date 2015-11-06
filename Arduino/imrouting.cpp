#include <imrouting.h>


byte IMRouting::Send(IMFrame & frame)
{
//   byte i = empty();
  return 0;

}


IMAddress IMRouting::Forward(IMAddress addr)
{
  for(byte i=0;i<count;i++)
  {
  }
  return 0xFF;
}

IMAddress IMRouting::Repeater(IMAddress addr)
{
  for(byte i=1;i<count;i++)
  {
    if (ORIGIN[i]==addr)
      return BYPASS[i];
  }
  return 0;
}

byte IMRouting::find(IMMAC mac)
{
 for(byte i=1;i<count;i++)
  {
    if (MMAC[i]==mac)
      return i;
  }
  return 0;

}
void IMRouting::addMAC(IMMAC mac,IMAddress bypass)
{
   byte x=find(mac);
   if (x==0){

     MMAC[count]=mac;
     BYPASS[count]=bypass;
      ++count;
   }
}

void IMRouting::addAddress(IMMAC mac,IMAddress addr)
{
   byte x=find(mac);
   if (x){
      ORIGIN[x]=addr;

    }



}

void IMRouting::reset()
{
  count=0;
}
