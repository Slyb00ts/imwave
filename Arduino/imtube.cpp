#include <imtube.h>

IMTube::IMTube()
{

  Reset();
}
void IMTube::PrepareInvalid(IMFrameSetup &se)
{
  ++invalidSequence;
  se.slavechannel = invalidSequence;
  se.address = wSequence; //++on wellcome
}

void IMTube::PrepareStatus(IMFrameSetup &se)
{
  ++sSequence;
  se.slavechannel = invalidSequence;
  se.address = wSequence; //++on wellcome
}


void IMTube::Reset()
{
  invalidSequence = 0;
  wSequence = 0;
  sSequence = 0;
}

void IMTube::OnWelcome()
{
  invalidSequence /= 2;
  ++wSequence ;
}
