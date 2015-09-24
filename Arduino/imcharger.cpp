#include <Wire.h>
#include <imcharger.h>

//Private:
//
// Reset Watchdog Timer
// Watchdog timer typ. 30 seconds
void IMCharger::WatchdogRst(void)
{
	// Write "1" to TMR_RST 
	uint8_t stat = ReadReg(CHRG_STATUS_CONTROL);
	stat |= (1 << 7);  // TMR_RST-Bit  
	WriteReg(CHRG_STATUS_CONTROL, stat);
}

uint8_t IMCharger::ReadReg(uint8_t regist)
{
	uint8_t value = 0;
	Wire.beginTransmission(CHRG_ADDR);
	Wire.write(regist);        //sends address to read from
	Wire.endTransmission(); //end transmission
	Wire.beginTransmission(CHRG_ADDR); //start transmission to device
	Wire.requestFrom(CHRG_ADDR, 1);    // request 1 byte from device
	while (Wire.available()) {  // ((Wire.available())&&(i<6))
		value = Wire.read();  // receive one byte
	}
	Wire.endTransmission(); //end transmission
	return value;
}

// Set Termination current
// If charging current falls below this level,
// charging will be disabled
void IMCharger::SetTerminationLimit(uint8_t limit)
{
	uint8_t stat = ReadReg(CHRG_BATTERM_FASTCHRG);
	stat &= 0xF8;  // Set termination current bits to 0  
	stat |= limit;
	WriteReg(CHRG_BATTERM_FASTCHRG, stat);
}

// Set Charging current limit
void IMCharger::SetCurrentLimit(uint8_t limit)
{
	uint8_t stat = ReadReg(CHRG_BATTERM_FASTCHRG);
	stat &= 0x07;  // Set current bits to 0  
	stat |= (limit << 3);
	WriteReg(CHRG_BATTERM_FASTCHRG, stat);
}

// Set the target Charge voltage
void IMCharger::SetVbat(uint8_t voltage)
{
	uint8_t stat = ReadReg(CHRG_BAT_VOLTAGE);
	stat &= 0x03;  // Set VBAT bits to 0
	stat |= (voltage << 2);  // 
	WriteReg(CHRG_BAT_VOLTAGE, stat);
}

// Sets USB input current limit
void IMCharger::SetUSBLimit(uint8_t limit)
{
	uint8_t stat = 0;
	if (limit > 5)  // Invalid value
	{
		limit = CHRG_USB_100MA;
	}
	stat = ReadReg(CHRG_CONTROL);  // Get current register content
	stat &= 0x0F;  // Set Bit7 to 0, Set USB-limit bits to 0
	stat |= (limit << 4);  // Set USB-limit bits  
	WriteReg(CHRG_CONTROL, stat);
}

// Read Fault bits which indicate
// a fault state
uint8_t IMCharger::ReadFault(void)
{
	uint8_t stat = ReadReg(CHRG_STATUS_CONTROL);
	stat &= 0x07;  // Get bits 2,1 and 0
	return stat;
}

// Read stat bits which indicate different
// states of the charger
uint8_t IMCharger::ReadStat(void)
{
	uint8_t stat = ReadReg(CHRG_STATUS_CONTROL);
	stat >>= 4;  // Get bits 6,5, and 4
	stat &= 0x07;
	return stat;
}

// keep charging by resetting the watchdog
void IMCharger::EnableCharging(void)
{
	uint8_t stat = ReadReg(CHRG_CONTROL);  // Get current register content
	stat &= ~((1 << 7) | (1 << 1));  // Set Bit7 to 0 and CE to 0
	WriteReg(CHRG_CONTROL, stat);
}

// Disable Charging
void IMCharger::DisableCharging(void)
{
	uint8_t stat = ReadReg(CHRG_CONTROL);  // Get current register content
	stat &= ~(1 << 7);  // no effect on RESET!
	stat |= (1 << 1);  // Set CE to 1, disable charge
	WriteReg(CHRG_CONTROL, stat);
}

// Disable the temperature control and disable safety timer to 
// which disables charging if the
// battery is too cold / too hot
void IMCharger::DisableTS(void)
{
	uint8_t stat = ReadReg(CHRG_SAFETYTIMER_NTC);  // Get current register content  
	stat &= ~(1 << 3);  // Set TS-bit to 0  
	stat |= (1 << 5);   //6h fast charge safety timer!
	WriteReg(CHRG_SAFETYTIMER_NTC, stat);
}

void IMCharger::WriteReg(uint8_t regist, uint8_t value)
{
	Wire.beginTransmission(CHRG_ADDR);
	Wire.write(regist);  // sends address to write to
	Wire.write(value);  // Write data
	Wire.endTransmission(); //end transmission
}



//Public:
//
// Init Charger module, reset wdt also!
boolean IMCharger::Init(void)
{
	//EnableStatLED();  //  Enable STAT LED to display Charging status
	DisableTS();  // Disable temperature monitoring
	SetVbat(CHRG_VBATREG_4_2V);  //  4.2V Charge voltage
	EnableStatLED();
	SetTerminationLimit(CHRG_TERM_CURRENT_50MA);  //  50mA Termination current limit
	SetCurrentLimit(CHRG_CURRENT_550MA);  //  500 mA Charge current
	SetUSBLimit(CHRG_USB_500MA);  // 500mA USB limit
	WatchdogRst();  // Reset watchdog timer
	return true;
}

// Enable STAT LED to display charging status
void IMCharger::EnableStatLED(void)
{
	uint8_t stat = ReadReg(CHRG_CONTROL);  // Get current register content
	stat &= ~(1 << 7);  // Set Bit7 to 0
	stat |= (1 << 3);  // Set EN_STAT to 1  
	WriteReg(CHRG_CONTROL, stat);
}

// This function will be called at 1/8 Hz
// Reset Watchdog but also take care that the charger 
// will get a fresh init every 160 seconds
void IMCharger::KeepAlive(void)
{
	static uint16_t count = 0;

	if (!(count % 20))  // Init every 160 seconds
	{
		Init();
	}

	count++;

	WatchdogRst();  // Reset Watchdog Timer
}

// Reads the appropiate registers to
// check which VIN input is connected
// or if there is no Vin connected / 
// no valid Vin connected
uint8_t IMCharger::GetVinStat(void)
{
	uint8_t stat = ReadReg(CHRG_BAT_SUPPLY);
	stat &= 0xF0;
	if (!stat)  return CHRG_VIN_BOTH;  //  Both supplies connected
	if (!((stat & 0xC0)))  return CHRG_VIN_EXT;  //  External supply connected
	if (!(stat & 0x30))  return CHRG_VIN_USB;  //  USB connected
	return CHRG_VIN_NONE;
}

// returns 1 if charging
uint8_t IMCharger::IsChrg(void)
{
	uint8_t stat = ReadReg(CHRG_STATUS_CONTROL);
	stat >>= 4;  // Get bits 6,5, and 4
	stat &= 0x07;
	if (stat == 3 || stat == 4)
		return 1;
	return 0;
}