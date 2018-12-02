#include "Arduino.h"
#if defined(USE_NEW_MILLIS) && USE_NEW_MILLIS
#include "MillisMicrosDelay.h"

// millis() and micros() are disabled entirely with NO_MILLIS
#ifndef NO_MILLIS
#include <avr/interrupt.h>

/*
 * The overflow interrupt and incrementing of the counter is done
 * by pins_arduino.c of the variant in usage, this is an example 
 * of how you would do that in pins_arduino.c
 
  extern volatile MillisMicrosTime_t ovrf;
  
  ISR(TIM0_OVF_vect)
  {
    ovrf++; //Increment counter every 256 clock cycles  
  }
*/

   
  // A quick refresher on the calculations for millis() and micros()
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // The millis() and micros() calculations look complex, but it's really not that 
  // tricky.
  //
  // Example of F_CPU = 16kHz, MILLIS_TIMER_PRESCALE = 1
  //  16000 / 1    = 16000 ticks per second
  //        / 256  = 62.5 overflows per second
  //        / 1000 = 0.0625 overflows per mS
  //   
  //    1 / 0.0625 = 16 mS per overflow, so millis() = overf * 16 
  //    
  // Example F_CPU = 32.768kHz, MILLIS_TIMER_PRESCALE = 1
  //  32768 / 1    = 32768 ticks per second
  //        / 256  = 128 overflows per second
  //        / 1000 = 0.128 overflows per mS
  //   
  //    1 / 0.128 = 7.8125 mS per overflow, so millis() = overf * 7.8125
  //
  //    To avoid floating point arithmetic various integer math only 
  //    approximations are presented depending on the acceptable level of error.
  // 
  // micros() is of course pretty similar.

volatile MillisMicrosTime_t ovrf=0;

/* 
 * The ISR is defined in pins_arduno.c for the variant as it could, well, vary
 * depending on the particular chip, all we care about is that ovrf gets updated
 * every frequency/prescale/256 ticks.  Exactly how each variant chooses to do that
 * is up to it.
 *
 * A typical ISR would look like...

  extern volatile MillisMicrosTime_t ovrf;
  ISR(TIM0_OVF_vect)
  {
    ovrf++; //Increment counter every 256 clock cycles  
  }
  */

