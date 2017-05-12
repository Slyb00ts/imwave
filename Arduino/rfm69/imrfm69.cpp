// **********************************************************************************
// Driver definition for HopeRF RFM69W/RFM69HW/RFM69CW/RFM69HCW, Semtech SX1231/1231H
// **********************************************************************************
// Copyright Felix Rusu (2014), felix@lowpowerlab.com
// http://lowpowerlab.com/
// **********************************************************************************
// License
// **********************************************************************************
// This program is free software; you can redistribute it
// and/or modify it under the terms of the GNU General
// Public License as published by the Free Software
// Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will
// be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE. See the GNU General Public
// License for more details.
//
// You should have received a copy of the GNU General
// Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.
//
// Licence can be viewed at
// http://www.gnu.org/licenses/gpl-3.0.txt
//
// Please maintain this license information along with authorship
// and copyright notices in any redistribution of this code
// **********************************************************************************
#include <imrfm69.h>
#include <imrfmregisters.h>
//#include <SPI.h>

volatile uint8_t RFM69::DATA[RF69_BUF_LEN];
volatile uint8_t RFM69::_mode;        // current transceiver state
//volatile uint8_t RFM69::DATALEN;
volatile uint8_t RFM69::IRQ2;
volatile uint8_t RFM69::IRNN;
volatile uint8_t RFM69::_head;
volatile uint8_t RFM69::_tail;
//volatile uint8_t RFM69::SENDERID;
//volatile uint8_t RFM69::TARGETID;     // should match _address
volatile uint8_t RFM69::_lock;
//volatile uint8_t RFM69::PAYLOADLEN;
volatile int16_t RFM69::RSSI;          // most accurate RSSI during reception (closest to the reception)
RFM69* RFM69::selfPointer;

