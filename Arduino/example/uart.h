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
unsigned short seqnr = 0;
static uint8_t lasthop=0;//counter for retries


void shiftUartBuffer(unsigned short x)
{
    Serial.print("Shift");
    Serial.print(uartBufLen);
    Serial.print(" ");
    Serial.println(x);

     uartBufLen -= x;
      for (unsigned short i=0 ; i<uartBufLen ; i++ )
      {
        uartBuf[i] = uartBuf[x+i];
      }
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
      seqnr++;  //new data -> increase sequence number
      frame.Header.Sequence = seqnr;
      byte x=frame.Put((uint8_t *)uartBuf,uartBufLen);

      shiftUartBuffer(x);

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
