//
//    FILE: imeeprom.h
// VERSION: 0.1.00
// PURPOSE: EEPROM Read/Write for Arduino

//
// HISTORY:
//

#ifndef imEprom_h
#define imEprom_h
#include <Arduino.h>



static struct IMEConfig
{
	uint32_t MacAddress;
        uint16_t myMode;
        uint8_t myId;
        uint8_t hostId;
        uint8_t serverId;
        uint8_t myChannel;
        uint8_t myBroadcast;

}imEConfig;

class IMEprom
{
	private:
	public:
		void ReadConfig(void);
		void WriteConfig(void);

};
#endif
