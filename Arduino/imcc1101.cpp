#include <imcc1101.h>
#include <imframe.h>

#if defined(ARDUINO) && ARDUINO >= 100  
	#include "Arduino.h"  
#else  
	#include "WProgram.h"  
#endif 

//Private
void IMCC1101::SetRxState(void)
{
	//rfState = RFSTATE_RX;
	SpiStrobe(CC1101_SRX);
}

void IMCC1101::SetTxState(void)
{
	//rfState = RFSTATE_TX;
	SpiStrobe(CC1101_STX);
}

void IMCC1101::SetIdleState(void)
{
	//rfState = RFSTATE_IDLE;
	SpiStrobe(CC1101_SIDLE);
}

void IMCC1101::FlushRxFifo(void)
{
	SetIdleState();
	SpiStrobe(CC1101_SFRX);
}

void IMCC1101::FlushTxFifo(void)
{
	SetIdleState();
	SpiStrobe(CC1101_SFTX);
}

void IMCC1101::FlushFifo(void)
{
	SetIdleState();
	SpiStrobe(CC1101_SFRX); //Flush RX Fifo
	SpiStrobe(CC1101_SFTX); //Flush TX Fifo
}

void IMCC1101::SpiInit(void)
{
	// initialize the SPI pins
	pinMode(SCK_PIN, OUTPUT);
	pinMode(MOSI_PIN, OUTPUT);
	pinMode(MISO_PIN, INPUT);
	pinMode(SS_PIN, OUTPUT);

	// enable SPI Master, MSB, SPI mode 0, FOSC/4
	SpiMode(0);
}

void IMCC1101::SpiMode(byte config)
{
	byte tmp;

	// enable SPI master with configuration byte specified
	SPCR = 0;
	SPCR = (config & 0x7F) | (1 << SPE) | (1 << MSTR);
	tmp = SPSR;
	tmp = SPDR;
}

byte IMCC1101::SpiTransfer(byte value)
{
	SPDR = value;
	while (!(SPSR & (1 << SPIF)));
	return SPDR;
}

void IMCC1101::GDO_Set(void)
{
	pinMode(GDO0, INPUT);
	pinMode(GDO2, INPUT);
}

void IMCC1101::SpiWriteReg(byte addr, byte value)
{
	digitalWrite(SS_PIN, LOW);
	while (digitalRead(MISO_PIN));
	SpiTransfer(addr);
	SpiTransfer(value);
	digitalWrite(SS_PIN, HIGH);
}

void IMCC1101::SpiWriteBurstReg(byte addr, byte *buffer, byte num)
{
	byte i, temp;

	temp = addr | WRITE_BURST;
	digitalWrite(SS_PIN, LOW);
	while (digitalRead(MISO_PIN));
	SpiTransfer(temp);
	for (i = 0; i < num; i++)
	{
		SpiTransfer(buffer[i]);
	}
	digitalWrite(SS_PIN, HIGH);
}

void IMCC1101::SpiStrobe(byte strobe)
{
	digitalWrite(SS_PIN, LOW);
	while (digitalRead(MISO_PIN));
	SpiTransfer(strobe);
	digitalWrite(SS_PIN, HIGH);
}

byte IMCC1101::SpiReadReg(byte addr)
{
	byte temp, value;

	temp = addr | READ_SINGLE;
	digitalWrite(SS_PIN, LOW);
	while (digitalRead(MISO_PIN));
	SpiTransfer(temp);
	value = SpiTransfer(0);
	digitalWrite(SS_PIN, HIGH);

	return value;
}

void IMCC1101::SpiReadBurstReg(byte addr, byte *buffer, byte num)
{
	byte i, temp;

	temp = addr | READ_BURST;
	digitalWrite(SS_PIN, LOW);
	while (digitalRead(MISO_PIN));
	SpiTransfer(temp);
	for (i = 0; i<num; i++)
	{
		buffer[i] = SpiTransfer(0);
	}
	digitalWrite(SS_PIN, HIGH);
}