bool RFM69::initialize(uint8_t freqBand, uint8_t nodeID, uint8_t networkID)
{
  _head=0;
  _tail=0;
  _lock=0;
  _mode=0;
  const uint8_t CONFIG[][2] =
  {
    /* 0x01 */ { REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF | RF_OPMODE_STANDBY },
    /* 0x02 */ { REG_DATAMODUL, RF_DATAMODUL_DATAMODE_PACKET | RF_DATAMODUL_MODULATIONTYPE_FSK | RF_DATAMODUL_MODULATIONSHAPING_01 }, // no shaping
    /* 0x03 */ { REG_BITRATEMSB, RF_BITRATEMSB_100000}, // default: 4.8 KBPS
    /* 0x04 */ { REG_BITRATELSB, RF_BITRATELSB_100000},
    /* 0x05 */ { REG_FDEVMSB, RF_FDEVMSB_100000}, // default: 5KHz, (FDEV + BitRate / 2 <= 500KHz)
    /* 0x06 */ { REG_FDEVLSB, RF_FDEVLSB_100000},

    /* 0x07 */ { REG_FRFMSB, (uint8_t) (freqBand==RF69_315MHZ ? RF_FRFMSB_315 : (freqBand==RF69_433MHZ ? RF_FRFMSB_433 : (freqBand==RF69_868MHZ ? RF_FRFMSB_868 : RF_FRFMSB_915))) },
    /* 0x08 */ { REG_FRFMID, (uint8_t) (freqBand==RF69_315MHZ ? RF_FRFMID_315 : (freqBand==RF69_433MHZ ? RF_FRFMID_433 : (freqBand==RF69_868MHZ ? RF_FRFMID_868 : RF_FRFMID_915))) },
    /* 0x09 */ { REG_FRFLSB, (uint8_t) (freqBand==RF69_315MHZ ? RF_FRFLSB_315 : (freqBand==RF69_433MHZ ? RF_FRFLSB_433 : (freqBand==RF69_868MHZ ? RF_FRFLSB_868 : RF_FRFLSB_915))) },

    // looks like PA1 and PA2 are not implemented on RFM69W, hence the max output power is 13dBm
    // +17dBm and +20dBm are possible on RFM69HW
    // +13dBm formula: Pout = -18 + OutputPower (with PA0 or PA1**)
    // +17dBm formula: Pout = -14 + OutputPower (with PA1 and PA2)**
    // +20dBm formula: Pout = -11 + OutputPower (with PA1 and PA2)** and high power PA settings (section 3.3.7 in datasheet)
    ///* 0x11 */ { REG_PALEVEL, RF_PALEVEL_PA0_ON | RF_PALEVEL_PA1_OFF | RF_PALEVEL_PA2_OFF | RF_PALEVEL_OUTPUTPOWER_11111},
    /* 0x11 */ { REG_PALEVEL, RF_PALEVEL_PA0_OFF | RF_PALEVEL_PA1_ON | RF_PALEVEL_PA2_OFF | RF_PALEVEL_OUTPUTPOWER_10001},

//    {REG_PALEVEL, RF_PALEVEL_PA0_OFF | RF_PALEVEL_PA1_ON | RF_PALEVEL_PA2_ON  | RF_PALEVEL_OUTPUTPOWER_11111}, // enable P1 & P2 amplifier stages

    ///* 0x13 */ { REG_OCP, RF_OCP_ON | RF_OCP_TRIM_95  }, // over current protection (default is 95mA)
    ///* 0x18 */ { REG_LNA, RF_LNA_ZIN_50| RF_LNA_GAINSELECT_MAX  },
  /* 0x18 */ { REG_LNA, RF_LNA_ZIN_50| RF_LNA_GAINSELECT_AUTO  },

    // RXBW defaults are { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_24 | RF_RXBW_EXP_5} (RxBw: 10.4KHz)
    /* 0x19 */ { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_16 | RF_RXBW_EXP_2 }, // (BitRate < 2 * RxBw)
//     /* 0x1E */ { REG_AFCFEI, RF_AFCFEI_AFCAUTO_ON | RF_AFCFEI_AFCAUTOCLEAR_ON },                         // Automatic AFC on, clear after each packet
    //for BR-19200: /* 0x19 */ { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_24 | RF_RXBW_EXP_3 },
    /* 0x25 */ { REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_11 }, // DIO0 is the only IRQ we're using
    /* 0x26 */ { REG_DIOMAPPING2, RF_DIOMAPPING2_CLKOUT_OFF }, // DIO5 ClkOut disable for power saving
    /* 0x28 */ { REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN }, // writing to this bit ensures that the FIFO & status flags are reset
    /* 0x29 */ { REG_RSSITHRESH, 180 }, // must be set to dBm = (-Sensitivity / 2), default is 0xE4 = 228 so -114dBm
    /* 0x2D */ { REG_PREAMBLELSB, RF_PREAMBLESIZE_LSB_VALUE }, // default 3 preamble bytes 0xAAAAAA
    /* 0x2E */ { REG_SYNCCONFIG, RF_SYNC_ON | RF_SYNC_FIFOFILL_AUTO | RF_SYNC_SIZE_3 | RF_SYNC_TOL_0 },
    /* 0x2F */ { REG_SYNCVALUE1, 0x2D },      // attempt to make this compatible with sync1 byte of RFM12B lib
    /* 0x30 */ { REG_SYNCVALUE2, networkID }, // NETWORK ID
    /* 0x31 */ { REG_SYNCVALUE3, 0xAA }, // NETWORK ID
    /* 0x37 */ { REG_PACKETCONFIG1, RF_PACKET1_FORMAT_FIXED | RF_PACKET1_DCFREE_OFF | RF_PACKET1_CRC_ON | RF_PACKET1_CRCAUTOCLEAR_OFF | RF_PACKET1_ADRSFILTERING_OFF },
    /* 0x38 */ { REG_PAYLOADLENGTH, 32 }, // in variable length mode: the max frame size, not used in TX
    ///* 0x39 */ { REG_NODEADRS, nodeID }, // turned off because we're not using address filtering
    /* 0x3C */ { REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTART_FIFONOTEMPTY  | RF_FIFOTHRESH_VALUE }, // TX on FIFO not empty
    /* 0x3D */ { REG_PACKETCONFIG2, RF_PACKET2_RXRESTARTDELAY_2BITS | RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF }, // RXRESTARTDELAY must match transmitter PA ramp-down time (bitrate dependent)
    //for BR-19200: /* 0x3D */ { REG_PACKETCONFIG2, RF_PACKET2_RXRESTARTDELAY_NONE | RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF }, // RXRESTARTDELAY must match transmitter PA ramp-down time (bitrate dependent)
//    /* 0x6F */ { REG_TESTLNA, RF_TESTLNA_NORMAL}, // run DAGC continuously in RX mode for Fading Margin Improvement, recommended default for AfcLowBetaOn=0
    /* 0x6F */ { REG_TESTDAGC, RF_DAGC_IMPROVED_LOWBETA0 }, // run DAGC continuously in RX mode for Fading Margin Improvement, recommended default for AfcLowBetaOn=0
    {255, 0}
  };

  digitalWrite(RF69_RESET_PIN, LOW);
  pinMode(RF69_RESET_PIN, OUTPUT);
  digitalWrite(_slaveSelectPin, HIGH);
  pinMode(_slaveSelectPin, OUTPUT);
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
//  SPI.setClockDivider(SPI_CLOCK_DIV4); // decided to slow down from DIV2 after SPI stalling in some instances, especially visible on mega1284p when RFM69 and FLASH chip both present
  _lock=0;
  long start = millis();
  long timeout = 150;
  do writeReg(REG_SYNCVALUE1, 0xAA); while (readReg(REG_SYNCVALUE1) != 0xaa && millis()-start < timeout);
  start = millis();
  do writeReg(REG_SYNCVALUE1, 0x55); while (readReg(REG_SYNCVALUE1) != 0x55 && millis()-start < timeout);

  for (uint8_t i = 0; CONFIG[i][0] != 255; i++)
    writeReg(CONFIG[i][0], CONFIG[i][1]);

  // Encryption is persistent between resets and can trip you up during debugging.
  // Disable it during initialization so we always start from a known state.
  encrypt(0);
  setHighPower(true); // called regardless if it's a RFM69W or RFM69HW

//  setHighPower(_isRFM69HW); // called regardless if it's a RFM69W or RFM69HW
  setMode(RF69_MODE_STANDBY);
  start = millis();
  while (((readReg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00) && millis()-start < timeout); // wait for ModeReady
  if (millis()-start >= timeout)
    return false;
  DBGINFO("RFM initialize");
  readAllRegs();
  attachInterrupt(RF69_IRQ_NUM, RFM69::isr0, RISING);

  selfPointer = this;
  _address = nodeID;
  return true;
}

void RFM69::reset()
{
     writeReg(REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF |  RF_OPMODE_STANDBY);
     writeReg( REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN );

}
//static void RFM69::receivedDataNull(byte){}
// return the frequency (in Hz)
uint32_t RFM69::getFrequency()
{
  return RF69_FSTEP * (((uint32_t) readReg(REG_FRFMSB) << 16) + ((uint16_t) readReg(REG_FRFMID) << 8) + readReg(REG_FRFLSB));
}

// set the frequency (in Hz)
void RFM69::setFrequency(uint32_t freqHz)
{
  uint8_t oldMode = _mode;
  if (oldMode == RF69_MODE_TX) {
    setMode(RF69_MODE_RX);
  }
  freqHz /= RF69_FSTEP; // divide down by FSTEP to get FRF
  writeReg(REG_FRFMSB, freqHz >> 16);
  writeReg(REG_FRFMID, freqHz >> 8);
  writeReg(REG_FRFLSB, freqHz);
  if (oldMode == RF69_MODE_RX) {
    setMode(RF69_MODE_SYNTH);
  }
  setMode(oldMode);
}

void RFM69::setMode(uint8_t newMode)
{
  if (newMode == _mode)
    return;

  switch (newMode) {
    case RF69_MODE_TX:
      writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_TRANSMITTER);
      if (_isRFM69HW) setHighPowerRegs(true);
      break;
    case RF69_MODE_RX:
      writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_RECEIVER);
      if (_isRFM69HW) setHighPowerRegs(false);
      break;
    case RF69_MODE_SYNTH:
      writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_SYNTHESIZER);
      break;
    case RF69_MODE_STANDBY:
      writeReg(REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF |  RF_OPMODE_STANDBY);
      break;
    case RF69_MODE_SLEEP:
      writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_SLEEP);
      break;
    default:
      return;
  }

  // we are using packet mode, so this check is not really needed
  // but waiting for mode ready is necessary when going from sleep because the FIFO may not be immediately available from previous mode
  uint32_t ttt=0;

  while((_mode == RF69_MODE_SLEEP) && ((readReg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00) &&(ttt<100)) {++ttt;}; // wait for ModeReady

  _mode = newMode;
}