MillisMicrosTime_t millis()
{
  MillisMicrosTime_t x;
  uint8_t oldSREG = SREG;
  cli();
      
  // To come up with these calculations I wrote a Google Spreadsheet which includes 
  // an automation script to generated the C code required.  Should you need to
  // add different frequencies in future, feel free to copy the sheet and use it.
  //
  //   https://goo.gl/sgANEr
  //
  // James Sleeman, james@gogo.co.nz, http://sparks.gogo.co.nz/
 
#if (F_CPU / MILLIS_TIMER_PRESCALE) >= 24000000UL
  // 24 MHz
    //  Error: 0.0146% (0.0001455417 Decimal)
    // Jitter: 0.0372% (0.0003718387 Decimal)
    x = (ovrf * 0)  + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 8192) + (ovrf / 32768) + (ovrf / 65536);

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 20000000UL
  // 20 MHz
    //  Error: 0.1080% (0.0010804776999999998 Decimal)
    // Jitter: 0.0367% (0.0003666667 Decimal)
    x = (ovrf * 0)  + (ovrf / 128) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 16384) + (ovrf / 32768);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 19200000UL
  // 19.2 MHz
    //  Error: 0.0992% (0.0009916172 Decimal)
    // Jitter: 0.0329% (0.0003291789 Decimal)
    x = (ovrf * 0)  + (ovrf / 128) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 16000000UL
  // 16 MHz
    //  Error: 0.0580% (0.0005804791 Decimal)
    // Jitter: 0.0177% (0.00017671090000000002 Decimal)
    x = (ovrf * 0)  + (ovrf / 64) + (ovrf / 4096) + (ovrf / 8192);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 12000000UL
  // 12 MHz
    //  Error: 0.0146% (0.0001455417 Decimal)
    // Jitter: 0.0372% (0.0003718387 Decimal)
    x = (ovrf * 0)  + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 32768);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 9600000UL
  // 9.6 MHz
    //  Error: 0.0429% (0.0004293574 Decimal)
    // Jitter: 0.0377% (0.0003769841 Decimal)
    x = (ovrf * 0)  + (ovrf / 64) + (ovrf / 128) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 32768) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 8000000UL
  // 8 MHz
    //  Error: 0.0114% (0.0001138951 Decimal)
    // Jitter: 0.0224% (0.0002238095 Decimal)
    x = (ovrf * 0)  + (ovrf / 32) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 6400000UL
  // 6.4 MHz
    //  Error: 0.0237% (0.0002371862 Decimal)
    // Jitter: 0.0301% (0.0003012704 Decimal)
    x = (ovrf * 0)  + (ovrf / 32) + (ovrf / 128) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 8192) + (ovrf / 16384) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 4800000UL
  // 4.8 MHz
    //  Error: 0.0153% (0.00015272219999999998 Decimal)
    // Jitter: 0.0413% (0.0004126984 Decimal)
    x = (ovrf * 0)  + (ovrf / 32) + (ovrf / 64) + (ovrf / 256) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 16384) + (ovrf / 32768) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 4000000UL
  // 4 MHz
    //  Error: 0.0114% (0.0001138951 Decimal)
    // Jitter: 0.0224% (0.0002238095 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 1024) + (ovrf / 2048) + (ovrf / 32768);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 3000000UL
  // 3 MHz
    //  Error: 0.0145% (0.0001452576 Decimal)
    // Jitter: 0.0369% (0.0003692755 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 8192);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2500000UL
  // 2.5 MHz
    //  Error: 0.0205% (0.0002048921 Decimal)
    // Jitter: 0.0556% (0.0005560046 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 32) + (ovrf / 128) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 32768);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2400000UL
  // 2.4 MHz
    //  Error: 0.0153% (0.00015272219999999998 Decimal)
    // Jitter: 0.0413% (0.0004126984 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 32) + (ovrf / 128) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 8192) + (ovrf / 16384) + (ovrf / 32768);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2000000UL
  // 2 MHz
    //  Error: 0.0113% (0.0001126104 Decimal)
    // Jitter: 0.0224% (0.0002238139 Decimal)
    x = (ovrf * 0)  + (ovrf / 8) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 16384);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1500000UL
  // 1.5 MHz
    //  Error: 0.0141% (0.0001406642 Decimal)
    // Jitter: 0.0375% (0.0003745527 Decimal)
    x = (ovrf * 0)  + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 256) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 4096);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1200000UL
  // 1.2 MHz
    //  Error: 0.0096% (0.0000955207 Decimal)
    // Jitter: 0.0480% (0.0004795937 Decimal)
    x = (ovrf * 0)  + (ovrf / 8) + (ovrf / 16) + (ovrf / 64) + (ovrf / 128) + (ovrf / 512) + (ovrf / 4096) + (ovrf / 8192) + (ovrf / 16384) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1000000UL
  // 1 MHz
    //  Error: 0.0063% (0.0000628935 Decimal)
    // Jitter: 0.0278% (0.0002777817 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 256) + (ovrf / 512) + (ovrf / 8192) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 800000UL
  // 800 kHz
    //  Error: 0.0115% (0.0001145359 Decimal)
    // Jitter: 0.0387% (0.0003865699 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 256) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 32768) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 600000UL
  // 600 kHz
    //  Error: 0.0091% (0.0000913535 Decimal)
    // Jitter: 0.0480% (0.0004795937 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 32) + (ovrf / 64) + (ovrf / 256) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 8192) + (ovrf / 32768);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 500000UL
  // 500 kHz
    //  Error: 0.0066% (0.0000656635 Decimal)
    // Jitter: 0.0278% (0.0002779503 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 128) + (ovrf / 256) + (ovrf / 4096) + (ovrf / 32768);

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 375000UL
  // 375 kHz
    //  Error: 0.0092% (0.0000915869 Decimal)
    // Jitter: 0.0435% (0.0004345473 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 64) + (ovrf / 128) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 32768) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 312500UL
  // 312.5 kHz
    //  Error: 0.0098% (0.000098003 Decimal)
    // Jitter: 0.0678% (0.0006777827 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16) + (ovrf / 256) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 32768) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 300000UL
  // 300 kHz
    //  Error: 0.0082% (0.00008233490000000001 Decimal)
    // Jitter: 0.0485% (0.0004849638 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16) + (ovrf / 32) + (ovrf / 128) + (ovrf / 1024) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 16384);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 250000UL
  // 250 kHz
    //  Error: 0.0055% (0.0000546916 Decimal)
    // Jitter: 0.0255% (0.0002551896 Decimal)
    x = (ovrf * 1)  + (ovrf / 64) + (ovrf / 128) + (ovrf / 2048) + (ovrf / 16384);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 187500UL
  // 187.5 kHz
    //  Error: 0.0085% (0.0000847758 Decimal)
    // Jitter: 0.0385% (0.0003851959 Decimal)
    x = (ovrf * 1)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 32) + (ovrf / 64) + (ovrf / 256) + (ovrf / 512) + (ovrf / 16384) + (ovrf / 32768);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 150000UL
  // 150 kHz
    //  Error: 0.0082% (0.00008233490000000001 Decimal)
    // Jitter: 0.0485% (0.0004849638 Decimal)
    x = (ovrf * 1)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 16) + (ovrf / 64) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 2048) + (ovrf / 8192);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 128000UL
  // 128 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 2) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 125000UL
  // 125 kHz
    //  Error: 0.0055% (0.0000545291 Decimal)
    // Jitter: 0.0258% (0.00025814839999999996 Decimal)
    x = (ovrf * 2)  + (ovrf / 32) + (ovrf / 64) + (ovrf / 1024) + (ovrf / 8192) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 100000UL
  // 100 kHz
    //  Error: 0.0087% (0.00008692650000000001 Decimal)
    // Jitter: 0.0373% (0.0003726614 Decimal)
    x = (ovrf * 2)  + (ovrf / 2) + (ovrf / 32) + (ovrf / 64) + (ovrf / 128) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 8192) + (ovrf / 16384);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 75000UL
  // 75 kHz
    //  Error: 0.0068% (0.00006818429999999999 Decimal)
    // Jitter: 0.0407% (0.00040653280000000003 Decimal)
    x = (ovrf * 3)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 32) + (ovrf / 256) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 4096);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 62500UL
  // 62.5 kHz
    //  Error: 0.0053% (0.000053184600000000006 Decimal)
    // Jitter: 0.0258% (0.0002582281 Decimal)
    x = (ovrf * 4)  + (ovrf / 16) + (ovrf / 32) + (ovrf / 512) + (ovrf / 4096) + (ovrf / 32768) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 37500UL
  // 37.5 kHz
    //  Error: 0.0064% (0.0000640909 Decimal)
    // Jitter: 0.0364% (0.0003637025 Decimal)
    x = (ovrf * 6)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16) + (ovrf / 128) + (ovrf / 256) + (ovrf / 512) + (ovrf / 2048);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 32768UL
  // 32.768 kHz
    //  Error: 0.0000% (0 Decimal)
    // Jitter: 0.0000% (0 Decimal)
    x = (ovrf * 7)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 31250UL
  // 31.25 kHz
    //  Error: 0.0052% (0.0000522177 Decimal)
    // Jitter: 0.0258% (0.0002581808 Decimal)
    x = (ovrf * 8)  + (ovrf / 8) + (ovrf / 16) + (ovrf / 256) + (ovrf / 2048) + (ovrf / 16384) + (ovrf / 32768);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 18750UL
  // 18.75 kHz
    //  Error: 0.0050% (0.000049767899999999995 Decimal)
    // Jitter: 0.0300% (0.00030018009999999997 Decimal)
    x = (ovrf * 13)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 64) + (ovrf / 128) + (ovrf / 256) + (ovrf / 1024);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 16000UL
  // 16 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 16) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 15625UL
  // 15.625 kHz
    //  Error: 0.0046% (0.0000461119 Decimal)
    // Jitter: 0.0242% (0.0002421586 Decimal)
    x = (ovrf * 16)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 128) + (ovrf / 1024) + (ovrf / 8192) + (ovrf / 16384) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 12500UL
  // 12.5 kHz
    //  Error: 0.0078% (0.0000782744 Decimal)
    // Jitter: 0.0369% (0.00036935969999999995 Decimal)
    x = (ovrf * 20)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 16) + (ovrf / 32) + (ovrf / 128) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 2048) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 9375UL
  // 9.375 kHz
    //  Error: 0.0050% (0.000049767899999999995 Decimal)
    // Jitter: 0.0300% (0.00030018009999999997 Decimal)
    x = (ovrf * 27)  + (ovrf / 4) + (ovrf / 32) + (ovrf / 64) + (ovrf / 128) + (ovrf / 512) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 4096UL
  // 4.096 kHz
    //  Error: 0.0000% (0 Decimal)
    // Jitter: 0.0000% (0 Decimal)
    x = (ovrf * 62)  + (ovrf / 2);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2000UL
  // 2 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 128) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1953UL
  // 1.953 kHz
    //  Error: 0.0031% (0.0000310192 Decimal)
    // Jitter: 0.0141% (0.0001412729 Decimal)
    x = (ovrf * 131)  + (ovrf / 16) + (ovrf / 64) + (ovrf / 512) + (ovrf / 4096) + (ovrf / 16384);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 512UL
  // 0.512 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 500) ;
#endif
  SREG = oldSREG;
  return x;
}