byte IMCC1101::SpiReadStatus(byte addr)
{
	byte value, temp;

	temp = addr | READ_BURST;
	digitalWrite(SS_PIN, LOW);
	while (digitalRead(MISO_PIN));
	SpiTransfer(temp);
	value = SpiTransfer(0);
	digitalWrite(SS_PIN, HIGH);

	return value;
}

void IMCC1101::RegConfigSettings(void)
{
	SpiWriteReg(CC1101_FSCTRL1, 0x06);
	SpiWriteReg(CC1101_FSCTRL0, 0x00);
	SpiWriteReg(CC1101_FREQ2, 0x21);
	SpiWriteReg(CC1101_FREQ1, 0x62);
	SpiWriteReg(CC1101_FREQ0, 0x76);
	SpiWriteReg(CC1101_MDMCFG4, 0xCA);
	SpiWriteReg(CC1101_MDMCFG3, 0x83);
	SpiWriteReg(CC1101_MDMCFG2, 0x13);
	SpiWriteReg(CC1101_MDMCFG1, 0x22);
	SpiWriteReg(CC1101_MDMCFG0, 0xF8);
	SpiWriteReg(CC1101_CHANNR, 0x00);
	SpiWriteReg(CC1101_DEVIATN, 0x35);
	SpiWriteReg(CC1101_FREND1, 0x56);
	SpiWriteReg(CC1101_FREND0, 0x10);
	SpiWriteReg(CC1101_MCSM0, 0x18);
	SpiWriteReg(CC1101_FOCCFG, 0x16);
	SpiWriteReg(CC1101_BSCFG, 0x1C);
	SpiWriteReg(CC1101_AGCCTRL2, 0xC7);
	SpiWriteReg(CC1101_AGCCTRL1, 0x00);
	SpiWriteReg(CC1101_AGCCTRL0, 0xB2);
	SpiWriteReg(CC1101_FSCAL3, 0xE9);
	SpiWriteReg(CC1101_FSCAL2, 0x2A);
	SpiWriteReg(CC1101_FSCAL1, 0x00);
	SpiWriteReg(CC1101_FSCAL0, 0x1F);
	SpiWriteReg(CC1101_FSTEST, 0x59);
	SpiWriteReg(CC1101_TEST2, 0x81);
	SpiWriteReg(CC1101_TEST1, 0x35);
	SpiWriteReg(CC1101_TEST0, 0x09);
	SpiWriteReg(CC1101_IOCFG2, 0x0B); 	//serial clock.synchronous to the data in synchronous serial mode
	SpiWriteReg(CC1101_IOCFG0, 0x06);  	//asserts when sync word has been sent/received, and de-asserts at the end of the packet 
	SpiWriteReg(CC1101_PKTCTRL1, 0x04);		//two status bytes will be appended to the payload of the packet,including RSSI LQI and CRC OK
											//No address check
	SpiWriteReg(CC1101_PKTCTRL0, 0x05);		//whitening off;CRC Enable£»variable length packets, packet length configured by the first byte after sync word
	SpiWriteReg(CC1101_ADDR, 0x00);		//address used for packet filtration.
	SpiWriteReg(CC1101_PKTLEN, 0x3D); 	//61 bytes max length

}

//Public
void IMCC1101::EnableCCA(void)
{
	SpiWriteReg(CC1101_MCSM1, CC1101_DEFVAL_MCSM1);
}

void IMCC1101::DisableCCA(void)
{
	SpiWriteReg(CC1101_MCSM1, 0);
}

void IMCC1101::EnableAddressCheck(void)
{
	SpiWriteReg(CC1101_PKTCTRL1, 0x06);
}

void IMCC1101::DisableAddressCheck(void)
{
	SpiWriteReg(CC1101_PKTCTRL1, 0x04);
}

void IMCC1101::SetChannel(byte CHANNR)
{
	SpiWriteReg(CC1101_CHANNR, CHANNR);
}

void IMCC1101::PowerDown(void)
{
	// Comming from RX state, we need to enter the IDLE state first
	SpiStrobe(CC1101_SIDLE);
	// Enter Power-down state
	SpiStrobe(CC1101_SPWD);
}

void IMCC1101::WakeUp(void)
{
	digitalWrite(SS_PIN, LOW);
	while (digitalRead(MISO_PIN));
	digitalWrite(SS_PIN, HIGH);
}