//put transceiver in sleep mode to save battery - to wake or resume receiving just call receiveDone()
void RFM69::sleep() {
  ++_lock;
  setMode(RF69_MODE_SLEEP);
  --_lock;
}
void RFM69::idle() {
//  while ((readReg(REG_OPMODE) & RF_OPMODE_SEQUENCER_ON) != RF_OPMODE_SEQUENCER_ON);
 ++_lock;
  setMode(RF69_MODE_STANDBY);
  --_lock;
  _lock=0;
}


//set this node's address
void RFM69::setAddress(uint8_t addr)
{
//  _address = addr;
  writeReg(REG_NODEADRS, _address);
}

//set this node's network id
void RFM69::setNetwork(uint8_t networkID)
{
  writeReg(REG_SYNCVALUE2, networkID);
}

// set *transmit/TX* output power: 0=min, 31=max
// this results in a "weaker" transmitted signal, and directly results in a lower RSSI at the receiver
// the power configurations are explained in the SX1231H datasheet (Table 10 on p21; RegPaLevel p66): http://www.semtech.com/images/datasheet/sx1231h.pdf
// valid powerLevel parameter values are 0-31 and result in a directly proportional effect on the output/transmission power
// this function implements 2 modes as follows:
//       - for RFM69W the range is from 0-31 [-18dBm to 13dBm] (PA0 only on RFIO pin)
//       - for RFM69HW the range is from 0-31 [5dBm to 20dBm]  (PA1 & PA2 on PA_BOOST pin & high Power PA settings - see section 3.3.7 in datasheet, p22)
void RFM69::setPowerLevel(uint8_t powerLevel)
{
//  _powerLevel = (powerLevel > 31 ? 31 : powerLevel);
//  if (_isRFM69HW) _powerLevel /= 2;
  writeReg(REG_PALEVEL, (readReg(REG_PALEVEL) & 0xE0) | powerLevel);
}

