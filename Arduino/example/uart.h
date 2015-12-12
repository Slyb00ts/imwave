// 
//    FILE: dht.h
// VERSION: 0.1.00
// PURPOSE: Transiver for Arduino
//
//
// HISTORY:
//

#ifndef imUart_h
#define imUart_h

#include "Arduino.h"
#include "imframe.h"
#include "imdebug.h"


#define UART_BUFFSIZE 125  //Buffer for UART
#define UART_SENDTHRES 110  //Start sending when this full

char uartBuf[UART_BUFFSIZE] = {0,};
unsigned short uartBufLen = 0;
static uint8_t lasthop=0;//counter for retries


void shiftUartBuffer(unsigned short x)
{
//    DBGINFO("ShiftUart");
//    DBGINFO(uartBufLen);
//    DBGINFO(" ");
//    DBGINFO(x);

     uartBufLen -= x;
      for (unsigned short i=0 ; i<uartBufLen ; i++ )
      {
        uartBuf[i] = uartBuf[x+i];
      }
}

long internalVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

long internalTemp() {
//  https://code.google.com/p/tinkerit/wiki/SecretThermometer
  long result;
  // Read temperature sensor against 1.1V reference
  ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX3);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = (result - 125) * 1075;
  return result;
}


void generatorUart()
{
    uartBuf[uartBufLen++] = 'A';
    uartBuf[uartBufLen++] = 'B';
    uartBuf[uartBufLen++] = 'C';
    uartBuf[uartBufLen++] = lasthop++;
   if (uartBufLen>40) uartBufLen=1;

}

void UartPrepareData(IMFrame &frame)
{
  IMFrameData *dt=frame.Data();
  DBGINFO("ml:");
  DBGINFO(millis());
  dt->w[0]=millis();
  dt->w[1]=millis()>>16;
//  long x =internalVcc();
  long x=500;
  DBGINFO("vcc:");
  DBGINFO(x);
  dt->w[2]=x;
  dt->w[3]=x >>16;
//  x =internalTemp();
  dt->w[4]=x;
  dt->w[5]=x >>16;
  DBGINFO("tmo:");
  DBGINFO(x);
  DBGINFO("\r\n");

 //   shiftUartBuffer(x);

}

/************** UART to radio ************************/

/*
  //read incoming chars from UART
  crc = bridge.available();
  while(uartBufLen<UART_BUFFSIZE && crc)  //still have bufferspace and remaining chars on uart
  {
    cnt = bridge.read();
    crc--;
    //last char within timeout OR fresh block
    if (uart_timeout>millis() || uartBufLen==0)
    {
      uartBuf[uartBufLen++] = cnt;
      uart_timeout = millis()+UART_TIMEOUT;
    }
    if (!crc) crc = bridge.available();
  }
  */



#endif
//
// END OF FILE
//