void IMCC1101::Reset(void)
{
	digitalWrite(SS_PIN, LOW);
	delay(1);
	digitalWrite(SS_PIN, HIGH);
	delay(1);
	digitalWrite(SS_PIN, LOW);
	while(digitalRead(MISO_PIN));
	SpiTransfer(CC1101_SRES);
	while(digitalRead(MISO_PIN));
	digitalWrite(SS_PIN, HIGH);
}

void IMCC1101::Init(void)
{
	SpiInit();										//spi initialization
	GDO_Set();										//GDO set
	digitalWrite(SS_PIN, HIGH);
	digitalWrite(SCK_PIN, HIGH);
	digitalWrite(MOSI_PIN, LOW);
	Reset();										//CC1101 reset
	RegConfigSettings();							//CC1101 register config
	SpiWriteBurstReg(CC1101_PATABLE,PaTabel,8);		//CC1101 PATABLE config
	EnableCCA();
}

void IMCC1101::Reinit(void)
{
	pinMode(SCK_PIN, INPUT);
	pinMode(MOSI_PIN, INPUT);
	pinMode(MISO_PIN, OUTPUT);
	pinMode(SS_PIN, INPUT);
	pinMode(GDO0, OUTPUT);
	pinMode(GDO2, OUTPUT);
	digitalWrite(SS_PIN, LOW);
	digitalWrite(SCK_PIN, LOW);
	digitalWrite(MOSI_PIN, HIGH);
	Init();
}

boolean IMCC1101::SendData(IMFrame &frame)
{
	//Prepare TX_buffer and send
	byte TX_buffer[sizeof(frame)];
	memcpy(TX_buffer, &frame, sizeof(frame));
	SendData(TX_buffer, sizeof(frame));
}

boolean IMCC1101::SendData(byte *txBuffer,byte size)
{
	byte marcState;
	bool res = false;

	SetRxState();

	// Check that the RX state has been entered
	while (((marcState = SpiReadStatus(CC1101_MARCSTATE)) & 0x1F) != 0x0D)
	{
		if (marcState == 0x11) // RX_OVERFLOW
		{
			FlushRxFifo(); // flush receive queue
		}
	}

	delayMicroseconds(500);

	SpiWriteReg(CC1101_TXFIFO,size);
	SpiWriteBurstReg(CC1101_TXFIFO,txBuffer,size); //write data to send
	SetTxState(); //start send	
															
	marcState = SpiReadStatus(CC1101_MARCSTATE) & 0x1F;
	if ((marcState != 0x13) && (marcState != 0x14) && (marcState != 0x15))
	{
		FlushTxFifo();
		SetRxState();         // Back to RX state

		return false;
	}

	while (!digitalRead(GDO0));	// Wait for GDO0 to be set -> sync transmitted  
	while (digitalRead(GDO0));	// Wait for GDO0 to be cleared -> end of packet

	// Check that the TX FIFO is empty
	if ((SpiReadStatus(CC1101_TXBYTES) & 0x7F) == 0)
		res = true;

	FlushTxFifo();

	return res;
}

void IMCC1101::SetReceive(void)
{
	FlushRxFifo();
	SetRxState();
}

byte IMCC1101::CheckReceiveFlag(void)
{
	if(digitalRead(GDO0))			//receive data
	{
		while (digitalRead(GDO0));
		return 1;
	}
	else							// no data
	{
		return 0;
	}
}

byte IMCC1101::ReceiveData(byte *rxBuffer)
{
	byte size;
	byte status[2];

	if(SpiReadStatus(CC1101_RXBYTES) & BYTES_IN_RXFIFO)
	{
		size=SpiReadReg(CC1101_RXFIFO);
		SpiReadBurstReg(CC1101_RXFIFO,rxBuffer,size);
		SpiReadBurstReg(CC1101_RXFIFO,status,2);	
		rxBuffer[5] = status[0];
		rxBuffer[6] = status[1];
		FlushRxFifo();
		return status[1] & 0x80u;
	}
	else
	{
		FlushRxFifo();
		return 0;
	}
}