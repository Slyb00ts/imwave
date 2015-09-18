#include <imframe.h>

#define _stackSize	10

#define CC1101		1
#define NRF24L01	2
#define NRF69		3

struct IMStack
{
	byte Device;
	byte[32] Frame;
} imStack[_stackSize];