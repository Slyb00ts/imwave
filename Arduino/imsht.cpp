#include <inttypes.h>
#include <Wire.h>
#include "imsht.h"

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

//Private:
//
uint16_t IMSht2x::ReadData(uint8_t command)
{
	uint16_t result;

	Wire.beginTransmission(Sht2xAddr);	//begin
	Wire.write(command);				//send the pointer location
	delay(100);
	Wire.endTransmission();				//end

	Wire.requestFrom(Sht2xAddr, 3);
	while (Wire.available() < 3) {
		; //wait
	}

	//Store the result
	result = ((Wire.read()) << 8);
	result += Wire.read();
	result &= ~0x0003;   // clear two low bits (status bits)
	return result;
}

//Public:
uint16_t IMSht2x::GetHumidityBin(void)
{
	return (ReadData(Sht2xHumCmd));
}
uint16_t IMSht2x::GetTemperatureBin(void)
{
	return (ReadData(Sht2xTempCmd));
}


float IMSht2x::GetHumidity(void)
{
	return (-6.0 + 125.0 / 65536.0 * (float)(ReadData(Sht2xHumCmd)));
}

float IMSht2x::GetTemperature(void)
{
	return (-46.85 + 175.72 / 65536.0 * (float)(ReadData(Sht2xTempCmd)));
}
