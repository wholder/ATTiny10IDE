/*
 * This is a "lite" version of wiring.c  Perhaps one day this will be better termed
 * a "better" version of wiring.
 * 
 * In essence, chip specific code has been ripped out and must be implemented
 * by the variant through macros and/or functions in pins_arduino.h/c
 * 
 * In short,
 *   #define USE_WIRING_LITE 1
 *   #define USE_NEW_MILLIS  1
 *   #define turnOnMillis(prescaleValue)  ( ...whatever is required... )
 *   #define turnOnPWM(timerNumber,value) ( ...whatever is required... )
 * 
 * For initial development testing and comparison some old code is still
 * remaining here so you can set USE_NEW_MILLIS = 0 for the moment
 * to use the old millis code.
 * 
 * millis(), micros(), delay() and delayMicroseconds() have been split off
 * into MillisMicrosDelay.h/c with more accurate implementation and
 * smaller code size (if you accept a given level of error).
 * 
 * James Sleeman <james@gogo.co.nz>, http://sparks.gogo.co.nz/
 * 
 */

#include "Arduino.h"

#if defined(USE_WIRING_LITE) && USE_WIRING_LITE
#include "wiring_private.h"

#if defined(USE_NEW_MILLIS) && USE_NEW_MILLIS
#include "MillisMicrosDelay.h"
#else
#ifndef NO_MILLIS
#include <avr/interrupt.h>
volatile unsigned long ovrf=0;
ISR(TIM0_OVF_vect){
  ovrf++; //Increment counter every 256 clock cycles
}
// This is the original millis code from "core13"
// as you can see, it includes a bunch of non-binary-friendly
// integer divisions, and further more, they are not accurate
// divisions.
//
// Example, take 9.6MHz, this will produce 37500 overflows
//  per second (9600000 / 256 = 37500), 37.5 per millisecond
//  but millis() just goes ahead and rounds that to 37
unsigned long millis(){
  unsigned long x;
  asm("cli"); 
  /*Scale number of timer overflows to milliseconds*/
  #if F_CPU == 128000
  x = ovrf * 2;
    #elif F_CPU == 600000
  x = ovrf / 2;
  #elif F_CPU == 1000000
  x = ovrf / 4;
  #elif F_CPU == 1200000
  x = ovrf / 5;
  #elif F_CPU == 4000000
  x = ovrf / 16;
  #elif F_CPU == 4800000
  x = ovrf / 19;
  #elif F_CPU == 8000000
  x = ovrf / 31;
  #elif F_CPU == 9600000
  x = ovrf / 37;
  #elif F_CPU == 10000000
  x = ovrf / 39;
  #elif F_CPU == 12000000
  x = ovrf / 47;
  #elif F_CPU == 16000000
  x = ovrf / 63;
  #else
  #error This CPU frequency is not defined
  #endif
  asm("sei");
  return x;
}
/*The following improved micros() code was contributed by a sourceforge user "BBC25185" Thanks!*/
unsigned long micros(){
  unsigned long x;
  asm("cli");
  #if F_CPU == 128000
  x = ovrf * 2000;
  #elif F_CPU == 600000
  x = ovrf * 427;
  #elif F_CPU == 1000000
  x = ovrf * 256;
  #elif F_CPU == 1200000
  x = ovrf * 213;
  #elif F_CPU == 4000000
  x = ovrf * 64;
  #elif F_CPU == 4800000
  x = ovrf * 53;
  #elif F_CPU == 8000000
  x = ovrf * 32;
  #elif F_CPU == 9600000
  x = ovrf * 27;
  #elif F_CPU == 10000000
  x = ovrf * 26;
  #elif F_CPU == 12000000
  x = ovrf * 21;
  #elif F_CPU == 16000000
  x = ovrf * 16;
  #else 
  #error This CPU frequency is not defined
  #endif
  asm("sei");
  return x; 
}
#endif

void delay(unsigned long ms){
  while(ms--){
    delayMicroseconds(1000); 
  }
}

