/*
 Copyright (C) 2015 Dariusz Mazur <darekm@emadar.com>

 */
 
/**
 * @file me_frame.h
 *
 * Setup necessary to direct stdout to imwave library
 */

#include "imatmega.h"

// http://code.google.com/p/tinkerit/wiki/SecretVoltmeter
long internalVccOld() {
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

//http://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
uint16_t internalVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

uint16_t internalTemp() {
    //Enable the Temp sensor. 0bxx0yyyyy sets mode.
    // xx is the reference, set to internal 2.56v as per datasheet
    // yyyyy is the lower 5 bits of the mux value so 00111 as per what you found

  // routine provided by TCWORLD in Sparkfun forums - thank you very much TCWORLD!
  // see here for details: http://forum.sparkfun.com/viewtopic.php?f=32&t=32433
  ADMUX = 0b11000111;                  // set the adc to compare the internal temp sensor against
  ADCSRB |= (1 << MUX5);               // the 2.56v internal reference (datasheet section 24.6)
  delay(5);                            // wait a moment for the adc to settle
//  sbi(ADCSRA, ADSC);                   // initiate the first conversion - this one is junk as per datasheet
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));    // wait for the conversion to finish
//  delay(5);                            // another little extra pause just in case
//  sbi(ADCSRA, ADSC);                   // initiate the second conversion - this is the good one
//  while (bit_is_set(ADCSRA, ADSC));    // wait for the conversion to finish
  byte low  = ADCL;                    // read the low byte
  byte high = ADCH;                    // read the high byte
  int temperature = (high << 8) | low;     // result is the absolute temperature in Kelvin * i think *
  temperature = temperature - 273;     // subtract 273 to get the degrees C
  return temperature;
}

#else

long internalTemp328() {
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



#endif

int freeRam ()
{
      extern int __heap_start, *__brkval;
      int v;
      return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}


uint16_t internalrandom() {
  long x= internalVcc();
  x=x+micros();
  x=x +(x >>7);
  return x;

}

void reboot() {
  wdt_disable();
  do{
  wdt_enable(WDTO_15MS);
  for(;;){};
//  while (1) {}
  }while(0);
}

#define soft_restart()        \
do                          \
{                           \
    wdt_enable(WDTO_15MS);  \
    for(;;)                 \
    {                       \
    }                       \
} while(0)


//
// END OF FILE
//
