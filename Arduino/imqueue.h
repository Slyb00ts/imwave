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

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#define _QueueSize 8
#define _QueueMask 0x7

#define  NodeQueue IMFrame

class  IMQueue
{
//  private:
	public:
		typedef uint8_t address;
		address tail;
		uint8_t head;
		address temp;
		IMFrame tab[_QueueSize];
		NodeQueue getObject(address id );
		void setObject(address id ,const NodeQueue & node);

	public:
		unsigned short Length();
		bool pop(NodeQueue & node);
		void push(const NodeQueue & node);
} ;


#endif