MillisMicrosTime_t micros()
{
  MillisMicrosTime_t x;
  uint8_t  oldSREG = SREG;
  cli();

  // To come up with these calculations I wrote a Google Spreadsheet which includes
  // an automation script to generated the C code required.  Should you need to
  // add different frequencies in future, feel free to copy the sheet and use it.
  //
  //   https://goo.gl/sgANEr
  //
  // James Sleeman, james@gogo.co.nz, http://sparks.gogo.co.nz/

#if (F_CPU / MILLIS_TIMER_PRESCALE) >= 24000000UL
  // 24 MHz
    //  Error: 0.0074% (0.0000739307 Decimal)
    // Jitter: 0.0315% (0.00031501690000000003 Decimal)
    x = (ovrf * 10)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 32768);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 20000000UL
  // 20 MHz
    //  Error: 0.0068% (0.000067535 Decimal)
    // Jitter: 0.0318% (0.0003179226 Decimal)
    x = (ovrf * 12)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 32) + (ovrf / 64) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 8192) + (ovrf / 16384);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 19200000UL
  // 19.2 MHz
    //  Error: 0.0065% (0.0000651144 Decimal)
    // Jitter: 0.0384% (0.000384375 Decimal)
    x = (ovrf * 13)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 16000000UL
  // 16 MHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 16) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 12000000UL
  // 12 MHz
    //  Error: 0.0060% (0.0000600665 Decimal)
    // Jitter: 0.0258% (0.00025756650000000004 Decimal)
    x = (ovrf * 21)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 9600000UL
  // 9.6 MHz
    //  Error: 0.0068% (0.0000677557 Decimal)
    // Jitter: 0.0384% (0.0003841938 Decimal)
    x = (ovrf * 26)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 32768);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 8000000UL
  // 8 MHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 32) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 6400000UL
  // 6.4 MHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 40) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 4800000UL
  // 4.8 MHz
    //  Error: 0.0054% (0.0000540151 Decimal)
    // Jitter: 0.0292% (0.0002917454 Decimal)
    x = (ovrf * 53)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 4000000UL
  // 4 MHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 64) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 3000000UL
  // 3 MHz
    //  Error: 0.0046% (0.0000462079 Decimal)
    // Jitter: 0.0191% (0.0001911724 Decimal)
    x = (ovrf * 85)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2500000UL
  // 2.5 MHz
    //  Error: 0.0059% (0.0000587247 Decimal)
    // Jitter: 0.0293% (0.0002925152 Decimal)
    x = (ovrf * 102)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 64) + (ovrf / 128) + (ovrf / 1024) + (ovrf / 2048) + (ovrf / 16384) + (ovrf / 32768);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2400000UL
  // 2.4 MHz
    //  Error: 0.0056% (0.0000559561 Decimal)
    // Jitter: 0.0288% (0.00028813900000000003 Decimal)
    x = (ovrf * 106)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 32768);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2000000UL
  // 2 MHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 128) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1500000UL
  // 1.5 MHz
    //  Error: 0.0046% (0.0000463401 Decimal)
    // Jitter: 0.0193% (0.0001928854 Decimal)
    x = (ovrf * 170)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 32768);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1200000UL
  // 1.2 MHz
    //  Error: 0.0042% (0.0000424244 Decimal)
    // Jitter: 0.0204% (0.0002038113 Decimal)
    x = (ovrf * 213)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1000000UL
  // 1 MHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 256) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 800000UL
  // 800 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 320) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 600000UL
  // 600 kHz
    //  Error: 0.0030% (0.0000295508 Decimal)
    // Jitter: 0.0204% (0.0002038113 Decimal)
    x = (ovrf * 426)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 32768);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 500000UL
  // 500 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 512) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 375000UL
  // 375 kHz
    //  Error: 0.0026% (0.0000256368 Decimal)
    // Jitter: 0.0133% (0.0001331647 Decimal)
    x = (ovrf * 682)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 32768);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 312500UL
  // 312.5 kHz
    //  Error: 0.0024% (0.0000237285 Decimal)
    // Jitter: 0.0174% (0.00017438310000000002 Decimal)
    x = (ovrf * 819)  + (ovrf / 8) + (ovrf / 16) + (ovrf / 128) + (ovrf / 256) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 32768) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 300000UL
  // 300 kHz
    //  Error: 0.0028% (0.0000281963 Decimal)
    // Jitter: 0.0209% (0.00020863839999999998 Decimal)
    x = (ovrf * 853)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 250000UL
  // 250 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 1024) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 187500UL
  // 187.5 kHz
    //  Error: 0.0026% (0.0000256368 Decimal)
    // Jitter: 0.0133% (0.0001331647 Decimal)
    x = (ovrf * 1365)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 150000UL
  // 150 kHz
    //  Error: 0.0023% (0.000023453 Decimal)
    // Jitter: 0.0167% (0.00016740600000000002 Decimal)
    x = (ovrf * 1706)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 32768);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 128000UL
  // 128 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 2000) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 125000UL
  // 125 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 2048) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 100000UL
  // 100 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 2560) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 75000UL
  // 75 kHz
    //  Error: 0.0013% (0.00001288 Decimal)
    // Jitter: 0.0146% (0.00014647720000000001 Decimal)
    x = (ovrf * 3413)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 62500UL
  // 62.5 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 4096) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 37500UL
  // 37.5 kHz
    //  Error: 0.0008% (0.0000076401 Decimal)
    // Jitter: 0.0146% (0.00014647720000000001 Decimal)
    x = (ovrf * 6826)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 32768);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 32768UL
  // 32.768 kHz
    //  Error: 0.0003% (0.0000030611000000000002 Decimal)
    // Jitter: 0.0128% (0.0001279918 Decimal)
    x = (ovrf * 7812)  + (ovrf / 2);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 31250UL
  // 31.25 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 8192) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 18750UL
  // 18.75 kHz
  //  Error: 0.0003% (0.0000032111 Decimal)
  // Jitter: 0.0037% (0.000036620600000000006 Decimal)

  x = (ovrf * 13653)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 16000UL
  // 16 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 16000) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 15625UL
  // 15.625 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 16384) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 12500UL
  // 12.5 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 20480) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 9375UL
  // 9.375 kHz
  //  Error: 0.0002% (0.00000191 Decimal)
  // Jitter: 0.0037% (0.000036620600000000006 Decimal)

  x = (ovrf * 27306)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 32768);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 4096UL
  // 4.096 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 62500) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2000UL
  // 2 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 128000) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1953UL
  // 1.953 kHz
  //  Error: 0.0000% (3.3800000000000004e-7 Decimal)
  // Jitter: 0.0004% (0.0000038144 Decimal)

  x = (ovrf * 131080)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 128) + (ovrf / 256) + (ovrf / 512) + (ovrf / 4096) + (ovrf / 8192) + (ovrf / 16384) + (ovrf / 32768) + (ovrf / 65536);


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 512UL
  // 0.512 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 500000) ;
#endif
  
  SREG = oldSREG;
  return x;
}
#endif

void delay(DelayTime_t ms)
{
  while(ms--){
    delayMicroseconds(1000); 
  }
#if 0
#ifdef NO_MILLIS
  while(ms--){
    delayMicroseconds(1000); 
  }
#else
  // If millis() is present, that means we have an interrupt running which will
  // screw up delayMicroseconds() unless we disable interrupts (this problem
  // exists in the Arduino core too, not much you can do about it without 
  // spending code on checking for interrupts and adjusting and even then
  // we have no way to know how big/often any other interrupts that might be
  // configured are).
  //
  // Anyway, we can't use delayMicroseconds() for this if millis() is available
  // or the delay will be quite inaccurate.
    
  MillisMicrosTime_t current = millis();
  while(millis() - current < ms);
  return;
#endif
#endif
}

