
//
//    FILE: imqueue.h
// VERSION: 0.1.00
// PURPOSE: FIFO Queue for Arduino
// based on lock free queue
//           http://www.emadar.com/fpc/lockfree.htm

//
// HISTORY:
//
#ifndef imQueue_h
#define imQueue_h


#include <imframe.h>

#define _QueueSize 16
#define _QueueMask 0xF0

#define  NodeQueue IMFrame
class  IMQueue
{
  private:
     typedef uint8_t address;
     address tail;
      uint8_t head;
      uint8_t integer;
      IMFrame tab[_QueueSize];
      NodeQueue getObject(address lp );
      void setObject(address id ,NodeQueue & node);

   public:
     address Length();
     NodeQueue pop();
     void push(NodeQueue & node);

} ;


#endif
