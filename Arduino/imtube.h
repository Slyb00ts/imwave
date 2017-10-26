//
//    FILE: imtube.h
// VERSION: 0.1.00
// PURPOSE: prepare frame to send

#ifndef imTube_h
#define imTube_h


#include "imframe.h"
#include "imdebug.h"


class IMTube
{
  private:
  public:
      uint16_t invalidSequence;
      byte wsequence;

    IMTube();
    void PrepareInvalid(IMFrameSetup &se);
    void Reset();
};

#endif

