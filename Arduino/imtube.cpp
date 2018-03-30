#include <imtube.h>

IMTube::IMTube()
{

  Reset();
}
void IMTube::PrepareInvalid(IMFrameSetup &se)
{
  ++invalidSequence;
  se.slavechannel = invalidSequence;
  se.address = wsequence; //++on wellcome
}

void IMTube::Reset()
{
  invalidSequence = 0;
  wsequence = 0;
}