// For clock-counting/sim/debug you might want to add
//   __attribute__ ((noinline)) 
// so it's easier to see what's going on in the decompilation
void delayMicrosecondsWithoutMillisInterruptAdjustment(DelayMicrosecondsTime_t us)
{
  // This is pretty much the standard Arduino delayMicroseconds() however I have 
  // recalculated all the numbers for improved consistency and accuracy, and 
  // have done away with any attempt to handle delays shorter than the minimum 
  // overhead + 1 busy-loop.  That is, if you try and delayMicroseconds() for 
  // shorter than we can do, it will just return immediately.  
  //
  // As of writing, the minimum us delay to actually get a busy-loop are:
  //
  //  24MHz: 1
  //  20MHz: 4
  //  16MHz: 2
  //  12MHz: 6
  //  9.6MHz: 9
  //  8MHz: 3
  //  6.4MHz: 12
  //  4.8MHz: 16
  //  4.0MHz: 4
  //  2.4MHz: 33
  //  2.0MHz: 9
  //  1.2MHz: 64
  //  1.0MHz: 21
  //  800kHz: 91
  //  600kHz: 132
  //  128kHz: 323
  //  125kHz: 329
  //  32.768kHz: 643
  //  16kHz: 1126
  //
  // anything shorter than those will return in close to ~0us, anything longer
  // than those should be pretty close to the requested us.
  //
  // To come up with these calculations I wrote a Google Spreadsheet which includes 
  // an automation script to generated the C code required.  Should you need to
  // add different frequencies in future, feel free to copy the sheet and use it.
  //
  //   https://goo.gl/sgANEr
  //
  // James Sleeman, james@gogo.co.nz, http://sparks.gogo.co.nz/
  
#if F_CPU >= 24000000UL
  // 24MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 0) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 6.0000 * us
  //        Adjustment Takes: 4 Clocks (Approx)
  us = ( us * 6) ;

  // Compensate for the combined overhead time of 0.5833us
  // by subtracting 3 Loop Cycles
  // us at this point is at least 6 Loops - 1us
  //       Subtraction Takes: 2 Clocks
  us -= 3;

#elif F_CPU >= 19200000UL
  // 19.2MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 4) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 4.8000 * us
  //        Adjustment Takes: 75 Clocks (Approx)
  us = ( us * 4) + (us / 2) + (us / 4) + (us / 32) + (us / 64) ;

  // Compensate for the combined overhead time of 4.4271us
  // by subtracting 21 Loop Cycles
  // us at this point is at least 23 Loops - 5us
  //       Subtraction Takes: 2 Clocks
  us -= 21;
  
#elif F_CPU >= 20000000UL
  // 20MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 3) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 5.0000 * us
  //        Adjustment Takes: 57 Clocks (Approx)
  us = ( us * 5) ;

  // Compensate for the combined overhead time of 3.3500us
  // by subtracting 16 Loop Cycles
  // us at this point is at least 20 Loops - 4us
  //       Subtraction Takes: 2 Clocks
  us -= 16;


#elif F_CPU >= 16000000UL
  // 16MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 1) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 4.0000 * us
  //        Adjustment Takes: 8 Clocks (Approx)
  us = ( us * 4) ;

  // Compensate for the combined overhead time of 1.1250us
  // by subtracting 4 Loop Cycles
  // us at this point is at least 8 Loops - 2us
  //       Subtraction Takes: 2 Clocks
  us -= 4;


#elif F_CPU >= 12000000UL
  // 12MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 5) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 3.0000 * us
  //        Adjustment Takes: 50 Clocks (Approx)
  us = ( us * 3) ;

  // Compensate for the combined overhead time of 5.0000us
  // by subtracting 15 Loop Cycles
  // us at this point is at least 18 Loops - 6us
  //       Subtraction Takes: 2 Clocks
  us -= 15;


#elif F_CPU >= 9600000UL
  // 9.6MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 8) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 2.4000 * us
  //        Adjustment Takes: 68 Clocks (Approx)
  us = ( us * 2) + (us / 4) + (us / 8) + (us / 64) + (us / 128) ;

  // Compensate for the combined overhead time of 8.1250us
  // by subtracting 19 Loop Cycles
  // us at this point is at least 21 Loops - 9us
  //       Subtraction Takes: 2 Clocks
  us -= 19;


#elif F_CPU >= 8000000UL
  // 8MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 2) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 2.0000 * us
  //        Adjustment Takes: 6 Clocks (Approx)
  us = ( us * 2) ;

  // Compensate for the combined overhead time of 2.0000us
  // by subtracting 4 Loop Cycles
  // us at this point is at least 6 Loops - 3us
  //       Subtraction Takes: 2 Clocks
  us -= 4;


#elif F_CPU >= 6400000UL
  // 6.4MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 11) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 1.6000 * us
  //        Adjustment Takes: 59 Clocks (Approx)
  us = ( us * 1) + (us / 2) + (us / 16) + (us / 32) + (us / 256);

  // Compensate for the combined overhead time of 10.7813us
  // by subtracting 17 Loop Cycles
  // us at this point is at least 18 Loops - 12us
  //       Subtraction Takes: 2 Clocks
  us -= 17;


#elif F_CPU >= 4800000UL
  // 4.8MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 15) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 1.2000 * us
  //        Adjustment Takes: 54 Clocks (Approx)
  us = ( us * 1) + (us / 8) + (us / 16) + (us / 128) + (us / 256);

  // Compensate for the combined overhead time of 13.3333us
  // by subtracting 16 Loop Cycles
  // us at this point is at least 19 Loops - 16us
  //       Subtraction Takes: 2 Clocks
  us -= 16;


#elif F_CPU >= 4000000UL
  // 4MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 3) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 1.0000 * us
  //        Adjustment Takes: 0 Clocks (Approx)
  us = ( us * 1) ;

  // Compensate for the combined overhead time of 2.5000us
  // by subtracting 2 Loop Cycles
  // us at this point is at least 4 Loops - 4us
  //       Subtraction Takes: 2 Clocks
  us -= 2;


#elif F_CPU >= 2400000UL
  // 2.4MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 32) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.6000 * us
  //        Adjustment Takes: 55 Clocks (Approx)
  us = ( us * 0) + (us / 2) + (us / 16) + (us / 32) + (us / 256);

  // Compensate for the combined overhead time of 27.0833us
  // by subtracting 16 Loop Cycles
  // us at this point is at least 19 Loops - 33us
  //       Subtraction Takes: 2 Clocks
  us -= 16;


#elif F_CPU >= 2000000UL
  // 2MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 8) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.5000 * us
  //        Adjustment Takes: 2 Clocks (Approx)
  us = ( us * 0) + (us / 2) ;

  // Compensate for the combined overhead time of 6.0000us
  // by subtracting 3 Loop Cycles
  // us at this point is at least 4 Loops - 9us
  //       Subtraction Takes: 2 Clocks
  us -= 3;


#elif F_CPU >= 1200000UL
  // 1.2MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 63) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.3000 * us
  //        Adjustment Takes: 65 Clocks (Approx)
  us = ( us * 0) + (us / 4) + (us / 32) + (us / 64) ;

  // Compensate for the combined overhead time of 62.5000us
  // by subtracting 18 Loop Cycles
  // us at this point is at least 19 Loops - 64us
  //       Subtraction Takes: 2 Clocks
  us -= 18;


#elif F_CPU >= 1000000UL
  // 1MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 20) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.2500 * us
  //        Adjustment Takes: 6 Clocks (Approx)
  us = ( us * 0) + (us / 4) ;

  // Compensate for the combined overhead time of 16.0000us
  // by subtracting 4 Loop Cycles
  // us at this point is at least 5 Loops - 21us
  //       Subtraction Takes: 2 Clocks
  us -= 4;


#elif F_CPU >= 800000UL
  // 800kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 90) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.2000 * us
  //        Adjustment Takes: 50 Clocks (Approx)
  us = ( us * 0) + (us / 8) + (us / 16) + (us / 128) + (us / 256);

  // Compensate for the combined overhead time of 75.0000us
  // by subtracting 15 Loop Cycles
  // us at this point is at least 16 Loops - 91us
  //       Subtraction Takes: 2 Clocks
  us -= 15;


#elif F_CPU >= 600000UL
  // 600kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 131) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.1500 * us
  //        Adjustment Takes: 56 Clocks (Approx)
  us = ( us * 0) + (us / 8) + (us / 64) + (us / 128) ;

  // Compensate for the combined overhead time of 110.0000us
  // by subtracting 16 Loop Cycles
  // us at this point is at least 19 Loops - 132us
  //       Subtraction Takes: 2 Clocks
  us -= 16;


#elif F_CPU >= 128000UL
  // 128kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 322) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.0320 * us
  //        Adjustment Takes: 27 Clocks (Approx)
  us = ( us * 0) + (us / 32) ;

  // Compensate for the combined overhead time of 289.0625us
  // by subtracting 9 Loop Cycles
  // us at this point is at least 10 Loops - 323us
  //       Subtraction Takes: 2 Clocks
  us -= 9;


#elif F_CPU >= 125000UL
  // 125kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 328) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.0313 * us
  //        Adjustment Takes: 27 Clocks (Approx)
  us = ( us * 0) + (us / 32) ;

  // Compensate for the combined overhead time of 296.0000us
  // by subtracting 9 Loop Cycles
  // us at this point is at least 10 Loops - 329us
  //       Subtraction Takes: 2 Clocks
  us -= 9;


#elif F_CPU >= 32768UL
  // 32.768kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 642) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.0082 * us
  //        Adjustment Takes: 7 Clocks (Approx)
  us = ( us * 0) + (us / 128) ;

  // Compensate for the combined overhead time of 518.7988us
  // by subtracting 4 Loop Cycles
  // us at this point is at least 5 Loops - 643us
  //       Subtraction Takes: 2 Clocks
  us -= 4;


#elif F_CPU >= 16000UL
  // 16kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 1125) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.0040 * us
  //        Adjustment Takes: 4 Clocks (Approx)
  us = ( us * 0) + (us / 256);

  // Compensate for the combined overhead time of 875.0000us
  // by subtracting 3 Loop Cycles
  // us at this point is at least 4 Loops - 1126us
  //       Subtraction Takes: 2 Clocks
  us -= 3;


#endif

#if ! ( defined( REDUCED_CORE_TINYAVR ) && REDUCED_CORE_TINYAVR )
  // The 4/5/9/10 "Reduced Core" have problems with GCC compiling this    
  __asm__ __volatile__ (
    "1: sbiw %0,1" "\n\t" // 2 cycles
    "brne 1b" : "=w" (us) : "0" (us) // 2 cycles
  );
#else
  // However this is fine, and I think it maintains the same 4-clock-per-loop
  // count.  The asm("") prevents the empty loop from being optimized out.
  // This could probably be used instead of the above on other chips, but 
  // leaving it just for the reduced core ones for now.  
  while(us--) asm("");
#endif
  // return = 4 cycles
}