/* Delay for the given number of microseconds.  Assumes a 1, 8, 12, 16, 20 or 24 MHz clock. */
void delayMicroseconds(unsigned int us)
{
  // call = 4 cycles + 2 to 4 cycles to init us(2 for constant delay, 4 for variable)
  
  // calling avrlib's delay_us() function with low values (e.g. 1 or
  // 2 microseconds) gives delays longer than desired.
  //delay_us(us);
#if F_CPU >= 24000000L
  // for the 24 MHz clock for the aventurous ones, trying to overclock

  // zero delay fix
  if (!us) return; //  = 3 cycles, (4 when true)

  // the following loop takes a 1/6 of a microsecond (4 cycles)
  // per iteration, so execute it six times for each microsecond of
  // delay requested.
  us *= 6; // x6 us, = 7 cycles

  // account for the time taken in the preceeding commands.
  // we just burned 22 (24) cycles above, remove 5, (5*4=20)
  // us is at least 6 so we can substract 5
  us -= 5; //=2 cycles

#elif F_CPU >= 20000000L
  // for the 20 MHz clock on rare Arduino boards

  // for a one-microsecond delay, simply return.  the overhead
  // of the function call takes 18 (20) cycles, which is 1us
  __asm__ __volatile__ (
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop"); //just waiting 4 cycles
  if (us <= 1) return; //  = 3 cycles, (4 when true)

  // the following loop takes a 1/5 of a microsecond (4 cycles)
  // per iteration, so execute it five times for each microsecond of
  // delay requested.
  us = (us << 2) + us; // x5 us, = 7 cycles

  // account for the time taken in the preceeding commands.
  // we just burned 26 (28) cycles above, remove 7, (7*4=28)
  // us is at least 10 so we can substract 7
  us -= 7; // 2 cycles

#elif F_CPU >= 16000000L
  // for the 16 MHz clock on most Arduino boards

  // for a one-microsecond delay, simply return.  the overhead
  // of the function call takes 14 (16) cycles, which is 1us
  if (us <= 1) return; //  = 3 cycles, (4 when true)

  // the following loop takes 1/4 of a microsecond (4 cycles)
  // per iteration, so execute it four times for each microsecond of
  // delay requested.
  us <<= 2; // x4 us, = 4 cycles

  // account for the time taken in the preceeding commands.
  // we just burned 19 (21) cycles above, remove 5, (5*4=20)
  // us is at least 8 so we can substract 5
  us -= 5; // = 2 cycles, 

#elif F_CPU >= 12000000L
  // for the 12 MHz clock if somebody is working with USB

  // for a 1 microsecond delay, simply return.  the overhead
  // of the function call takes 14 (16) cycles, which is 1.5us
  if (us <= 1) return; //  = 3 cycles, (4 when true)

  // the following loop takes 1/3 of a microsecond (4 cycles)
  // per iteration, so execute it three times for each microsecond of
  // delay requested.
  us = (us << 1) + us; // x3 us, = 5 cycles

  // account for the time taken in the preceeding commands.
  // we just burned 20 (22) cycles above, remove 5, (5*4=20)
  // us is at least 6 so we can substract 5
  us -= 5; //2 cycles

#elif F_CPU >= 8000000L
  // for the 8 MHz internal clock

  // for a 1 and 2 microsecond delay, simply return.  the overhead
  // of the function call takes 14 (16) cycles, which is 2us
  if (us <= 2) return; //  = 3 cycles, (4 when true)

  // the following loop takes 1/2 of a microsecond (4 cycles)
  // per iteration, so execute it twice for each microsecond of
  // delay requested.
  us <<= 1; //x2 us, = 2 cycles

  // account for the time taken in the preceeding commands.
  // we just burned 17 (19) cycles above, remove 4, (4*4=16)
  // us is at least 6 so we can substract 4
  us -= 4; // = 2 cycles
#elif F_CPU >= 6000000L
  // for that unusual 6mhz clock... 

  // for a 1 and 2 microsecond delay, simply return.  the overhead
  // of the function call takes 14 (16) cycles, which is 2us
  if (us <= 2) return; //  = 3 cycles, (4 when true)

  // the following loop takes 2/3rd microsecond (4 cycles)
  // per iteration, so we want to add it to half of itself
  us +=us>>1;
  us -= 2; // = 2 cycles

#elif F_CPU >= 4000000L
  // for that unusual 4mhz clock... 

  // for a 1 and 2 microsecond delay, simply return.  the overhead
  // of the function call takes 14 (16) cycles, which is 2us
  if (us <= 2) return; //  = 3 cycles, (4 when true)

  // the following loop takes 1 microsecond (4 cycles)
  // per iteration, so nothing to do here! \o/

  us -= 2; // = 2 cycles


#else
  // for the 1 MHz internal clock (default settings for common AVR microcontrollers)

  // the overhead of the function calls is 14 (16) cycles
  if (us <= 16) return; //= 3 cycles, (4 when true)
  if (us <= 25) return; //= 3 cycles, (4 when true), (must be at least 25 if we want to substract 22)

  // compensate for the time taken by the preceeding and next commands (about 22 cycles)
  us -= 22; // = 2 cycles
  // the following loop takes 4 microseconds (4 cycles)
  // per iteration, so execute it us/4 times
  // us is at least 4, divided by 4 gives us 1 (no zero delay bug)
  us >>= 2; // us div 4, = 4 cycles
  

#endif

  // busy wait
  __asm__ __volatile__ (
    "1: sbiw %0,1" "\n\t" // 2 cycles
    "brne 1b" : "=w" (us) : "0" (us) // 2 cycles
  );
  // return = 4 cycles
}

#endif

void init(){  
#ifdef  setCPUFrequency
  setCPUFrequency(F_CPU);
#endif 
#ifndef NO_MILLIS
  #if (!defined(turnOnMillis) || !(defined(USE_NEW_MILLIS) && USE_NEW_MILLIS))
  // Start timer0 running, setup the millis() interrupt to run
  TCCR0B |= _BV(CS00);
  TCCR0A |= _BV(WGM00)|_BV(WGM01);
  TIMSK0 |= 2;
  TCNT0=0;   
  #else
    turnOnMillis(MILLIS_TIMER_PRESCALE);
  #endif
  sei();
#else
  #ifndef turnOnPWM
  // Enabled fast PWM on the timer (not connected to the pin, that happens in 
  // analogWrite()
  //
  // if there is a turnOnPWM(t,v) macro, then we don't need to do this, because
  // turnOnPWM() should ensure it has been done when it does that
  TCCR0B |= _BV(CS00);
  TCCR0A |= _BV(WGM00)|_BV(WGM01);
  #endif
#endif
  
  
#if defined(INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER) && INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER  
  #if defined(turnOnADC)
    // If a variant has to do things differently, they can define turnOnADC to do so.
    turnOnADC(F_CPU);
  #elif !defined(turnOnADC) && defined(ADCSRA)
    // Default implementation, so far I have not come across a tiny that works differently in this regard
    // so probably the default is just fine.        
    //
    // [ Datasheet: By default, the successive approximation circuitry requires an input clock frequency 
    // between 50 kHz and 200 kHz to get maximum resolution. ]
    
    // It seems that there is no practical advantage in accuracy to run with a clock lower than 200kHZ
    // so the numbers here are the maximum frequency for which the given prescaler (2^[PRESCALER]) will
    // result in 200kHZ or less
    #if   F_CPU <= 400000UL   
      #define ADC_ARDUINO_PRESCALER 1 
    #elif F_CPU <= 800000UL 
      #define ADC_ARDUINO_PRESCALER 2
    #elif F_CPU <= 1600000UL 
      #define ADC_ARDUINO_PRESCALER 3
    #elif F_CPU <= 3200000UL 
      #define ADC_ARDUINO_PRESCALER 4
    #elif F_CPU <= 6400000UL 
      #define ADC_ARDUINO_PRESCALER 5
    #elif F_CPU <= 12800000UL 
      #define ADC_ARDUINO_PRESCALER 6
    #else
      #define ADC_ARDUINO_PRESCALER 7
    #endif
    
    // Since this is init(), ADCSRA is already going to be zero, we can just
    // set our bits indiscriminantly
    ADCSRA = (ADC_ARDUINO_PRESCALER << ADPS0) | _BV(ADEN);
  #endif
#endif  
}
#endif