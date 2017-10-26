#include <imrouting.h>


IMRouting::IMRouting()
{
  reset();
//   byte i = empty();
//  return 0;
}

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
/*
byte IMRouting::getChannel(IMAddress addr)
{
  for(byte i=1;i<count;i++)
  {
    if (ORIGIN[i]==addr)
      return CHANNEL[i];
  }
  return 0;
}
  */

byte IMRouting::find(IMMAC mac)
{
 for(byte i=1;i<count;i++)
  {
    if (MACARRAY[i]==mac)
      return i;
  }
  return 0;

}
bool IMRouting::addMAC(IMMAC mac,IMAddress ward)
{
   byte x=find(mac);
   if (x==0){
     if(count>=MAXTableRouting){
      count= MAXTableRouting /2;
      return false;
     }

     MACARRAY[count]=mac;
     WARD[count]=ward;
     ORIGIN[count]=0;
//     CHANNEL[count]=0;
      ++count;
   DBGINFO("\r\nROUTING");
   DBGINFO(count);
   DBGINFO(":");
   DBGINFO(mac);
   DBGINFO(">");
   }
   return true;
}

byte IMRouting::addAddress(IMMAC mac,IMAddress addr,byte channel)
{
   byte x=find(mac);
   if (x){

      ORIGIN[x]=addr;
//      CHANNEL[x]=channel;
      return WARD[x];

   }
   return 0xFF;
}

void IMRouting::reset()
{
  count=0;
}
