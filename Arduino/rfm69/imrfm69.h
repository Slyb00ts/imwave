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
#ifndef RFM69_h
#define RFM69_h
//#include <Arduino.h>            // assumes Arduino IDE v1.0 or greater
#include "SPI.h"
#include "imatmega.h"
#include "imdebug.h"

#define RF69_MAX_DATA_LEN       61 // to take advantage of the built in AES/CRC we want to limit the frame size to the internal FIFO size (66 bytes - 3 bytes overhead - 2 bytes crc)
#define RF69_SPI_CS             SS // SS is the SPI slave select pin, for instance D10 on ATmega328
#define RF69_BUF_LEN            128
#define RF69_FRAME_LEN           32
// INT0 on AVRs should be connected to RFM69's DIO0 (ex on ATmega328 it's D2, on ATmega644/1284 it's D2)
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega88) || defined(__AVR_ATmega8__) || defined(__AVR_ATmega88__)
  #define RF69_IRQ_PIN          3  //D2  D3
  #define RF69_IRQ_NUM          1  // 0   1
  #define RF69_RESET_PIN        4
#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
  #define RF69_IRQ_PIN          2
  #define RF69_IRQ_NUM          2
#elif defined(__AVR_ATmega32U4__)
  #define RF69_IRQ_PIN          3
  #define RF69_IRQ_NUM          0
#else
  #define RF69_IRQ_PIN          3
  #define RF69_IRQ_NUM          1
#endif


#define CSMA_LIMIT              -90 // upper RX signal sensitivity threshold in dBm for carrier sense access
#define RF69_MODE_SLEEP         0 // XTAL OFF
#define RF69_MODE_STANDBY       1 // XTAL ON
#define RF69_MODE_SYNTH         2 // PLL ON
#define RF69_MODE_RX            3 // RX MODE
#define RF69_MODE_TX            4 // TX MODE

// available frequency bands
#define RF69_315MHZ            31 // non trivial values to avoid misconfiguration
#define RF69_433MHZ            43
#define RF69_868MHZ            86
#define RF69_915MHZ            91

#define null                  0
#define COURSE_TEMP_COEF    -90 // puts the temperature reading in the ballpark, user can fine tune the returned value
#define RF69_BROADCAST_ADDR 255
#define RF69_CSMA_LIMIT_MS 1000
#define RF69_TX_LIMIT_MS   100
#define RF69_FSTEP  61.03515625 // == FXOSC / 2^19 = 32MHz / 2^19 (p13 in datasheet)

// TWS: define CTLbyte bits
#define RFM69_CTL_SENDACK   0x80
#define RFM69_CTL_REQACK    0x40
typedef void( * funTransceiverRF69 )(byte );

class RFM69 {
  private:
     static void receivedDataNull(byte){};
    void listenMode();
    void receiveMode();
    static volatile uint8_t _lock;

  public:
    static volatile uint8_t DATA[RF69_BUF_LEN]; // recv/xmit buf, including header & crc bytes
    static volatile uint8_t IRQ2;
    static volatile uint8_t IRNN;
    static volatile uint8_t _head;
    static volatile uint8_t _tail;
//    static volatile uint8_t SENDERID;
//    static volatile uint8_t TARGETID; // should match _address
//    static volatile uint8_t PAYLOADLEN;
//    static volatile uint8_t ACK_REQUESTED;
//    static volatile uint8_t ACK_RECEIVED; // should be polled immediately after sending a packet with ACK request
    static volatile int16_t RSSI; // most accurate RSSI during reception (closest to the reception)
    static volatile uint8_t _mode; // should be protected?
    funTransceiverRF69 receivedData;

    RFM69(uint8_t slaveSelectPin=RF69_SPI_CS, /*uint8_t interruptPin=RF69_IRQ_PIN,*/ bool isRFM69HW=true) {
      _slaveSelectPin = slaveSelectPin;
//      _interruptPin = interruptPin;
//      _interruptNum = interruptNum;
      _mode = RF69_MODE_STANDBY;
      _promiscuousMode = false;
      _powerLevel = 28;
      _isRFM69HW = isRFM69HW;
      receivedData=&receivedDataNull;
    }

    bool initialize(uint8_t freqBand, uint8_t ID, uint8_t networkID=1);
    void setAddress(uint8_t addr);
    void setNetwork(uint8_t networkID);
    bool canSend();
    bool canRead();
    bool send( const void* buffer, uint8_t bufferSize);
    virtual bool receiveDone();
    virtual void receiveBegin();
    uint32_t getFrequency();
    void setFrequency(uint32_t freqHz);
    void encrypt(const char* key);
    int16_t readRSSI(bool forceTrigger=false);
    void promiscuous(bool onOff=true);
    virtual void setHighPower(bool onOFF=true); // has to be called after initialize() for RFM69HW
    virtual void setPowerLevel(uint8_t level); // reduce/increase transmit power level
    void sleep();
    void idle();
    void reset();
    uint8_t readTemperature(uint8_t calFactor=0); // get CMOS temperature (8bit)
    void rcCalibration(); // calibrate the internal RC oscillator for use in wide temperature variations - see datasheet section [4.3.5. RC Timer Accuracy]

    // allow hacking registers by making these public
    uint8_t readReg(uint8_t addr);
    void writeReg(uint8_t addr, uint8_t val);
    void readAllRegs();

  protected:
    static void isr0();
    void virtual interruptHandler();
    virtual void interruptHook(uint8_t CTLbyte) {};
    virtual void sendFrame(uint8_t toAddress, const void* buffer, uint8_t size);

    static RFM69* selfPointer;
    uint8_t _slaveSelectPin;
    uint8_t _interruptPin;
    uint8_t _address;
    bool _promiscuousMode;
    uint8_t _powerLevel;
    bool _isRFM69HW;
//    uint8_t _SPCR;
//    uint8_t _SPSR;

    virtual void setMode(uint8_t mode);
    virtual void setHighPowerRegs(bool onOff);
    virtual void select();
    virtual void unselect();

};

#endif