bool RFM69::canSend()
{
  if (_mode == RF69_MODE_STANDBY)
     return true;
  if (_lock) return false;
  if (_mode == RF69_MODE_RX /*&& PAYLOADLEN == 0*/ /*&& readRSSI() < CSMA_LIMIT*/) // if signal stronger than -100dBm is detected assume channel activity
  {
//    setMode(RF69_MODE_STANDBY);
    return true;
  }

  return false;
}

bool RFM69::send( const void* buffer, uint8_t bufferSize)
{
  DBGINFO(IRNN);
  DBGINFO("IRNN@@@");
  writeReg(REG_PACKETCONFIG2, (readReg(REG_PACKETCONFIG2) & 0xFB) | RF_PACKET2_RXRESTART); // avoid RX deadlocks
   t_Time _Start = millisT2();
  while (!canSend() && ((millisT2() - _Start) < RF69_CSMA_LIMIT_MS));
  ++_lock;
    // receiveDone();
  sendFrame(0, buffer, bufferSize);
  --_lock;
  return true;
}

// to increase the chance of getting a packet across, call this function instead of send
// and it handles all the ACK requesting/retrying for you :)
// The only twist is that you have to manually listen to ACK requests on the other side and send back the ACKs
// The reason for the semi-automaton is that the lib is interrupt driven and
// requires user action to read the received data and decide what to do with it
// replies usually take only 5..8ms at 50kbps@915MHz


