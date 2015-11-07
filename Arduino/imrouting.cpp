#include <imrouting.h>


byte IMRouting::Send(IMFrame & frame)
{
//   byte i = empty();
  return 0;

}



IMAddress IMRouting::Repeater(IMAddress addr)
{
  for(byte i=1;i<count;i++)
  {
    if (ORIGIN[i]==addr)
      return WARD[i];
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
void IMRouting::addMAC(IMMAC mac,IMAddress ward)
{
   byte x=find(mac);
   if (x==0){

     MMAC[count]=mac;
     WARD[count]=ward;
      ++count;
   }
}

byte IMRouting::addAddress(IMMAC mac,IMAddress addr)
{
   byte x=find(mac);
   if (x){
      ORIGIN[x]=addr;
      return WARD[x];

   }
   return 0xFF;



}

void IMRouting::reset()
{
  count=0;
}
