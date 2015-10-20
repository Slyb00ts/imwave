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
    if (Source[i]==addr)
      return Destination[i];
  }
  return 0xFF;
}

IMAddress IMRouting::Repeater(IMAddress addr)
{
  return 0;
}

byte IMRouting::addMAC(IMMAC mac)
{
  _mac=mac;
}


