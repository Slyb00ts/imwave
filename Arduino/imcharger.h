//
//    FILE: imcharger.h
// VERSION: 0.1.00
// PURPOSE: BQ24161RGE Charger library for Arduino

//
// HISTORY:
//

#ifndef imCharger_h
#define imCharger_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

// BQ24161RGE Charger support
#define  CHRG_ADDR				0x6B
// registers  
#define  CHRG_STATUS_CONTROL    0x00  //  read / write
#define  CHRG_BAT_SUPPLY        0x01  //  read / write
#define  CHRG_CONTROL           0x02  //  read / write
#define  CHRG_BAT_VOLTAGE       0x03  //  read / write
#define  CHRG_VEND_PART_REV     0x04  //  read only
#define  CHRG_BATTERM_FASTCHRG  0x05  //  read / write
#define  CHRG_VINDMP_DPPM       0x06  //  partially read only
#define  CHRG_SAFETYTIMER_NTC   0x07  //  read / write  
// USB input modes
// used in ChrgSetUSBLimit()
#define  CHRG_USB_100MA              0b000  //  USB 2.0 - 100mA limit
#define  CHRG_USB_150MA              0b001  //  USB 3.0 - 150mA limit
#define  CHRG_USB_500MA              0b010  //  USB 2.0 - 500mA limit
#define  CHRG_USB_800MA              0b011  //  USB - 800mA limit
#define  CHRG_USB_900MA              0b100  //  USB 3.0 - 900mA limit
#define  CHRG_USB_1500MA             0b101  //  USB - 1500mA limit  
// Charge voltages
#define  CHRG_VBATREG_4_0V           0b011001  //  4.0V  ->3.5+0.5  -> 500/20 = 25 = 0x19
#define  CHRG_VBATREG_4_2V           0b100011  //  4.2V  ->3.5 + 0.7-> 700/20 = 35 = 0x23  
// Charge Currents
#define  CHRG_CURRENT_550MA          0b00000  //  550mA
#define  CHRG_CURRENT_700MA          0b00010  //  700mA
#define  CHRG_CURRENT_1000MA         0b00110  //  1000mA = 450+550 = 300+150 + 550
#define  CHRG_CURRENT_1300MA         0b01010  //  1300mA  
// Termination Currents
#define  CHRG_TERM_CURRENT_50MA      0b000  //  50mA
#define  CHRG_TERM_CURRENT_100MA     0b001  //  100mA
#define  CHRG_TERM_CURRENT_150MA     0b010  //  150mA
// VIN status
#define  CHRG_VIN_NONE               0b00  //  No external supply connected
#define  CHRG_VIN_USB                0b01  //  USB connected
#define  CHRG_VIN_EXT                0b10  //  External supply connected
#define  CHRG_VIN_BOTH               0b11  //  Both USB and external supply connected

class IMCharger
{
  private:
	void WatchdogRst(void);
	uint8_t ReadReg(uint8_t regist);
	void SetTerminationLimit(uint8_t limit);
	void SetCurrentLimit(uint8_t limit);
	void SetVbat(uint8_t voltage);
	void SetUSBLimit(uint8_t limit);
	uint8_t ReadFault(void);
	uint8_t ReadStat(void);
	void EnableCharging(void);
	void DisableCharging(void);
	void DisableTS(void);
	void WriteReg(uint8_t regist, uint8_t value);

  public:
	boolean Init(void);
	void EnableStatLED(void);
	void KeepAlive(void);
	uint8_t GetVinStat(void);
	uint8_t IsChrg(void);
};
#endif