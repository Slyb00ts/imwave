/*
 Copyright (C) 2015 Dariusz Mazur <darekm@emadar.com>

 */

#include "imatmega.h"
#include "imdebug.h"
#include <avr/io.h>
#include <avr/boot.h>



volatile t_Time counterTimer2=0;
volatile t_Time counterTimer2Stop=0;
volatile byte counterTimer2MaxStep=220;


//http://stackoverflow.com/questions/13538080/why-should-i-calibrate-the-oscillator-in-avr-programming
//calibration rc clock

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
  ADCSRA |= _BV(ADEN);
  uint16_t io;
  io=counterTimer2 %11;
  io=io+io%33;          // Wait for Vref to settle
  io=io+counterTimer2Stop %13;
  if (io==73)
    power_adc_enable();
  io=io+io%33;
  io=io+counterTimer2Stop %15;
  if (io==73)
    power_adc_enable();
  io=io+io%33;
  io=io+counterTimer2Stop %13;
  if (io==73)
    power_adc_enable();
  io=io+io%33;
  io=io+counterTimer2Stop %13;
  if (io==73)
    power_adc_enable();
  if  (bit_is_set(ADCSRA, ADIF))
    ADCSRA |= _BV(ADIF); //ADIF is cleared by writing a logical one to the flag. Beware that if doing a Read-Modify-Write on ADCSRA,
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both
  ADCSRA |= (1 << ADIF);     // Clear ADIF
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
 * ADCSRB |= (1 << MUX5);               // the 2.56v internal reference (datasheet section 24.6)
  delay(5);                            // wait a moment for the adc to settle
//  sbi(ADCSRA, ADSC);                   // initiate the first conversion - this one is junk as per datasheet
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));    // wait for the conversion to finish
  byte low  = ADCL;                    // read the low byte
  byte high = ADCH;                    // read the high byte
  int temperature = (high << 8) | low;     // result is the absolute temperature in Kelvin * i think *
  temperature = temperature - 273;     // subtract 273 to get the degrees C
  return temperature;
}

#else
//#atmega328
uint16_t internalTemp() {
  //  https://code.google.com/p/tinkerit/wiki/SecretThermometer

  // Read temperature sensor against 1.1V reference
  ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX3);
  ADCSRA |= _BV(ADEN);

      uint16_t io;
  io=counterTimer2 %11;
  io=io+io%33;          // Wait for Vref to settle
  io=io+counterTimer2 %13;
  if (io==73)
    power_adc_enable();
  io=io+io%33;
  io=io+counterTimer2 %15;
  if (io==73)
    power_adc_enable();
  io=io+io%33;
  io=io+counterTimer2 %13;
  if (io==73)
    power_adc_enable();
  io=io+io%33;
  io=io+counterTimer2 %13;
  if (io==73)
    power_adc_enable(); //   Wait for Vref to settle
  if  (bit_is_set(ADCSRA, ADIF))
     ADCSRA |= _BV(ADIF);
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  byte low  = ADCL;                    // read the low byte
  byte high = ADCH;                    // read the high byte
  uint16_t result = (high << 8) | low;     // result is the absolute temperature in Kelvin * i think *
  ADCSRA |= (1 << ADIF);       // Clear ADIF
  return result;
}
#endif

t_Time millisT2(){
 #if defined(__sleepT2)
   return counterTimer2;
 #else
   return millis();
 #endif
}

t_Time millisTNow(){
 #if defined(__sleepT2)
   while (bit_is_set(ASSR,TCN2UB));

   return counterTimer2+TCNT2;
 #else
   return millis();
 #endif
}

void syncTimer2(int8_t step)
{
  counterTimer2+=step;
}


#ifndef CRYSTAL32K
t_Time incTimer2(){
  return counterTimer2++;
}
void stopTimer2(t_Time aTime){
}

#endif

void setSleepModeT2()
{
 #if defined(__sleepT2)
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
  long x=0;
// x=x+ internalVcc();
  x=x+micros();
  x=x +(x >>7);
  x=x+ analogRead(0);
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
 #if defined(CRYSTAL32K)
   waitASSR();
   goSleep();
 #else
   // if (F_CPU==16000000L)
       incTimer2();
   if (F_CPU==8000000L)
       incTimer2();
    goSleep();
#endif
}