void delayMicrosecondsAdjustedForMillisInterrupt(DelayMicrosecondsTime_t us)
{
#if MILLIS_TIMER_PRESCALE == 1
#if F_CPU >= 24000000UL
  // 24MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 10) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 1.9152 * us
  //        Adjustment Takes: 63 Clocks (Approx)
  us = ( us * 1) + (us / 2) + (us / 4) + (us / 8) + (us / 32) + (us / 128) ;

  // Compensate for the combined overhead time of 3.9684us
  // by subtracting 18 Loop Cycles
  // us at this point is at least 19 Loops - 11us
  //       Subtraction Takes: 2 Clocks
  us -= 18;

#elif F_CPU >= 19200000UL
  // 19.2MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 17) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 1.7346 * us
  //        Adjustment Takes: 104 Clocks (Approx)
  us = ( us * 1) + (us / 2) + (us / 8) + (us / 16) + (us / 32) + (us / 64) ;

  // Compensate for the combined overhead time of 7.7466us
  // by subtracting 28 Loop Cycles
  // us at this point is at least 30 Loops - 18us
  //       Subtraction Takes: 2 Clocks
  us -= 28;
  
#elif F_CPU >= 20000000UL
  // 20MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 13) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 1.9814 * us
  //        Adjustment Takes: 89 Clocks (Approx)
  us = ( us * 1) + (us / 2) + (us / 4) + (us / 8) + (us / 16) + (us / 32) + (us / 128) + (us / 256);

  // Compensate for the combined overhead time of 4.9500us
  // by subtracting 24 Loop Cycles
  // us at this point is at least 25 Loops - 14us
  //       Subtraction Takes: 2 Clocks
  us -= 24;


#elif F_CPU >= 16000000UL
  // 16MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 12) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 1.8028 * us
  //        Adjustment Takes: 75 Clocks (Approx)
  us = ( us * 1) + (us / 2) + (us / 4) + (us / 32) + (us / 64) + (us / 256);

  // Compensate for the combined overhead time of 5.3125us
  // by subtracting 21 Loop Cycles
  // us at this point is at least 22 Loops - 13us
  //       Subtraction Takes: 2 Clocks
  us -= 21;


#elif F_CPU >= 12000000UL
  // 12MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 7) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 1.5673 * us
  //        Adjustment Takes: 32 Clocks (Approx)
  us = ( us * 1) + (us / 2) + (us / 16) + (us / 256);

  // Compensate for the combined overhead time of 3.5000us
  // by subtracting 10 Loop Cycles
  // us at this point is at least 12 Loops - 8us
  //       Subtraction Takes: 2 Clocks
  us -= 10;


#elif F_CPU >= 9600000UL
  // 9.6MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 12) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 1.8395 * us
  //        Adjustment Takes: 73 Clocks (Approx)
  us = ( us * 1) + (us / 2) + (us / 4) + (us / 16) + (us / 64) + (us / 128) ;

  // Compensate for the combined overhead time of 11.2801us
  // by subtracting 21 Loop Cycles
  // us at this point is at least 22 Loops - 13us
  //       Subtraction Takes: 2 Clocks
  us -= 21;


#elif F_CPU >= 8000000UL
  // 8MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 25) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 1.2427 * us
  //        Adjustment Takes: 109 Clocks (Approx)
  us = ( us * 1) + (us / 8) + (us / 16) + (us / 32) + (us / 64) + (us / 128) ;

  // Compensate for the combined overhead time of 14.8750us
  // by subtracting 29 Loop Cycles
  // us at this point is at least 30 Loops - 26us
  //       Subtraction Takes: 2 Clocks
  us -= 29;


#elif F_CPU >= 6400000UL
  // 6.4MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 11) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 1.0756 * us
  //        Adjustment Takes: 37 Clocks (Approx)
  us = ( us * 1) + (us / 16) + (us / 128) + (us / 256);

  // Compensate for the combined overhead time of 7.3438us
  // by subtracting 11 Loop Cycles
  // us at this point is at least 12 Loops - 12us
  //       Subtraction Takes: 2 Clocks
  us -= 11;


#elif F_CPU >= 4800000UL
  // 4.8MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 11) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.8787 * us
  //        Adjustment Takes: 25 Clocks (Approx)
  us = ( us * 0) + (us / 2) + (us / 4) + (us / 8) ;

  // Compensate for the combined overhead time of 7.2917us
  // by subtracting 8 Loop Cycles
  // us at this point is at least 10 Loops - 12us
  //       Subtraction Takes: 2 Clocks
  us -= 8;


#elif F_CPU >= 4000000UL
  // 4MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 17) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.7665 * us
  //        Adjustment Takes: 40 Clocks (Approx)
  us = ( us * 0) + (us / 2) + (us / 4) + (us / 64) ;

  // Compensate for the combined overhead time of 12.5000us
  // by subtracting 12 Loop Cycles
  // us at this point is at least 13 Loops - 18us
  //       Subtraction Takes: 2 Clocks
  us -= 12;


#elif F_CPU >= 2400000UL
  // 2.4MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 9) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.5073 * us
  //        Adjustment Takes: 6 Clocks (Approx)
  us = ( us * 0) + (us / 2) + (us / 256);

  // Compensate for the combined overhead time of 6.6667us
  // by subtracting 4 Loop Cycles
  // us at this point is at least 5 Loops - 10us
  //       Subtraction Takes: 2 Clocks
  us -= 4;


#elif F_CPU >= 2000000UL
  // 2MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 64) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.4339 * us
  //        Adjustment Takes: 93 Clocks (Approx)
  us = ( us * 0) + (us / 4) + (us / 8) + (us / 32) + (us / 64) + (us / 128) + (us / 256);

  // Compensate for the combined overhead time of 51.5000us
  // by subtracting 25 Loop Cycles
  // us at this point is at least 27 Loops - 65us
  //       Subtraction Takes: 2 Clocks
  us -= 25;


#elif F_CPU >= 1200000UL
  // 1.2MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 105) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.2299 * us
  //        Adjustment Takes: 73 Clocks (Approx)
  us = ( us * 0) + (us / 8) + (us / 16) + (us / 32) + (us / 128) ;

  // Compensate for the combined overhead time of 90.2409us
  // by subtracting 20 Loop Cycles
  // us at this point is at least 22 Loops - 106us
  //       Subtraction Takes: 2 Clocks
  us -= 20;


#elif F_CPU >= 1000000UL
  // 1MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 103) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.2323 * us
  //        Adjustment Takes: 77 Clocks (Approx)
  us = ( us * 0) + (us / 8) + (us / 16) + (us / 32) + (us / 128) + (us / 256);

  // Compensate for the combined overhead time of 87.0000us
  // by subtracting 21 Loop Cycles
  // us at this point is at least 22 Loops - 104us
  //       Subtraction Takes: 2 Clocks
  us -= 21;


#elif F_CPU >= 800000UL
  // 800kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 77) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.1885 * us
  //        Adjustment Takes: 39 Clocks (Approx)
  us = ( us * 0) + (us / 8) + (us / 16) ;

  // Compensate for the combined overhead time of 61.2500us
  // by subtracting 12 Loop Cycles
  // us at this point is at least 13 Loops - 78us
  //       Subtraction Takes: 2 Clocks
  us -= 12;


#elif F_CPU >= 600000UL
  // 600kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 113) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.1434 * us
  //        Adjustment Takes: 49 Clocks (Approx)
  us = ( us * 0) + (us / 8) + (us / 64) ;

  // Compensate for the combined overhead time of 98.3333us
  // by subtracting 14 Loop Cycles
  // us at this point is at least 15 Loops - 114us
  //       Subtraction Takes: 2 Clocks
  us -= 14;


#elif F_CPU >= 128000UL
  // 128kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 322) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.0317 * us
  //        Adjustment Takes: 27 Clocks (Approx)
  us = ( us * 0) + (us / 32) ;

  // Compensate for the combined overhead time of 289.0625us
  // by subtracting 9 Loop Cycles
  // us at this point is at least 10 Loops - 323us
  //       Subtraction Takes: 2 Clocks
  us -= 9;


#elif F_CPU >= 125000UL
  // 125kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 552) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.0310 * us
  //        Adjustment Takes: 43 Clocks (Approx)
  us = ( us * 0) + (us / 64) + (us / 128) + (us / 256);

  // Compensate for the combined overhead time of 424.0000us
  // by subtracting 13 Loop Cycles
  // us at this point is at least 14 Loops - 553us
  //       Subtraction Takes: 2 Clocks
  us -= 13;


#elif F_CPU >= 32768UL
  // 32.768kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 642) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.0082 * us
  //        Adjustment Takes: 7 Clocks (Approx)
  us = ( us * 0) + (us / 128) ;

  // Compensate for the combined overhead time of 518.7988us
  // by subtracting 4 Loop Cycles
  // us at this point is at least 5 Loops - 643us
  //       Subtraction Takes: 2 Clocks
  us -= 4;


#elif F_CPU >= 16000UL
  // 16kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 1125) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.0040 * us
  //        Adjustment Takes: 4 Clocks (Approx)
  us = ( us * 0) + (us / 256);

  // Compensate for the combined overhead time of 875.0000us
  // by subtracting 3 Loop Cycles
  // us at this point is at least 4 Loops - 1126us
  //       Subtraction Takes: 2 Clocks
  us -= 3;


#endif
#elif MILLIS_TIMER_PRESCALE == 8
#if F_CPU >= 24000000UL
  // 24MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 6) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 4.7371 * us
  //        Adjustment Takes: 108 Clocks (Approx)
  us = ( us * 4) + (us / 2) + (us / 8) + (us / 16) + (us / 32) + (us / 64) ;

  // Compensate for the combined overhead time of 5.1039us
  // by subtracting 29 Loop Cycles
  // us at this point is at least 31 Loops - 7us
  //       Subtraction Takes: 2 Clocks
  us -= 29;

#elif F_CPU >= 19200000UL
  // 19.2MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 10) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 3.9315 * us
  //        Adjustment Takes: 141 Clocks (Approx)
  us = ( us * 3) + (us / 2) + (us / 4) + (us / 8) + (us / 32) + (us / 64) + (us / 128) ;

  // Compensate for the combined overhead time of 8.1641us
  // by subtracting 37 Loop Cycles
  // us at this point is at least 41 Loops - 11us
  //       Subtraction Takes: 2 Clocks
  us -= 37;
  
#elif F_CPU >= 20000000UL
  // 20MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 4) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 4.2002 * us
  //        Adjustment Takes: 58 Clocks (Approx)
  us = ( us * 4) + (us / 8) + (us / 16) + (us / 128) + (us / 256);

  // Compensate for the combined overhead time of 3.4000us
  // by subtracting 17 Loop Cycles
  // us at this point is at least 20 Loops - 5us
  //       Subtraction Takes: 2 Clocks
  us -= 17;


#elif F_CPU >= 16000000UL
  // 16MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 10) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 3.4712 * us
  //        Adjustment Takes: 122 Clocks (Approx)
  us = ( us * 3) + (us / 4) + (us / 8) + (us / 16) + (us / 32) ;

  // Compensate for the combined overhead time of 8.2500us
  // by subtracting 33 Loop Cycles
  // us at this point is at least 36 Loops - 11us
  //       Subtraction Takes: 2 Clocks
  us -= 33;


#elif F_CPU >= 12000000UL
  // 12MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 6) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 2.6924 * us
  //        Adjustment Takes: 51 Clocks (Approx)
  us = ( us * 2) + (us / 2) + (us / 8) + (us / 16) + (us / 256);

  // Compensate for the combined overhead time of 5.0833us
  // by subtracting 15 Loop Cycles
  // us at this point is at least 17 Loops - 7us
  //       Subtraction Takes: 2 Clocks
  us -= 15;


#elif F_CPU >= 9600000UL
  // 9.6MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 10) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 2.3119 * us
  //        Adjustment Takes: 82 Clocks (Approx)
  us = ( us * 2) + (us / 4) + (us / 32) + (us / 64) + (us / 128) + (us / 256);

  // Compensate for the combined overhead time of 9.9483us
  // by subtracting 23 Loop Cycles
  // us at this point is at least 24 Loops - 11us
  //       Subtraction Takes: 2 Clocks
  us -= 23;


#elif F_CPU >= 8000000UL
  // 8MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 12) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 1.8584 * us
  //        Adjustment Takes: 72 Clocks (Approx)
  us = ( us * 1) + (us / 2) + (us / 4) + (us / 16) + (us / 32) + (us / 128) + (us / 256);

  // Compensate for the combined overhead time of 10.2500us
  // by subtracting 20 Loop Cycles
  // us at this point is at least 22 Loops - 13us
  //       Subtraction Takes: 2 Clocks
  us -= 20;


#elif F_CPU >= 6400000UL
  // 6.4MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 4) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 1.5081 * us
  //        Adjustment Takes: 13 Clocks (Approx)
  us = ( us * 1) + (us / 2) + (us / 128) ;

  // Compensate for the combined overhead time of 3.5938us
  // by subtracting 5 Loop Cycles
  // us at this point is at least 7 Loops - 5us
  //       Subtraction Takes: 2 Clocks
  us -= 5;


#elif F_CPU >= 4800000UL
  // 4.8MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 15) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 1.1476 * us
  //        Adjustment Takes: 57 Clocks (Approx)
  us = ( us * 1) + (us / 8) + (us / 64) + (us / 256);

  // Compensate for the combined overhead time of 13.9583us
  // by subtracting 16 Loop Cycles
  // us at this point is at least 18 Loops - 16us
  //       Subtraction Takes: 2 Clocks
  us -= 16;


#elif F_CPU >= 4000000UL
  // 4MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 27) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.9633 * us
  //        Adjustment Takes: 86 Clocks (Approx)
  us = ( us * 0) + (us / 2) + (us / 4) + (us / 8) + (us / 16) + (us / 64) + (us / 128) ;

  // Compensate for the combined overhead time of 24.0000us
  // by subtracting 24 Loop Cycles
  // us at this point is at least 25 Loops - 28us
  //       Subtraction Takes: 2 Clocks
  us -= 24;


#elif F_CPU >= 2400000UL
  // 2.4MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 33) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.5866 * us
  //        Adjustment Takes: 63 Clocks (Approx)
  us = ( us * 0) + (us / 2) + (us / 16) + (us / 64) + (us / 128) ;

  // Compensate for the combined overhead time of 30.4167us
  // by subtracting 18 Loop Cycles
  // us at this point is at least 19 Loops - 34us
  //       Subtraction Takes: 2 Clocks
  us -= 18;


#elif F_CPU >= 2000000UL
  // 2MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 64) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.4907 * us
  //        Adjustment Takes: 108 Clocks (Approx)
  us = ( us * 0) + (us / 4) + (us / 8) + (us / 16) + (us / 32) + (us / 64) + (us / 256);

  // Compensate for the combined overhead time of 59.0000us
  // by subtracting 29 Loop Cycles
  // us at this point is at least 31 Loops - 65us
  //       Subtraction Takes: 2 Clocks
  us -= 29;


#elif F_CPU >= 1200000UL
  // 1.2MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 49) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.2890 * us
  //        Adjustment Takes: 37 Clocks (Approx)
  us = ( us * 0) + (us / 4) + (us / 32) + (us / 256);

  // Compensate for the combined overhead time of 40.6584us
  // by subtracting 11 Loop Cycles
  // us at this point is at least 13 Loops - 50us
  //       Subtraction Takes: 2 Clocks
  us -= 11;


#elif F_CPU >= 1000000UL
  // 1MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 127) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.2476 * us
  //        Adjustment Takes: 109 Clocks (Approx)
  us = ( us * 0) + (us / 8) + (us / 16) + (us / 32) + (us / 64) + (us / 128) + (us / 256);

  // Compensate for the combined overhead time of 119.0000us
  // by subtracting 29 Loop Cycles
  // us at this point is at least 31 Loops - 128us
  //       Subtraction Takes: 2 Clocks
  us -= 29;


#elif F_CPU >= 800000UL
  // 800kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 87) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.1985 * us
  //        Adjustment Takes: 46 Clocks (Approx)
  us = ( us * 0) + (us / 8) + (us / 16) + (us / 128) ;

  // Compensate for the combined overhead time of 70.0000us
  // by subtracting 14 Loop Cycles
  // us at this point is at least 16 Loops - 88us
  //       Subtraction Takes: 2 Clocks
  us -= 14;


#elif F_CPU >= 600000UL
  // 600kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 127) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.1491 * us
  //        Adjustment Takes: 56 Clocks (Approx)
  us = ( us * 0) + (us / 8) + (us / 64) + (us / 128) ;

  // Compensate for the combined overhead time of 110.0000us
  // by subtracting 16 Loop Cycles
  // us at this point is at least 19 Loops - 128us
  //       Subtraction Takes: 2 Clocks
  us -= 16;


#elif F_CPU >= 128000UL
  // 128kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 322) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.0320 * us
  //        Adjustment Takes: 27 Clocks (Approx)
  us = ( us * 0) + (us / 32) ;

  // Compensate for the combined overhead time of 289.0625us
  // by subtracting 9 Loop Cycles
  // us at this point is at least 10 Loops - 323us
  //       Subtraction Takes: 2 Clocks
  us -= 9;


#elif F_CPU >= 125000UL
  // 125kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 552) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.0312 * us
  //        Adjustment Takes: 43 Clocks (Approx)
  us = ( us * 0) + (us / 64) + (us / 128) + (us / 256);

  // Compensate for the combined overhead time of 424.0000us
  // by subtracting 13 Loop Cycles
  // us at this point is at least 14 Loops - 553us
  //       Subtraction Takes: 2 Clocks
  us -= 13;


#elif F_CPU >= 32768UL
  // 32.768kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 642) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.0082 * us
  //        Adjustment Takes: 7 Clocks (Approx)
  us = ( us * 0) + (us / 128) ;

  // Compensate for the combined overhead time of 518.7988us
  // by subtracting 4 Loop Cycles
  // us at this point is at least 5 Loops - 643us
  //       Subtraction Takes: 2 Clocks
  us -= 4;


#elif F_CPU >= 16000UL
  // 16kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 1125) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.0040 * us
  //        Adjustment Takes: 4 Clocks (Approx)
  us = ( us * 0) + (us / 256);

  // Compensate for the combined overhead time of 875.0000us
  // by subtracting 3 Loop Cycles
  // us at this point is at least 4 Loops - 1126us
  //       Subtraction Takes: 2 Clocks
  us -= 3;


#endif
#elif MILLIS_TIMER_PRESCALE == 64
#if F_CPU >= 24000000UL
  // 24MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 6) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 5.8065 * us
  //        Adjustment Takes: 131 Clocks (Approx)
  us = ( us * 5) + (us / 2) + (us / 4) + (us / 32) + (us / 64) + (us / 128) ;

  // Compensate for the combined overhead time of 5.9030us
  // by subtracting 35 Loop Cycles
  // us at this point is at least 39 Loops - 7us
  //       Subtraction Takes: 2 Clocks
  us -= 35;


#elif F_CPU >= 20000000UL
  // 20MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 3) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 4.8837 * us
  //        Adjustment Takes: 40 Clocks (Approx)
  us = ( us * 4) + (us / 2) + (us / 4) + (us / 8) + (us / 128) ;

  // Compensate for the combined overhead time of 2.5000us
  // by subtracting 12 Loop Cycles
  // us at this point is at least 19 Loops - 4us
  //       Subtraction Takes: 2 Clocks
  us -= 12;

#elif F_CPU >= 19200000UL
  // 19.2MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 4) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 4.6710 * us
  //        Adjustment Takes: 65 Clocks (Approx)
  us = ( us * 4) + (us / 2) + (us / 8) + (us / 32) + (us / 128) + (us / 256);

  // Compensate for the combined overhead time of 3.9248us
  // by subtracting 18 Loop Cycles
  // us at this point is at least 22 Loops - 5us
  //       Subtraction Takes: 2 Clocks
  us -= 18;
  
#elif F_CPU >= 16000000UL
  // 16MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 9) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 3.9253 * us
  //        Adjustment Takes: 134 Clocks (Approx)
  us = ( us * 3) + (us / 2) + (us / 4) + (us / 8) + (us / 32) + (us / 64) ;

  // Compensate for the combined overhead time of 9.0000us
  // by subtracting 36 Loop Cycles
  // us at this point is at least 38 Loops - 10us
  //       Subtraction Takes: 2 Clocks
  us -= 36;


#elif F_CPU >= 12000000UL
  // 12MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 9) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 2.9578 * us
  //        Adjustment Takes: 89 Clocks (Approx)
  us = ( us * 2) + (us / 2) + (us / 4) + (us / 8) + (us / 16) + (us / 64) + (us / 256);

  // Compensate for the combined overhead time of 8.2500us
  // by subtracting 24 Loop Cycles
  // us at this point is at least 28 Loops - 10us
  //       Subtraction Takes: 2 Clocks
  us -= 24;


#elif F_CPU >= 9600000UL
  // 9.6MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 6) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 2.3886 * us
  //        Adjustment Takes: 40 Clocks (Approx)
  us = ( us * 2) + (us / 4) + (us / 8) + (us / 128) + (us / 256);

  // Compensate for the combined overhead time of 5.2331us
  // by subtracting 13 Loop Cycles
  // us at this point is at least 15 Loops - 7us
  //       Subtraction Takes: 2 Clocks
  us -= 13;


#elif F_CPU >= 8000000UL
  // 8MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 13) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 1.9811 * us
  //        Adjustment Takes: 89 Clocks (Approx)
  us = ( us * 1) + (us / 2) + (us / 4) + (us / 8) + (us / 16) + (us / 32) + (us / 128) + (us / 256);

  // Compensate for the combined overhead time of 12.3750us
  // by subtracting 24 Loop Cycles
  // us at this point is at least 25 Loops - 14us
  //       Subtraction Takes: 2 Clocks
  us -= 24;


#elif F_CPU >= 6400000UL
  // 6.4MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 13) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 1.5879 * us
  //        Adjustment Takes: 67 Clocks (Approx)
  us = ( us * 1) + (us / 2) + (us / 16) + (us / 64) + (us / 128) ;

  // Compensate for the combined overhead time of 12.0313us
  // by subtracting 19 Loop Cycles
  // us at this point is at least 21 Loops - 14us
  //       Subtraction Takes: 2 Clocks
  us -= 19;


#elif F_CPU >= 4800000UL
  // 4.8MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 14) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 1.1932 * us
  //        Adjustment Takes: 47 Clocks (Approx)
  us = ( us * 1) + (us / 8) + (us / 16) + (us / 256);

  // Compensate for the combined overhead time of 11.8750us
  // by subtracting 14 Loop Cycles
  // us at this point is at least 16 Loops - 15us
  //       Subtraction Takes: 2 Clocks
  us -= 14;


#elif F_CPU >= 4000000UL
  // 4MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 33) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.9953 * us
  //        Adjustment Takes: 113 Clocks (Approx)
  us = ( us * 0) + (us / 2) + (us / 4) + (us / 8) + (us / 16) + (us / 32) + (us / 64) + (us / 128) ;

  // Compensate for the combined overhead time of 30.7500us
  // by subtracting 30 Loop Cycles
  // us at this point is at least 32 Loops - 34us
  //       Subtraction Takes: 2 Clocks
  us -= 30;


#elif F_CPU >= 2400000UL
  // 2.4MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 31) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.5983 * us
  //        Adjustment Takes: 55 Clocks (Approx)
  us = ( us * 0) + (us / 2) + (us / 16) + (us / 32) + (us / 256);

  // Compensate for the combined overhead time of 27.0833us
  // by subtracting 16 Loop Cycles
  // us at this point is at least 19 Loops - 32us
  //       Subtraction Takes: 2 Clocks
  us -= 16;


#elif F_CPU >= 2000000UL
  // 2MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 68) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.4988 * us
  //        Adjustment Takes: 115 Clocks (Approx)
  us = ( us * 0) + (us / 4) + (us / 8) + (us / 16) + (us / 32) + (us / 64) + (us / 128) + (us / 256);

  // Compensate for the combined overhead time of 62.5000us
  // by subtracting 31 Loop Cycles
  // us at this point is at least 32 Loops - 69us
  //       Subtraction Takes: 2 Clocks
  us -= 31;


#elif F_CPU >= 1200000UL
  // 1.2MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 70) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.2986 * us
  //        Adjustment Takes: 65 Clocks (Approx)
  us = ( us * 0) + (us / 4) + (us / 32) + (us / 64) ;

  // Compensate for the combined overhead time of 62.7975us
  // by subtracting 18 Loop Cycles
  // us at this point is at least 20 Loops - 71us
  //       Subtraction Takes: 2 Clocks
  us -= 18;


#elif F_CPU >= 1000000UL
  // 1MHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 127) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.2497 * us
  //        Adjustment Takes: 109 Clocks (Approx)
  us = ( us * 0) + (us / 8) + (us / 16) + (us / 32) + (us / 64) + (us / 128) + (us / 256);

  // Compensate for the combined overhead time of 119.0000us
  // by subtracting 29 Loop Cycles
  // us at this point is at least 31 Loops - 128us
  //       Subtraction Takes: 2 Clocks
  us -= 29;


#elif F_CPU >= 800000UL
  // 800kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 87) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.1998 * us
  //        Adjustment Takes: 50 Clocks (Approx)
  us = ( us * 0) + (us / 8) + (us / 16) + (us / 128) + (us / 256);

  // Compensate for the combined overhead time of 75.0000us
  // by subtracting 15 Loop Cycles
  // us at this point is at least 16 Loops - 88us
  //       Subtraction Takes: 2 Clocks
  us -= 15;


#elif F_CPU >= 600000UL
  // 600kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 127) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.1499 * us
  //        Adjustment Takes: 56 Clocks (Approx)
  us = ( us * 0) + (us / 8) + (us / 64) + (us / 128) ;

  // Compensate for the combined overhead time of 110.0000us
  // by subtracting 16 Loop Cycles
  // us at this point is at least 19 Loops - 128us
  //       Subtraction Takes: 2 Clocks
  us -= 16;


#elif F_CPU >= 128000UL
  // 128kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 322) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.0320 * us
  //        Adjustment Takes: 27 Clocks (Approx)
  us = ( us * 0) + (us / 32) ;

  // Compensate for the combined overhead time of 289.0625us
  // by subtracting 9 Loop Cycles
  // us at this point is at least 10 Loops - 323us
  //       Subtraction Takes: 2 Clocks
  us -= 9;


#elif F_CPU >= 125000UL
  // 125kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 552) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.0312 * us
  //        Adjustment Takes: 43 Clocks (Approx)
  us = ( us * 0) + (us / 64) + (us / 128) + (us / 256);

  // Compensate for the combined overhead time of 424.0000us
  // by subtracting 13 Loop Cycles
  // us at this point is at least 14 Loops - 553us
  //       Subtraction Takes: 2 Clocks
  us -= 13;


#elif F_CPU >= 32768UL
  // 32.768kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 642) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.0082 * us
  //        Adjustment Takes: 7 Clocks (Approx)
  us = ( us * 0) + (us / 128) ;

  // Compensate for the combined overhead time of 518.7988us
  // by subtracting 4 Loop Cycles
  // us at this point is at least 5 Loops - 643us
  //       Subtraction Takes: 2 Clocks
  us -= 4;


#elif F_CPU >= 16000UL
  // 16kHz

  // Initialisation Overhead: 5 Clocks (Average)

  // Minimum us we can achieve
  //        Comparison Takes: 3 Clocks
  if(us <= 1125) return;

  // Convert us into a loop-counter for a busy-wait loop we will do.
  //   the following approximates 0.0040 * us
  //        Adjustment Takes: 4 Clocks (Approx)
  us = ( us * 0) + (us / 256);

  // Compensate for the combined overhead time of 875.0000us
  // by subtracting 3 Loop Cycles
  // us at this point is at least 4 Loops - 1126us
  //       Subtraction Takes: 2 Clocks
  us -= 3;


#endif
#endif
#if ! ( defined( REDUCED_CORE_TINYAVR ) && REDUCED_CORE_TINYAVR )
  // The 4/5/9/10 "Reduced Core" have problems with GCC compiling this    
  __asm__ __volatile__ (
    "1: sbiw %0,1" "\n\t" // 2 cycles
    "brne 1b" : "=w" (us) : "0" (us) // 2 cycles
  );
#else
  // However this is fine, and I think it maintains the same 4-clock-per-loop
  // count.  The asm("") prevents the empty loop from being optimized out.
  // This could probably be used instead of the above on other chips, but 
  // leaving it just for the reduced core ones for now.  
  while(us--) asm("");
#endif
  // return = 4 cycles
}

#endif
