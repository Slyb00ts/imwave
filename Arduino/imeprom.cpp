#include <imeprom.h>
#include <EEPROM.h>

//Public:
//
void IMEprom::ReadConfig(void)
{
	for (unsigned int t = 0; t < sizeof(imEConfig); t++)
	{
		*((char *)&imEConfig + t) = EEPROM.read(0 + t);
	}
}

void IMEprom::WriteConfig(void)
{
	for (unsigned int t = 0; t < sizeof(imEConfig); t++)
		EEPROM.write(0 + t, *((char *)&imEConfig + t));
}
