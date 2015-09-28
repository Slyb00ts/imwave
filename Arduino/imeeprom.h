//
//    FILE: imeeprom.h
// VERSION: 0.1.00
// PURPOSE: EEPROM Read/Write for Arduino

//
// HISTORY:
//

#ifndef imEeprom_h
#define imEeprom_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

static struct IMConfig
{
	uint32_t MacAddress1;
	uint32_t MacAddress2;
}imConfig;

class IMEeprom
{
	private:
	public:
		void ReadConfig(void);
		void WriteConfig(void);
		void WriteConfigIfNotExists(void);
};
#endif