// internal function
void RFM69::sendFrame(uint8_t toAddress, const void* buffer, uint8_t bufferSize)
{
  setMode(RF69_MODE_STANDBY); // turn off receiver to prevent reception while filling fifo
  uint16_t ttt=0;
  while (((readReg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00) &&(ttt<2000)) {++ttt;}; // wait for ModeReady
  writeReg(REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_00); // DIO0 is "Packet Sent"
//  if (bufferSize > RF69_MAX_DATA_LEN) bufferSize = RF69_MAX_DATA_LEN;

  // write to FIFO
  select();
  SPI.transfer(REG_FIFO | 0x80);

  for (uint8_t i = 0; i < bufferSize; i++)
    SPI.transfer(((uint8_t*) buffer)[i]);
  unselect();

  // no need to wait for transmit mode to be ready since its handled by the radio
  setMode(RF69_MODE_TX);
//  IRNN=0;
   t_Time txStart = millisT2();
  while (digitalRead(RF69_IRQ_PIN) == 0 && ((millisT2() - txStart) < RF69_TX_LIMIT_MS)); // wait for DIO0 to turn HIGH signalling transmission finish
  //while (readReg(REG_IRQFLAGS2) & RF_IRQFLAGS2_PACKETSENT == 0x00); // wait for ModeReady
  setMode(RF69_MODE_STANDBY);
   receiveBegin();
}

// internal function - interrupt gets called when a packet is received
void RFM69::interruptHandler() {
//    RSSI = readRSSI(true);
  DBGPINHIGH();
  ++IRNN;
  if (_mode != RF69_MODE_RX) return;
  if (_lock) return;
  ++_lock;
  RSSI = readRSSI();


  uint8_t ii=0;
  uint8_t rr;
   do{
     rr=readReg(REG_IRQFLAGS2);
     ii++;
    }
   while (!(rr & RF_IRQFLAGS2_PAYLOADREADY)|| (ii>28));
// uint8_t rr1=readReg(REG_IRQFLAGS1);

  if ( (rr & RF_IRQFLAGS2_PAYLOADREADY))
  {
    RSSI = readRSSI();
//    rr=readReg(REG_IRQFLAGS2);
         rr=readReg(REG_IRQFLAGS2);
    setMode(RF69_MODE_STANDBY);
   // PAYLOADLEN=RF69_FRAME_LEN;

//    RSSI = readRSSI();
    select();
    SPI.transfer(REG_FIFO & 0x7F);

    uint8_t xx=(_head & 0x03)*RF69_FRAME_LEN;
    for (uint8_t i = 0; i < RF69_FRAME_LEN ; i++)
    {
      DATA[xx++] = SPI.transfer(0);
    }
    _head++;

    unselect();

   IRQ2=rr;
 //  RX_time=millisTNow();
   receiveMode();
   receivedData(RF69_FRAME_LEN);
  } else{

    DBGINFO("****");
  }
       --_lock;

}

// internal function
void RFM69::isr0() { selfPointer->interruptHandler(); }

// internal function
void RFM69::receiveBegin() {
//    setMode(RF69_MODE_STANDBY);
//  RSSI = readRSSI(true);
//  if (readReg(REG_IRQFLAGS2) & RF_IRQFLAGS2_PAYLOADREADY)
//    writeReg(REG_PACKETCONFIG2, (readReg(REG_PACKETCONFIG2) & 0xFB) | RF_PACKET2_RXRESTART); // avoid RX deadlocks
  receiveMode();
//  listenMode();
}

// checks if a packet was received and/or puts transceiver in receive (ie RX or listen) mode
bool RFM69::canRead() {
 // if (_mode == RF69_MODE_RX && PAYLOADLEN > 0)
 //     return true;
 // return false;
 return (_head!=_tail);
}
void RFM69::receiveMode(){
  writeReg(REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_01); // set DIO0 to "PAYLOADREADY" in receive mode
  setMode(RF69_MODE_RX);
}

void RFM69::listenMode(){
  setMode(RF69_MODE_STANDBY);

  writeReg(REG_LISTEN1, RF_LISTEN1_RESOL_64 | RF_LISTEN1_RESOL_IDLE_64 |RF_LISTEN1_CRITERIA_RSSI |RF_LISTEN1_END_00); // set DIO0 to "PAYLOADREADY" in receive mode
  writeReg(REG_LISTEN2, 72);
  writeReg(REG_LISTEN3, 10);
  _mode=RF69_MODE_RX;
   writeReg(REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_ON | RF_OPMODE_STANDBY );
}

bool RFM69::receiveDone() {
//ATOMIC_BLOCK(ATOMIC_FORCEON)
//{
//  noInterrupts(); // re-enabled in unselect() via setMode() or via receiveBegin()
  if (_mode == RF69_MODE_RX )
  {
//    Serial.print("<>");
    setMode(RF69_MODE_STANDBY); // enables interrupts
    return true;
  }
  else if (_mode == RF69_MODE_RX) // already in RX no payload yet
  {
    return false;
  }
  receiveBegin();
  return false;
}

// To enable encryption: radio.encrypt("ABCDEFGHIJKLMNOP");
// To disable encryption: radio.encrypt(null) or radio.encrypt(0)
// KEY HAS TO BE 16 bytes !!!
void RFM69::encrypt(const char* key) {
  setMode(RF69_MODE_STANDBY);
  if (key != 0)
  {
    select();
    SPI.transfer(REG_AESKEY1 | 0x80);
    for (uint8_t i = 0; i < 16; i++)
      SPI.transfer(key[i]);
    unselect();
  }
  writeReg(REG_PACKETCONFIG2, (readReg(REG_PACKETCONFIG2) & 0xFE) | (key ? 1 : 0));
}

// get the received signal strength indicator (RSSI)
int16_t RFM69::readRSSI(bool forceTrigger) {
  int16_t rssi = 0;
  if (forceTrigger)
  {
    DBGINFO("*");
    // RSSI trigger not needed if DAGC is in continuous mode
    writeReg(REG_RSSICONFIG, RF_RSSI_START);
    while ((readReg(REG_RSSICONFIG) & RF_RSSI_DONE) == 0x00); // wait for RSSI_Ready
  }
  rssi = readReg(REG_RSSIVALUE);
  if (rssi==255){
//    while ((readReg(REG_RSSICONFIG) & RF_RSSI_DONE) == 0x00); // wait for RSSI_Ready
    rssi = readReg(REG_RSSIVALUE);
  }
  return rssi;
}

uint8_t RFM69::readReg(uint8_t addr)
{
  select();
  SPI.transfer(addr & 0x7F);
  uint8_t regval = SPI.transfer(0);
  unselect();
  return regval;
}

void RFM69::writeReg(uint8_t addr, uint8_t value)
{
  select();
  SPI.transfer(addr | 0x80);
  SPI.transfer(value);
  unselect();
}

// select the RFM69 transceiver (save SPI settings, set CS low)
void RFM69::select() {
//  noInterrupts();
  // save current SPI settings
//  _SPCR = SPCR;
//  _SPSR = SPSR;
  // set RFM69 SPI settings
//  SPI.setDataMode(SPI_MODE0);
//  SPI.setBitOrder(MSBFIRST);
//  SPI.setClockDivider(SPI_CLOCK_DIV2); // decided to slow down from DIV2 after SPI stalling in some instances, especially visible on mega1284p when RFM69 and FLASH chip both present
  digitalWrite(_slaveSelectPin, LOW);
}

// unselect the RFM69 transceiver (set CS high, restore SPI settings)
void RFM69::unselect() {
  digitalWrite(_slaveSelectPin, HIGH);
  // restore SPI settings to what they were before talking to RFM69
//  SPCR = _SPCR;
//  SPSR = _SPSR;
//  interrupts();
}

// true  = disable filtering to capture all frames on network
// false = enable node/broadcast filtering to capture only frames sent to this/broadcast address
void RFM69::promiscuous(bool onOff) {
  _promiscuousMode = onOff;
  //writeReg(REG_PACKETCONFIG1, (readReg(REG_PACKETCONFIG1) & 0xF9) | (onOff ? RF_PACKET1_ADRSFILTERING_OFF : RF_PACKET1_ADRSFILTERING_NODEBROADCAST));
}

// for RFM69HW only: you must call setHighPower(true) after initialize() or else transmission won't work
void RFM69::setChannel(uint8_t channel){
    uint8_t freqMSB=0xD9;
    uint8_t freqMID=0x0;
    uint8_t freqLSB=0x0;
  if (channel==0) return;
  if (channel==2) freqMID=0x20;
  if (channel==3) freqMID=0x30;
  if (channel==4) freqMID=0x40;

  writeReg(REG_FRFMSB, freqMSB );
  writeReg(REG_FRFMID, freqMID );
  writeReg(REG_FRFLSB, freqLSB);

}
void RFM69::setHighPower(bool onOff) {
  _isRFM69HW = true;
//  writeReg(REG_OCP, _isRFM69HW ? RF_OCP_OFF : RF_OCP_ON);
  //writeReg(REG_OCP, RF_OCP_ON | RF_OCP_TRIM_95 );
  if (onOff) // turning ON
  //    {REG_PALEVEL, RF_PALEVEL_PA0_OFF | RF_PALEVEL_PA1_ON | RF_PALEVEL_PA2_ON  | RF_PALEVEL_OUTPUTPOWER_11111}, // enable P1 & P2 amplifier stages

//    writeReg(REG_PALEVEL, (readReg(REG_PALEVEL) & 0x1F) | RF_PALEVEL_PA1_ON | RF_PALEVEL_PA2_ON); // enable P1 & P2 amplifier stages
    writeReg(REG_PALEVEL, RF_PALEVEL_PA0_OFF | RF_PALEVEL_PA1_ON | RF_PALEVEL_PA2_OFF  | RF_PALEVEL_OUTPUTPOWER_10111); // enable P1 & P2 amplifier stages
  else
    writeReg(REG_PALEVEL, RF_PALEVEL_PA0_ON | RF_PALEVEL_PA1_OFF | RF_PALEVEL_PA2_OFF | 28); // enable P0 only

//  /* 0x07 */ writeReg( REG_FRFMSB, 0xD9);
//    /* 0x08 */ writeReg( REG_FRFMID, 0x70);
 //   /* 0x09 */ writeReg( REG_FRFLSB, 0x10 );
//   setHighPowerRegs(onOff);
}

// internal function
void RFM69::setHighPowerRegs(bool onOff) {
  writeReg(REG_TESTPA1, onOff ? 0x5D : 0x55);
  writeReg(REG_TESTPA2, onOff ? 0x7C : 0x70);
}


//for debugging
#if DBGLVL>=1
#define REGISTER_DETAIL 1
#endif


#if REGISTER_DETAIL
// SERIAL PRINT
// replace Serial.print("string") with SerialPrint("string")
#define SerialPrint(x) SerialPrint_P(PSTR(x))
void SerialWrite ( uint8_t c ) {
    Serial.write ( c );
}

void SerialPrint_P(PGM_P str, void (*f)(uint8_t) = SerialWrite ) {
  for (uint8_t c; (c = pgm_read_byte(str)); str++) (*f)(c);
}
#endif

void RFM69::readAllRegs()
{
#if DBGLVL >= 1
  uint8_t regVal;

#if REGISTER_DETAIL
  int capVal;

  //... State Variables for intelligent decoding
  uint8_t modeFSK = 0;
  int bitRate = 0;
  int freqDev = 0;
  long freqCenter = 0;
#endif

  Serial.println("Address - HEX - BIN");
  for (uint8_t regAddr = 1; regAddr <= 0x4F; regAddr++)
  {
    select();
    SPI.transfer(regAddr & 0x7F); // send address + r/w bit
    regVal = SPI.transfer(0);
    unselect();

    Serial.print(regAddr, HEX);
    Serial.print(" - ");
    Serial.print(regVal,HEX);
    Serial.print(" - ");
    Serial.println(regVal,BIN);

#if REGISTER_DETAIL
    switch ( regAddr )
    {
        case 0x1 : {
            SerialPrint ( "Controls the automatic Sequencer ( see section 4.2 )\nSequencerOff : " );
            if ( 0x80 & regVal ) {
                SerialPrint ( "1 -> Mode is forced by the user\n" );
            } else {
                SerialPrint ( "0 -> Operating mode as selected with Mode bits in RegOpMode is automatically reached with the Sequencer\n" );
            }

            SerialPrint( "\nEnables Listen mode, should be enabled whilst in Standby mode:\nListenOn : " );
            if ( 0x40 & regVal ) {
                SerialPrint ( "1 -> On\n" );
            } else {
                SerialPrint ( "0 -> Off ( see section 4.3)\n" );
            }

            SerialPrint( "\nAborts Listen mode when set together with ListenOn=0 See section 4.3.4 for details (Always reads 0.)\n" );
            if ( 0x20 & regVal ) {
                SerialPrint ( "ERROR - ListenAbort should NEVER return 1 this is a write only register\n" );
            }

            SerialPrint("\nTransceiver's operating modes:\nMode : ");
            capVal = (regVal >> 2) & 0x7;
            if ( capVal == 0b000 ) {
                SerialPrint ( "000 -> Sleep mode (SLEEP)\n" );
            } else if ( capVal = 0b001 ) {
                SerialPrint ( "001 -> Standby mode (STDBY)\n" );
            } else if ( capVal = 0b010 ) {
                SerialPrint ( "010 -> Frequency Synthesizer mode (FS)\n" );
            } else if ( capVal = 0b011 ) {
                SerialPrint ( "011 -> Transmitter mode (TX)\n" );
            } else if ( capVal = 0b100 ) {
                SerialPrint ( "100 -> Receiver Mode (RX)\n" );
            } else {
                Serial.print( capVal, BIN );
                SerialPrint ( " -> RESERVED\n" );
            }
            SerialPrint ( "\n" );
            break;
        }

        case 0x2 : {

            SerialPrint("Data Processing mode:\nDataMode : ");
            capVal = (regVal >> 5) & 0x3;
            if ( capVal == 0b00 ) {
                SerialPrint ( "00 -> Packet mode\n" );
            } else if ( capVal == 0b01 ) {
                SerialPrint ( "01 -> reserved\n" );
            } else if ( capVal == 0b10 ) {
                SerialPrint ( "10 -> Continuous mode with bit synchronizer\n" );
            } else if ( capVal == 0b11 ) {
                SerialPrint ( "11 -> Continuous mode without bit synchronizer\n" );
            }

            SerialPrint("\nModulation scheme:\nModulation Type : ");
            capVal = (regVal >> 3) & 0x3;
            if ( capVal == 0b00 ) {
                SerialPrint ( "00 -> FSK\n" );
                modeFSK = 1;
            } else if ( capVal == 0b01 ) {
                SerialPrint ( "01 -> OOK\n" );
            } else if ( capVal == 0b10 ) {
                SerialPrint ( "10 -> reserved\n" );
            } else if ( capVal == 0b11 ) {
                SerialPrint ( "11 -> reserved\n" );
            }

            SerialPrint("\nData shaping: ");
            if ( modeFSK ) {
                SerialPrint( "in FSK:\n" );
            } else {
                SerialPrint( "in OOK:\n" );
            }
            SerialPrint ("ModulationShaping : ");
            capVal = regVal & 0x3;
            if ( modeFSK ) {
                if ( capVal == 0b00 ) {
                    SerialPrint ( "00 -> no shaping\n" );
                } else if ( capVal == 0b01 ) {
                    SerialPrint ( "01 -> Gaussian filter, BT = 1.0\n" );
                } else if ( capVal == 0b10 ) {
                    SerialPrint ( "10 -> Gaussian filter, BT = 0.5\n" );
                } else if ( capVal == 0b11 ) {
                    SerialPrint ( "11 -> Gaussian filter, BT = 0.3\n" );
                }
            } else {
                if ( capVal == 0b00 ) {
                    SerialPrint ( "00 -> no shaping\n" );
                } else if ( capVal == 0b01 ) {
                    SerialPrint ( "01 -> filtering with f(cutoff) = BR\n" );
                } else if ( capVal == 0b10 ) {
                    SerialPrint ( "10 -> filtering with f(cutoff) = 2*BR\n" );
                } else if ( capVal == 0b11 ) {
                    SerialPrint ( "ERROR - 11 is reserved\n" );
                }
            }

            SerialPrint ( "\n" );
            break;
        }

        case 0x3 : {
            bitRate = (regVal << 8);
            break;
        }

        case 0x4 : {
            bitRate |= regVal;
            SerialPrint ( "Bit Rate (Chip Rate when Manchester encoding is enabled)\nBitRate : ");
            unsigned long val = 32UL * 1000UL * 1000UL / bitRate;
            Serial.println( val );
            SerialPrint( "\n" );
            break;
        }

        case 0x5 : {
            freqDev = ( (regVal & 0x3f) << 8 );
            break;
        }

        case 0x6 : {
            freqDev |= regVal;
            SerialPrint( "Frequency deviation\nFdev : " );
            unsigned long val = 61UL * freqDev;
            Serial.println( val );
            SerialPrint ( "\n" );
            break;
        }

        case 0x7 : {
            unsigned long tempVal = regVal;
            freqCenter = ( tempVal << 16 );
            break;
        }

        case 0x8 : {
            unsigned long tempVal = regVal;
            freqCenter = freqCenter | ( tempVal << 8 );
            break;
        }

        case 0x9 : {
            freqCenter = freqCenter | regVal;
            SerialPrint ( "RF Carrier frequency\nFRF : " );
            unsigned long val = 61UL * freqCenter;
            Serial.println( val );
            SerialPrint( "\n" );
            break;
        }

        case 0xa : {
            SerialPrint ( "RC calibration control & status\nRcCalDone : " );
            if ( 0x40 & regVal ) {
                SerialPrint ( "1 -> RC calibration is over\n" );
            } else {
                SerialPrint ( "0 -> RC calibration is in progress\n" );
            }

            SerialPrint ( "\n" );
            break;
        }

        case 0xb : {
            SerialPrint ( "Improved AFC routine for signals with modulation index lower than 2.  Refer to section 3.4.16 for details\nAfcLowBetaOn : " );
            if ( 0x20 & regVal ) {
                SerialPrint ( "1 -> Improved AFC routine\n" );
            } else {
                SerialPrint ( "0 -> Standard AFC routine\n" );
            }
            SerialPrint ( "\n" );
            break;
        }

        case 0xc : {
            SerialPrint ( "Reserved\n\n" );
            break;
        }

        case 0xd : {
            byte val;
            SerialPrint ( "Resolution of Listen mode Idle time (calibrated RC osc):\nListenResolIdle : " );
            val = regVal >> 6;
            if ( val == 0b00 ) {
                SerialPrint ( "00 -> reserved\n" );
            } else if ( val == 0b01 ) {
                SerialPrint ( "01 -> 64 us\n" );
            } else if ( val == 0b10 ) {
                SerialPrint ( "10 -> 4.1 ms\n" );
            } else if ( val == 0b11 ) {
                SerialPrint ( "11 -> 262 ms\n" );
            }

            SerialPrint ( "\nResolution of Listen mode Rx time (calibrated RC osc):\nListenResolRx : " );
            val = (regVal >> 4) & 0x3;
            if ( val == 0b00 ) {
                SerialPrint ( "00 -> reserved\n" );
            } else if ( val == 0b01 ) {
                SerialPrint ( "01 -> 64 us\n" );
            } else if ( val == 0b10 ) {
                SerialPrint ( "10 -> 4.1 ms\n" );
            } else if ( val == 0b11 ) {
                SerialPrint ( "11 -> 262 ms\n" );
            }

            SerialPrint ( "\nCriteria for packet acceptance in Listen mode:\nListenCriteria : " );
            if ( 0x8 & regVal ) {
                SerialPrint ( "1 -> signal strength is above RssiThreshold and SyncAddress matched\n" );
            } else {
                SerialPrint ( "0 -> signal strength is above RssiThreshold\n" );
            }

            SerialPrint ( "\nAction taken after acceptance of a packet in Listen mode:\nListenEnd : " );
            val = (regVal >> 1 ) & 0x3;
            if ( val == 0b00 ) {
                SerialPrint ( "00 -> chip stays in Rx mode. Listen mode stops and must be disabled (see section 4.3)\n" );
            } else if ( val == 0b01 ) {
                SerialPrint ( "01 -> chip stays in Rx mode until PayloadReady or Timeout interrupt occurs.  It then goes to the mode defined by Mode. Listen mode stops and must be disabled (see section 4.3)\n" );
            } else if ( val == 0b10 ) {
                SerialPrint ( "10 -> chip stays in Rx mode until PayloadReady or Timeout occurs.  Listen mode then resumes in Idle state.  FIFO content is lost at next Rx wakeup.\n" );
            } else if ( val == 0b11 ) {
                SerialPrint ( "11 -> Reserved\n" );
            }


            SerialPrint ( "\n" );
            break;
        }

        default : {
        }
    }
#endif
  }
  unselect();
  #endif
}

uint8_t RFM69::readTemperature(uint8_t calFactor) // returns centigrade
{

  uint8_t oldMode = _mode;
  setMode(RF69_MODE_STANDBY);
  writeReg(REG_TEMP1, RF_TEMP1_MEAS_START);
  while ((readReg(REG_TEMP1) & RF_TEMP1_MEAS_RUNNING));
  return ~readReg(REG_TEMP2) + COURSE_TEMP_COEF + calFactor; // 'complement' corrects the slope, rising temp = rising val
//  setMode(oldMode);


} // COURSE_TEMP_COEF puts reading in the ballpark, user can add additional correction

void RFM69::rcCalibration()
{
  writeReg(REG_OSC1, RF_OSC1_RCCAL_START);
  while ((readReg(REG_OSC1) & RF_OSC1_RCCAL_DONE) == 0x00);
}