void delaySleepT2( t_Time t)
{
 #if defined(__sleepT2)
   t_Time startMillis = millisT2();
   #ifdef CRYSTAL32K
     stopTimer2(startMillis+t);
   #endif
   setSleepModeT2();
   do
   {
       delayT2();
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
void  ShutOffADC(void)
{   // https://www.gammon.com.au/power
    //https://www.seanet.com/~karllunt/atmegapowerdown.html
    ACSR = (1<<ACD);                        // disable A/D comparator
    ADCSRA |= ADIF;
    ADCSRA = (0<<ADEN);                     // disable A/D converter
   ADMUX=0;                                //internal Vref off
//    DIDR0 = 0x3f;                           // disable all A/D inputs (ADC0-ADC5)
//    DIDR1 = 0x03;                           // disable AIN0 and AIN1
}

void  SetupADC(void)
{
    //https://www.seanet.com/~karllunt/atmegapowerdown.html
    power_adc_enable(); // ADC converter
    ACSR = 48;                        // enable A/D comparator    ACI+ACO
//    ADCSRA = (1<<ADEN)+7;                     // ADPS2, ADPS1 and ADPS0 prescaler
    ADCSRB&=~ACME;
    ADCSRA = (1<<ADEN)| (1 << ADPS2) |  (1 << ADPS1);                     // ADPS2, ADPS1 and ADPS0 prescaler 64
//    DIDR0 = 0x00;                           // disable all A/D inputs (ADC0-ADC5)
//    DIDR1 = 0x00;                           // disable AIN0 and AIN1
}


void disableADCB()
{
  ShutOffADC();
  ADCSRB|=ACME;
 // ADCSRA = 0;                  //disable ADC
   DIDR0 = 0xff;                           // disable all A/D inputs (ADC0-ADC5)
   DIDR1 = 0x03;                           // disable AIN0 and AIN1

   // turn off brown-out enable in software
  MCUCR = bit (BODS) | bit (BODSE);  // turn on brown-out enable select
  MCUCR = bit (BODS);        // this must be done within 4 clock cycles of above
//ower Reduction Register (PRR)
    power_all_disable();
    power_adc_disable(); // ADC converter
//    power_spi_enable(); // SPI
#if DBGLVL<1
      power_usart0_disable(); // Serial (USART)
#else
   //   power_usart0_enable(); // Serial (USART)

#endif
    power_timer1_disable();
//    power_timer0_enable(); // Timer 0
    power_twi_disable(); // TWI (I2C)
    power_timer2_enable();
//    power_spi_enable();
}



void setupTimer2()
{
//http://electronics.stackexchange.com/questions/26363/how-do-i-create-a-timer-interrupt-with-arduino
  TCCR2B = 0x00;        //Disable Timer2 while we set it up
  TIFR2  = 0x00;        //Timer2 INT Flag Reg: Clear Timer Overflow Flag

   /* First disable the timer overflow interrupt while we're configuring */
  TIMSK2 &= ~(1<<TOIE2);
  TIMSK2 &= ~(1<<OCIE2A);

  /* Configure timer2 in normal mode (pure counting, no PWM etc.) */
  TCCR2A &= ~((1<<WGM21) | (1<<WGM20));
  TCCR2A |= ((1<<WGM21) );
  TCCR2B &= ~(1<<WGM22);
//   DDRD|= (1<<DDD3);
 // TCCR2A |= ((1<<COM2A0) );
 // TCCR2A &= ~(1<<COM2A1);
//  TCCR2A |= ((1<<COM2B0) );
//  TCCR2A &= ~(1<<COM2B1);


  /* Disable Compare Match A interrupt enable (only want overflow) */
  #ifdef CRYSTAL32K
  ASSR |= ((1<<AS2));    /* Select clock source: crystal32k */
  TCCR2B=0x3;
  OCR2A= 1;
  #else
   ASSR &= ~((1<<AS2));         /* Select clock source: internal I/O clock */

  /* Now configure the prescaler to CPU clock divided by 128 */
   TCCR2B |= (1<<CS22)  | (1<<CS20); // Set bits
   TCCR2B &= ~(1<<CS21);             // Clear bit
   //  if (F_CPU==8000000L)
   TCCR2B &= ~(1<<CS20);             // Clear bit

//      TCCR2B |= (1<<CS21)  ; // Set bits
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
  #endif
 //  TIMSK2 |= (1<<TOIE2);
  TIMSK2 |= (1<<OCIE2A);


//  TIFR2  = 0x00;        //Timer2 INT Flag Reg: Clear Timer Overflow Flag
//  TIMSK2 = 0x01;        //Timer2 INT Reg: Timer2 Overflow Interrupt Enable
}


void waitASSR(){
  while ((ASSR & (1<<OCR2AUB)) != 0x00) {  };
}
#endif
#ifdef CRYSTAL32K
byte addTimer2(byte aTime){
  counterTimer2+=(aTime+1);
  if ((counterTimer2) >= counterTimer2Stop)
   return 0;
  t_Time xDiv=counterTimer2Stop-counterTimer2;
  if (xDiv>counterTimer2MaxStep)
      xDiv=counterTimer2MaxStep;
  return xDiv;
//  if ((counterTimer2+220)<counterTimer2Stop)
//     return 210;
//  return counterTimer2Stop-counterTimer2;
}

byte nextTimer2()
{
 if ((counterTimer2+32)<counterTimer2Stop)
    return 32;
 return 1;
}


void stopTimer2(t_Time aTime){
  counterTimer2Stop=aTime;
}


#endif

void setMaxStepTimer(byte aStep){
 counterTimer2MaxStep=aStep;
}

void resetPin()
{
  MCUSR = 0;
  wdt_disable();
  for (byte i=0; i<20; i++) {    //make all pins inputs with pullups enabled
        pinMode(i, INPUT);
        pinMode(i, OUTPUT);
        digitalWrite(i,LOW);
  }
  DDRD  = 0b11111111;    //All pins in PORTD are outputs
  PORTD = 0b00000000;    //All pins in PORTD are low
}

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
  power_timer0_enable();
  wdt_disable();
  do{
  wdt_enable(WDTO_60MS);
  delay(100);
  for(;;){};
//  while (1) {}
  }while(0);
}

/* Delay for the given number of microseconds.  Assumes a 8 or 16 MHz clock. */
void delayMicros(unsigned int us)
{
        // calling avrlib's delay_us() function with low values (e.g. 1 or
        // 2 microseconds) gives delays longer than desired.
        //delay_us(us);
        // for the 16 MHz clock on most Arduino boards

        // for a one-microsecond delay, simply return.  the overhead
        // of the function call yields a delay of approximately 1 1/8 us.
        if (--us == 0)
                return;

        // the following loop takes a quarter of a microsecond (4 cycles)
        // per iteration, so execute it four times for each microsecond of
        // delay requested.
        us <<= 2;

        // account for the time taken in the preceeding commands.
        us -= 2;

        // busy wait
        __asm__ __volatile__ (
                "1: sbiw %0,1" "\n\t" // 2 cycles
                "brne 1b" : "=w" (us) : "0" (us) // 2 cycles
        );
}

#define READ_SIG_BYTE(idx)                       \
({                                               \
    uint8_t val = _BV(SPMEN) | _BV(SIGRD);       \
    uint8_t *sigPtr = (uint8_t *)(uint16_t)idx;  \
    __asm__                                      \
    (                                            \
        "out %2, %0"        "\n\t"               \
        "lpm %0, Z"         "\n\t"               \
        : "=r" (val)                             \
        : "z" (sigPtr),                          \
          "I" (_SFR_IO_ADDR(SPMCSR)),            \
          "0" (val)                              \
    );                                           \
    val;                                         \
})


#define BOOT_signature_byte_get(addr) \
(__extension__({                      \
      uint8_t __result;                         \
      __asm__ __volatile__                      \
      (                                         \
        "sts %1, %2\n\t"                        \
        "lpm %0, Z" "\n\t"                      \
        : "=r" (__result)                       \
        : "i" (_SFR_MEM_ADDR(__SPM_REG)),       \
          "r" ((uint8_t)(__BOOT_SIGROW_READ)),  \
          "z" ((uint16_t)(addr))                \
      );                                        \
      __result;                                 \
}))

unsigned long bootSignature(){
 unsigned long x=0;
//http://www.avrfreaks.net/forum/unique-id-atmega328pb
  for (int i=0 ;i<4;i++){
         x=x <<8;
    //    x=boot_signature_byte_get(i);
  }
  return x;
}


unsigned int sqrt32(unsigned long n)
{
//http://www.stm32duino.com/viewtopic.php?t=56
//http://www.codecodex.com/wiki/Calculate_an_integer_square_root
unsigned int c = 0x8000;
unsigned long g = 0x8000;

for(;;) {

     if(g*g > n) {
          g ^= c;
     }

     c >>= 1;

     if(c == 0) {
          return g;
     }

     g |= c;

     }
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
