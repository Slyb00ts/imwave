//
//    FILE: imeeprom.h
// VERSION: 0.2.00
// PURPOSE: EEPROM Read/Write for Arduino

//
// HISTORY:
//

#ifndef imEprom_h
#define imEprom_h
#include <Arduino.h>



static struct IMEConfig
{
	uint16_t MacLo;
        uint16_t MacHigh;
        uint16_t Mode;
        uint8_t Id;
        uint8_t HostId;
        uint8_t ServerId;
        uint8_t Channel;
        uint16_t SynchronizeCycle;
        uint8_t Power;
        uint8_t Future;

}imEConfig;

class IMEprom
{
	private:
	public:
		static void ReadConfig(void);
		static void WriteConfig(void);

};
#endif
