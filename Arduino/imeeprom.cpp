#include <imeeprom.h>
#include <EEPROM.h>

//Public:
//
void IMEeprom::ReadConfig(void) {
	for (unsigned int t = 0; t < sizeof(imConfig); t++) {
		*((char*)&imConfig + t) = EEPROM.read(0 + t);
	}
}

void IMEeprom::WriteConfig(void) {
	for (unsigned int t = 0; t<sizeof(imConfig); t++)
		EEPROM.write(0 + t, *((char*)&imConfig + t));
}

void IMEeprom::WriteConfigIfNotExists(void) {
	ReadConfig();
	if (imConfig.MacAddress1 == 0 || imConfig.MacAddress1 == 0xFFFFFFFFFFFFFFFF) {
		WriteConfig();
	}
}