/*
 Copyright (C) 2015 Dariusz Mazur <darekm@emadar.com>

 */
 
/**
 * @file me_frame.h
 *
 * Setup necessary to direct stdout to imwave library
 */

#include "imatmega.h"


volatile long counterTimer2=0;

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

long millisT2(){
 #if defined(__sleepT2)
   return counterTimer2;
 #else
   return millis();
 #endif
}

long incTimer2(){
  return counterTimer2++;
}

void setSleepModeT2()
{
 #if defined(__sleepT22)
    set_sleep_mode(SLEEP_MODE_PWR_SAVE);
 #else
    set_sleep_mode (SLEEP_MODE_IDLE);
 #endif
}

int freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}


uint16_t internalrandom()
{
  long x= internalVcc();
  x=x+micros();
  x=x +(x >>7);
  return x;
}


void delaySleep( unsigned long t)
{
  unsigned long startMillis = millis();
  unsigned long current;

  set_sleep_mode (SLEEP_MODE_IDLE);
  do
  {
    sleep_mode();
    current = millis();
  }
  while( (current - startMillis) <= t);
}

void delayT2()
{
    if (F_CPU==16000000L)
       incTimer2();
    goSleep();
}

void delaySleepT2( unsigned long t)
{
 #if defined(__sleepT2)

   unsigned long startMillis = millisT2();
   setSleepModeT2();
   do
   {
//    if (F_CPU==16000000L)
       incTimer2();
      sleep_mode();
   }
   while( (millisT2() - startMillis) <= t);
 #else
   delaySleep(t);
 #endif
}


void enterSleep(void)
 {
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  sleep_enable();
  sleep_mode();
  /** The program will continue from here. **/
  /* First thing to do is disable sleep. */
  sleep_disable();
 }

#if defined(__AVR_ATmega328P__)
void disableADCB()
{
 // disable ADC
  ADCSRA = 0;
   // turn off brown-out enable in software
  MCUCR = bit (BODS) | bit (BODSE);  // turn on brown-out enable select
  MCUCR = bit (BODS);        // this must be done within 4 clock cycles of above
//ower Reduction Register (PRR)
    power_adc_disable(); // ADC converter
//    power_spi_enable(); // SPI
//    power_usart0_enable(); // Serial (USART)
//    power_timer0_enable(); // Timer 0
//    power_timer1_enable(); // Timer 1
//    power_timer2_enable(); // Timer 2
    power_twi_disable(); // TWI (I2C)

}


void setupTimer2()
{
//http://electronics.stackexchange.com/questions/26363/how-do-i-create-a-timer-interrupt-with-arduino
  TCCR2B = 0x00;        //Disable Timer2 while we set it up
  TIFR2  = 0x00;        //Timer2 INT Flag Reg: Clear Timer Overflow Flag

   /* First disable the timer overflow interrupt while we're configuring */
  TIMSK2 &= ~(1<<TOIE2);

  /* Configure timer2 in normal mode (pure counting, no PWM etc.) */
  TCCR2A &= ~((1<<WGM21) | (1<<WGM20));

  TCCR2A |= ((1<<WGM21) );

  TCCR2B &= ~(1<<WGM22);

  /* Select clock source: internal I/O clock */
  ASSR &= ~(1<<AS2);

  /* Disable Compare Match A interrupt enable (only want overflow) */
  TIMSK2 &= ~(1<<OCIE2A);

  /* Now configure the prescaler to CPU clock divided by 128 */
  TCCR2B |= (1<<CS22)  | (1<<CS20); // Set bits
  TCCR2B &= ~(1<<CS21);             // Clear bit
  if (F_CPU==8000000L)
    TCCR2B &= ~(1<<CS20);             // Clear bit

  /* We need to calculate a proper value to load the timer counter.
   * The following loads the value 131 into the Timer 2 counter register
   * The math behind this is:
   * (CPU frequency) / (prescaler value) = 125000 Hz = 8us.
   * (desired period) / 8us = 125.
   * MAX(uint8) + 1 - 125 = 131;
   */
  /* Save value globally for later reload in ISR */

  //Setup Timer2 to fire every 1ms
  /* Finally load end enable the timer */
  OCR2A= 124;
  // (clock /prescaler*desired_time )-1
  // 16000000 /126*0.001 = 125

//  TCNT2 = counterTCNT2;
//  TIMSK2 |= (1<<TOIE2);
  TIMSK2 |= (1<<OCIE2A);


//  TIFR2  = 0x00;        //Timer2 INT Flag Reg: Clear Timer Overflow Flag
//  TIMSK2 = 0x01;        //Timer2 INT Reg: Timer2 Overflow Interrupt Enable
//  TCCR2A = 0x00;        //Timer2 Control Reg A: Wave Gen Mode normal
//  TCCR2B = 0x05;        //Timer2 Control Reg B: Timer Prescaler set to 128
}

#endif


void pciSetup(uint8_t pin)
{
//http://arduinomega.blogspot.com/2011/05/setting-interrupts-manually-real-int0.html
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

void(* SoftResetFuncA) (void) = 0; //declare reset function @ address 0

void SoftReset(){
  SoftResetFuncA();
}

void reboot(){
//http://wiblocks.luciani.org/docs/app-notes/software-reset.html
//nedeed modification of bootloader
  cli();
  wdt_disable();
  do{
  wdt_enable(WDTO_15MS);
  delay(100);
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
