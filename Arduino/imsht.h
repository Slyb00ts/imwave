//
//    FILE: imsht.h
// VERSION: 0.1.00
// PURPOSE: SHT2x Temperature sensor library for Arduino

//
// HISTORY:
//

#ifndef imSHT_h
#define imSHT_h

#include <inttypes.h>
#include <Wire.h>

#define Sht2xAddr		0x40
#define Sht2xTempCmd	0xE3
#define Sht2xHumCmd		0xE5

class IMSht2x
{
	private:
		uint16_t ReadData(uint8_t command);
	public:
		float GetHumidity(void);
		float GetTemperature(void);
		uint16_t GetHumidityBin(void);
		uint16_t GetTemperatureBin(void);
};
#endif
