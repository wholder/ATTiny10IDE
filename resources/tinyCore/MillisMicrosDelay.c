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
  //     Best Error Possible: 0.0146%  (0.0001455417 Decimal)
  //    Worst Error Possible: 26.7588% (0.267588149 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 267588UL
    //  Error: 26.7588% (0.267588149 Decimal)
    // Jitter: 0.0065% (0.0000647321 Decimal)
    x = (ovrf * 0)  + (ovrf / 128);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 84492UL
    //  Error: 8.4492% (0.0844921373 Decimal)
    // Jitter: 0.0098% (0.0000977444 Decimal)
    x = (ovrf * 0)  + (ovrf / 128) + (ovrf / 512);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 38726UL
    //  Error: 3.8726% (0.038726386 Decimal)
    // Jitter: 0.0160% (0.0001597744 Decimal)
    x = (ovrf * 0)  + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 15849UL
    //  Error: 1.5849% (0.015849382 Decimal)
    // Jitter: 0.0188% (0.0001879699 Decimal)
    x = (ovrf * 0)  + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 4096);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 4417UL
    //  Error: 0.4417% (0.0044165132000000005 Decimal)
    // Jitter: 0.0268% (0.0002679426 Decimal)
    x = (ovrf * 0)  + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 8192);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 1566UL
    //  Error: 0.1566% (0.0015659282000000001 Decimal)
    // Jitter: 0.0307% (0.00030690359999999996 Decimal)
    x = (ovrf * 0)  + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 8192) + (ovrf / 32768);
  #else
    //  Error: 0.0146% (0.0001455417 Decimal)
    // Jitter: 0.0372% (0.0003718387 Decimal)
    x = (ovrf * 0)  + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 8192) + (ovrf / 32768) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 20000000UL
  // 20 MHz
  //     Best Error Possible: 0.1080%  (0.0010804776999999998 Decimal)
  //    Worst Error Possible: 38.9659% (0.38965908730000004 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 389659UL
    //  Error: 38.9659% (0.38965908730000004 Decimal)
    // Jitter: 0.0079% (0.0000788352 Decimal)
    x = (ovrf * 0)  + (ovrf / 128);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 84494UL
    //  Error: 8.4494% (0.0844941297 Decimal)
    // Jitter: 0.0163% (0.00016339070000000001 Decimal)
    x = (ovrf * 0)  + (ovrf / 128) + (ovrf / 256);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8211UL
    //  Error: 0.8211% (0.00821125 Decimal)
    // Jitter: 0.0218% (0.0002175182 Decimal)
    x = (ovrf * 0)  + (ovrf / 128) + (ovrf / 256) + (ovrf / 1024);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 3454UL
    //  Error: 0.3454% (0.0034536073999999997 Decimal)
    // Jitter: 0.0284% (0.0002839416 Decimal)
    x = (ovrf * 0)  + (ovrf / 128) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 16384);
  #else
    //  Error: 0.1080% (0.0010804776999999998 Decimal)
    // Jitter: 0.0367% (0.0003666667 Decimal)
    x = (ovrf * 0)  + (ovrf / 128) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 16384) + (ovrf / 32768);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 19200000UL
  // 19.2 MHz
  //     Best Error Possible: 0.0992%  (0.0009916172 Decimal)
  //    Worst Error Possible: 41.4072% (0.4140718057 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 414072UL
    //  Error: 41.4072% (0.4140718057 Decimal)
    // Jitter: 0.0055% (0.0000551471 Decimal)
    x = (ovrf * 0)  + (ovrf / 128);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 121113UL
    //  Error: 12.1113% (0.1211131569 Decimal)
    // Jitter: 0.0106% (0.00010625 Decimal)
    x = (ovrf * 0)  + (ovrf / 128) + (ovrf / 256);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 47881UL
    //  Error: 4.7881% (0.0478814326 Decimal)
    // Jitter: 0.0148% (0.0001484375 Decimal)
    x = (ovrf * 0)  + (ovrf / 128) + (ovrf / 256) + (ovrf / 1024);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11271UL
    //  Error: 1.1271% (0.011270545 Decimal)
    // Jitter: 0.0223% (0.0002229665 Decimal)
    x = (ovrf * 0)  + (ovrf / 128) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 2048);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2126UL
    //  Error: 0.2126% (0.0021257332 Decimal)
    // Jitter: 0.0283% (0.0002829912 Decimal)
    x = (ovrf * 0)  + (ovrf / 128) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 2048) + (ovrf / 8192);
  #else
    //  Error: 0.0992% (0.0009916172 Decimal)
    // Jitter: 0.0329% (0.0003291789 Decimal)
    x = (ovrf * 0)  + (ovrf / 128) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 16000000UL
  // 16 MHz
  //     Best Error Possible: 0.0580%  (0.0005804791 Decimal)
  //    Worst Error Possible: 2.3447% (0.0234473254 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23447UL
    //  Error: 2.3447% (0.0234473254 Decimal)
    // Jitter: 0.0062% (0.0000625 Decimal)
    x = (ovrf * 0)  + (ovrf / 64);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8199UL
    //  Error: 0.8199% (0.008199243200000001 Decimal)
    // Jitter: 0.0120% (0.0001203125 Decimal)
    x = (ovrf * 0)  + (ovrf / 64) + (ovrf / 4096);
  #else
    //  Error: 0.0580% (0.0005804791 Decimal)
    // Jitter: 0.0177% (0.00017671090000000002 Decimal)
    x = (ovrf * 0)  + (ovrf / 64) + (ovrf / 4096) + (ovrf / 8192);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 12000000UL
  // 12 MHz
  //     Best Error Possible: 0.0146%  (0.0001455417 Decimal)
  //    Worst Error Possible: 26.7588% (0.267588149 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 267588UL
    //  Error: 26.7588% (0.267588149 Decimal)
    // Jitter: 0.0065% (0.0000647321 Decimal)
    x = (ovrf * 0)  + (ovrf / 64);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 84492UL
    //  Error: 8.4492% (0.0844921373 Decimal)
    // Jitter: 0.0098% (0.0000977444 Decimal)
    x = (ovrf * 0)  + (ovrf / 64) + (ovrf / 256);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 38726UL
    //  Error: 3.8726% (0.038726386 Decimal)
    // Jitter: 0.0160% (0.0001597744 Decimal)
    x = (ovrf * 0)  + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 15849UL
    //  Error: 1.5849% (0.015849382 Decimal)
    // Jitter: 0.0188% (0.0001879699 Decimal)
    x = (ovrf * 0)  + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 2048);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 4417UL
    //  Error: 0.4417% (0.0044165132000000005 Decimal)
    // Jitter: 0.0268% (0.0002679426 Decimal)
    x = (ovrf * 0)  + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 2048) + (ovrf / 4096);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 1566UL
    //  Error: 0.1566% (0.0015659282000000001 Decimal)
    // Jitter: 0.0307% (0.00030690359999999996 Decimal)
    x = (ovrf * 0)  + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 16384);
  #else
    //  Error: 0.0146% (0.0001455417 Decimal)
    // Jitter: 0.0372% (0.0003718387 Decimal)
    x = (ovrf * 0)  + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 32768);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 9600000UL
  // 9.6 MHz
  //     Best Error Possible: 0.0429%  (0.0004293574 Decimal)
  //    Worst Error Possible: 41.4072% (0.4140718057 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 414072UL
    //  Error: 41.4072% (0.4140718057 Decimal)
    // Jitter: 0.0055% (0.0000551471 Decimal)
    x = (ovrf * 0)  + (ovrf / 64);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 121113UL
    //  Error: 12.1113% (0.1211131569 Decimal)
    // Jitter: 0.0106% (0.00010625 Decimal)
    x = (ovrf * 0)  + (ovrf / 64) + (ovrf / 128);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 47881UL
    //  Error: 4.7881% (0.0478814326 Decimal)
    // Jitter: 0.0148% (0.0001484375 Decimal)
    x = (ovrf * 0)  + (ovrf / 64) + (ovrf / 128) + (ovrf / 512);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11271UL
    //  Error: 1.1271% (0.011270545 Decimal)
    // Jitter: 0.0223% (0.0002229665 Decimal)
    x = (ovrf * 0)  + (ovrf / 64) + (ovrf / 128) + (ovrf / 512) + (ovrf / 1024);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2126UL
    //  Error: 0.2126% (0.0021257332 Decimal)
    // Jitter: 0.0283% (0.0002829912 Decimal)
    x = (ovrf * 0)  + (ovrf / 64) + (ovrf / 128) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 4096);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 992UL
    //  Error: 0.0992% (0.0009916172 Decimal)
    // Jitter: 0.0329% (0.0003291789 Decimal)
    x = (ovrf * 0)  + (ovrf / 64) + (ovrf / 128) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 32768);
  #else
    //  Error: 0.0429% (0.0004293574 Decimal)
    // Jitter: 0.0377% (0.0003769841 Decimal)
    x = (ovrf * 0)  + (ovrf / 64) + (ovrf / 128) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 32768) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 8000000UL
  // 8 MHz
  //     Best Error Possible: 0.0114%  (0.0001138951 Decimal)
  //    Worst Error Possible: 2.3447% (0.0234473254 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23447UL
    //  Error: 2.3447% (0.0234473254 Decimal)
    // Jitter: 0.0062% (0.0000625 Decimal)
    x = (ovrf * 0)  + (ovrf / 32);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8199UL
    //  Error: 0.8199% (0.008199243200000001 Decimal)
    // Jitter: 0.0120% (0.0001203125 Decimal)
    x = (ovrf * 0)  + (ovrf / 32) + (ovrf / 2048);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 580UL
    //  Error: 0.0580% (0.0005804791 Decimal)
    // Jitter: 0.0177% (0.00017671090000000002 Decimal)
    x = (ovrf * 0)  + (ovrf / 32) + (ovrf / 2048) + (ovrf / 4096);
  #else
    //  Error: 0.0114% (0.0001138951 Decimal)
    // Jitter: 0.0224% (0.0002238095 Decimal)
    x = (ovrf * 0)  + (ovrf / 32) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 6400000UL
  // 6.4 MHz
  //     Best Error Possible: 0.0237%  (0.0002371862 Decimal)
  //    Worst Error Possible: 21.8758% (0.21875796979999998 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 218758UL
    //  Error: 21.8758% (0.21875796979999998 Decimal)
    // Jitter: 0.0068% (0.00006818179999999999 Decimal)
    x = (ovrf * 0)  + (ovrf / 32);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 23455UL
    //  Error: 2.3455% (0.023454928400000002 Decimal)
    // Jitter: 0.0108% (0.0001079545 Decimal)
    x = (ovrf * 0)  + (ovrf / 32) + (ovrf / 128);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11258UL
    //  Error: 1.1258% (0.0112578935 Decimal)
    // Jitter: 0.0136% (0.0001362782 Decimal)
    x = (ovrf * 0)  + (ovrf / 32) + (ovrf / 128) + (ovrf / 2048);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 5164UL
    //  Error: 0.5164% (0.0051639588 Decimal)
    // Jitter: 0.0186% (0.00018647909999999998 Decimal)
    x = (ovrf * 0)  + (ovrf / 32) + (ovrf / 128) + (ovrf / 2048) + (ovrf / 4096);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2123UL
    //  Error: 0.2123% (0.0021232221 Decimal)
    // Jitter: 0.0238% (0.0002382033 Decimal)
    x = (ovrf * 0)  + (ovrf / 32) + (ovrf / 128) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 8192);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 608UL
    //  Error: 0.0608% (0.0006080995 Decimal)
    // Jitter: 0.0290% (0.0002903811 Decimal)
    x = (ovrf * 0)  + (ovrf / 32) + (ovrf / 128) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 8192) + (ovrf / 16384);
  #else
    //  Error: 0.0237% (0.0002371862 Decimal)
    // Jitter: 0.0301% (0.0003012704 Decimal)
    x = (ovrf * 0)  + (ovrf / 32) + (ovrf / 128) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 8192) + (ovrf / 16384) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 4800000UL
  // 4.8 MHz
  //     Best Error Possible: 0.0153%  (0.00015272219999999998 Decimal)
  //    Worst Error Possible: 41.4072% (0.4140718057 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 414072UL
    //  Error: 41.4072% (0.4140718057 Decimal)
    // Jitter: 0.0055% (0.0000551471 Decimal)
    x = (ovrf * 0)  + (ovrf / 32);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 121113UL
    //  Error: 12.1113% (0.1211131569 Decimal)
    // Jitter: 0.0106% (0.00010625 Decimal)
    x = (ovrf * 0)  + (ovrf / 32) + (ovrf / 64);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 47881UL
    //  Error: 4.7881% (0.0478814326 Decimal)
    // Jitter: 0.0148% (0.0001484375 Decimal)
    x = (ovrf * 0)  + (ovrf / 32) + (ovrf / 64) + (ovrf / 256);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11271UL
    //  Error: 1.1271% (0.011270545 Decimal)
    // Jitter: 0.0223% (0.0002229665 Decimal)
    x = (ovrf * 0)  + (ovrf / 32) + (ovrf / 64) + (ovrf / 256) + (ovrf / 512);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2126UL
    //  Error: 0.2126% (0.0021257332 Decimal)
    // Jitter: 0.0283% (0.0002829912 Decimal)
    x = (ovrf * 0)  + (ovrf / 32) + (ovrf / 64) + (ovrf / 256) + (ovrf / 512) + (ovrf / 2048);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 992UL
    //  Error: 0.0992% (0.0009916172 Decimal)
    // Jitter: 0.0329% (0.0003291789 Decimal)
    x = (ovrf * 0)  + (ovrf / 32) + (ovrf / 64) + (ovrf / 256) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 16384);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 429UL
    //  Error: 0.0429% (0.0004293574 Decimal)
    // Jitter: 0.0377% (0.0003769841 Decimal)
    x = (ovrf * 0)  + (ovrf / 32) + (ovrf / 64) + (ovrf / 256) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 16384) + (ovrf / 32768);
  #else
    //  Error: 0.0153% (0.00015272219999999998 Decimal)
    // Jitter: 0.0413% (0.0004126984 Decimal)
    x = (ovrf * 0)  + (ovrf / 32) + (ovrf / 64) + (ovrf / 256) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 16384) + (ovrf / 32768) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 4000000UL
  // 4 MHz
  //     Best Error Possible: 0.0114%  (0.0001138951 Decimal)
  //    Worst Error Possible: 2.3447% (0.0234473254 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23447UL
    //  Error: 2.3447% (0.0234473254 Decimal)
    // Jitter: 0.0062% (0.0000625 Decimal)
    x = (ovrf * 0)  + (ovrf / 16);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8199UL
    //  Error: 0.8199% (0.008199243200000001 Decimal)
    // Jitter: 0.0120% (0.0001203125 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 1024);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 580UL
    //  Error: 0.0580% (0.0005804791 Decimal)
    // Jitter: 0.0177% (0.00017671090000000002 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 1024) + (ovrf / 2048);
  #else
    //  Error: 0.0114% (0.0001138951 Decimal)
    // Jitter: 0.0224% (0.0002238095 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 1024) + (ovrf / 2048) + (ovrf / 32768);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 3000000UL
  // 3 MHz
  //     Best Error Possible: 0.0145%  (0.0001452576 Decimal)
  //    Worst Error Possible: 26.7587% (0.2675870743 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 267587UL
    //  Error: 26.7587% (0.2675870743 Decimal)
    // Jitter: 0.0071% (0.00007142860000000001 Decimal)
    x = (ovrf * 0)  + (ovrf / 16);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 84491UL
    //  Error: 8.4491% (0.0844910299 Decimal)
    // Jitter: 0.0107% (0.0001071429 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 64);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 38726UL
    //  Error: 3.8726% (0.0387255216 Decimal)
    // Jitter: 0.0160% (0.0001602836 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 64) + (ovrf / 256);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 15849UL
    //  Error: 1.5849% (0.0158488616 Decimal)
    // Jitter: 0.0186% (0.0001862917 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 512);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 4416UL
    //  Error: 0.4416% (0.0044161647 Decimal)
    // Jitter: 0.0265% (0.00026546640000000004 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 512) + (ovrf / 1024);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 1566UL
    //  Error: 0.1566% (0.0015656226999999998 Decimal)
    // Jitter: 0.0307% (0.0003065773 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 4096);
  #else
    //  Error: 0.0145% (0.0001452576 Decimal)
    // Jitter: 0.0369% (0.0003692755 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 8192);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2500000UL
  // 2.5 MHz
  //     Best Error Possible: 0.0205%  (0.0002048921 Decimal)
  //    Worst Error Possible: 38.9657% (0.3896574804 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 389657UL
    //  Error: 38.9657% (0.3896574804 Decimal)
    // Jitter: 0.0091% (0.0000909091 Decimal)
    x = (ovrf * 0)  + (ovrf / 16);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 84492UL
    //  Error: 8.4492% (0.0844916438 Decimal)
    // Jitter: 0.0182% (0.0001818182 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 32);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8208UL
    //  Error: 0.8208% (0.008208097599999999 Decimal)
    // Jitter: 0.0224% (0.0002235294 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 32) + (ovrf / 128);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 3449UL
    //  Error: 0.3450% (0.0034504404 Decimal)
    // Jitter: 0.0288% (0.0002882353 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 32) + (ovrf / 128) + (ovrf / 2048);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 1077UL
    //  Error: 0.1077% (0.0010773807 Decimal)
    // Jitter: 0.0368% (0.00036803300000000003 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 32) + (ovrf / 128) + (ovrf / 2048) + (ovrf / 4096);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 492UL
    //  Error: 0.0492% (0.0004921641 Decimal)
    // Jitter: 0.0460% (0.0004600141 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 32) + (ovrf / 128) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 16384);
  #else
    //  Error: 0.0205% (0.0002048921 Decimal)
    // Jitter: 0.0556% (0.0005560046 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 32) + (ovrf / 128) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 32768);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2400000UL
  // 2.4 MHz
  //     Best Error Possible: 0.0153%  (0.00015272219999999998 Decimal)
  //    Worst Error Possible: 41.4072% (0.4140718057 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 414072UL
    //  Error: 41.4072% (0.4140718057 Decimal)
    // Jitter: 0.0055% (0.0000551471 Decimal)
    x = (ovrf * 0)  + (ovrf / 16);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 121113UL
    //  Error: 12.1113% (0.1211131569 Decimal)
    // Jitter: 0.0106% (0.00010625 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 32);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 47881UL
    //  Error: 4.7881% (0.0478814326 Decimal)
    // Jitter: 0.0148% (0.0001484375 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 32) + (ovrf / 128);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11271UL
    //  Error: 1.1271% (0.011270545 Decimal)
    // Jitter: 0.0223% (0.0002229665 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 32) + (ovrf / 128) + (ovrf / 256);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2126UL
    //  Error: 0.2126% (0.0021257332 Decimal)
    // Jitter: 0.0283% (0.0002829912 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 32) + (ovrf / 128) + (ovrf / 256) + (ovrf / 1024);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 992UL
    //  Error: 0.0992% (0.0009916172 Decimal)
    // Jitter: 0.0329% (0.0003291789 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 32) + (ovrf / 128) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 8192);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 429UL
    //  Error: 0.0429% (0.0004293574 Decimal)
    // Jitter: 0.0377% (0.0003769841 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 32) + (ovrf / 128) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 8192) + (ovrf / 16384);
  #else
    //  Error: 0.0153% (0.00015272219999999998 Decimal)
    // Jitter: 0.0413% (0.0004126984 Decimal)
    x = (ovrf * 0)  + (ovrf / 16) + (ovrf / 32) + (ovrf / 128) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 8192) + (ovrf / 16384) + (ovrf / 32768);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2000000UL
  // 2 MHz
  //     Best Error Possible: 0.0113%  (0.0001126104 Decimal)
  //    Worst Error Possible: 2.3446% (0.0234464846 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23445UL
    //  Error: 2.3446% (0.0234464846 Decimal)
    // Jitter: 0.0088% (0.00008823529999999999 Decimal)
    x = (ovrf * 0)  + (ovrf / 8);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8198UL
    //  Error: 0.8198% (0.008198212299999999 Decimal)
    // Jitter: 0.0124% (0.0001235294 Decimal)
    x = (ovrf * 0)  + (ovrf / 8) + (ovrf / 512);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 579UL
    //  Error: 0.0579% (0.0005791708 Decimal)
    // Jitter: 0.0179% (0.00017891220000000002 Decimal)
    x = (ovrf * 0)  + (ovrf / 8) + (ovrf / 512) + (ovrf / 1024);
  #else
    //  Error: 0.0113% (0.0001126104 Decimal)
    // Jitter: 0.0224% (0.0002238139 Decimal)
    x = (ovrf * 0)  + (ovrf / 8) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 16384);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1500000UL
  // 1.5 MHz
  //     Best Error Possible: 0.0141%  (0.0001406642 Decimal)
  //    Worst Error Possible: 26.7586% (0.2675857411 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 267586UL
    //  Error: 26.7586% (0.2675857411 Decimal)
    // Jitter: 0.0065% (0.00006493510000000001 Decimal)
    x = (ovrf * 0)  + (ovrf / 8);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 84490UL
    //  Error: 8.4490% (0.08448953660000001 Decimal)
    // Jitter: 0.0118% (0.0001176401 Decimal)
    x = (ovrf * 0)  + (ovrf / 8) + (ovrf / 32);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 38723UL
    //  Error: 3.8723% (0.038723407700000004 Decimal)
    // Jitter: 0.0125% (0.0001247285 Decimal)
    x = (ovrf * 0)  + (ovrf / 8) + (ovrf / 32) + (ovrf / 128);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 15846UL
    //  Error: 1.5846% (0.0158457416 Decimal)
    // Jitter: 0.0191% (0.00019142279999999999 Decimal)
    x = (ovrf * 0)  + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 256);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 4412UL
    //  Error: 0.4412% (0.0044117933 Decimal)
    // Jitter: 0.0270% (0.0002700851 Decimal)
    x = (ovrf * 0)  + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 256) + (ovrf / 512);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 1561UL
    //  Error: 0.1561% (0.0015610432 Decimal)
    // Jitter: 0.0309% (0.00030906229999999997 Decimal)
    x = (ovrf * 0)  + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 256) + (ovrf / 512) + (ovrf / 2048);
  #else
    //  Error: 0.0141% (0.0001406642 Decimal)
    // Jitter: 0.0375% (0.0003745527 Decimal)
    x = (ovrf * 0)  + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 256) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 4096);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1200000UL
  // 1.2 MHz
  //     Best Error Possible: 0.0096%  (0.0000955207 Decimal)
  //    Worst Error Possible: 41.4070% (0.4140701323 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 414069UL
    //  Error: 41.4070% (0.4140701323 Decimal)
    // Jitter: 0.0042% (0.0000424882 Decimal)
    x = (ovrf * 0)  + (ovrf / 8);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 121115UL
    //  Error: 12.1116% (0.1211156348 Decimal)
    // Jitter: 0.0101% (0.00010128149999999999 Decimal)
    x = (ovrf * 0)  + (ovrf / 8) + (ovrf / 16);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 47885UL
    //  Error: 4.7885% (0.047884874900000006 Decimal)
    // Jitter: 0.0141% (0.0001412517 Decimal)
    x = (ovrf * 0)  + (ovrf / 8) + (ovrf / 16) + (ovrf / 64);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11275UL
    //  Error: 1.1275% (0.0112745348 Decimal)
    // Jitter: 0.0216% (0.00021628579999999998 Decimal)
    x = (ovrf * 0)  + (ovrf / 8) + (ovrf / 16) + (ovrf / 64) + (ovrf / 128);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2130UL
    //  Error: 0.2130% (0.0021298607 Decimal)
    // Jitter: 0.0278% (0.0002783623 Decimal)
    x = (ovrf * 0)  + (ovrf / 8) + (ovrf / 16) + (ovrf / 64) + (ovrf / 128) + (ovrf / 512);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 995UL
    //  Error: 0.0996% (0.0009957534 Decimal)
    // Jitter: 0.0329% (0.0003288179 Decimal)
    x = (ovrf * 0)  + (ovrf / 8) + (ovrf / 16) + (ovrf / 64) + (ovrf / 128) + (ovrf / 512) + (ovrf / 4096);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 434UL
    //  Error: 0.0434% (0.000433541 Decimal)
    // Jitter: 0.0374% (0.0003740969 Decimal)
    x = (ovrf * 0)  + (ovrf / 8) + (ovrf / 16) + (ovrf / 64) + (ovrf / 128) + (ovrf / 512) + (ovrf / 4096) + (ovrf / 8192);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 157UL
    //  Error: 0.0157% (0.0001569292 Decimal)
    // Jitter: 0.0410% (0.0004097833 Decimal)
    x = (ovrf * 0)  + (ovrf / 8) + (ovrf / 16) + (ovrf / 64) + (ovrf / 128) + (ovrf / 512) + (ovrf / 4096) + (ovrf / 8192) + (ovrf / 16384);
  #else
    //  Error: 0.0096% (0.0000955207 Decimal)
    // Jitter: 0.0480% (0.0004795937 Decimal)
    x = (ovrf * 0)  + (ovrf / 8) + (ovrf / 16) + (ovrf / 64) + (ovrf / 128) + (ovrf / 512) + (ovrf / 4096) + (ovrf / 8192) + (ovrf / 16384) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1000000UL
  // 1 MHz
  //     Best Error Possible: 0.0063%  (0.0000628935 Decimal)
  //    Worst Error Possible: 2.3445% (0.023445183300000002 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23445UL
    //  Error: 2.3445% (0.023445183300000002 Decimal)
    // Jitter: 0.0083% (0.00008333329999999999 Decimal)
    x = (ovrf * 0)  + (ovrf / 4);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8197UL
    //  Error: 0.8197% (0.0081970502 Decimal)
    // Jitter: 0.0133% (0.0001333333 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 256);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 578UL
    //  Error: 0.0578% (0.0005781245999999999 Decimal)
    // Jitter: 0.0178% (0.00017781720000000002 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 256) + (ovrf / 512);
  #else
    //  Error: 0.0063% (0.0000628935 Decimal)
    // Jitter: 0.0278% (0.0002777817 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 256) + (ovrf / 512) + (ovrf / 8192) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 800000UL
  // 800 kHz
  //     Best Error Possible: 0.0115%  (0.0001145359 Decimal)
  //    Worst Error Possible: 21.8758% (0.21875796979999998 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 218758UL
    //  Error: 21.8758% (0.21875796979999998 Decimal)
    // Jitter: 0.0068% (0.00006818179999999999 Decimal)
    x = (ovrf * 0)  + (ovrf / 4);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 23455UL
    //  Error: 2.3455% (0.023454928400000002 Decimal)
    // Jitter: 0.0108% (0.0001079545 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 16);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11258UL
    //  Error: 1.1258% (0.0112578935 Decimal)
    // Jitter: 0.0136% (0.0001362782 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 256);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 5164UL
    //  Error: 0.5164% (0.0051639588 Decimal)
    // Jitter: 0.0186% (0.00018647909999999998 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 256) + (ovrf / 512);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2123UL
    //  Error: 0.2123% (0.0021232221 Decimal)
    // Jitter: 0.0238% (0.0002382033 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 256) + (ovrf / 512) + (ovrf / 1024);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 608UL
    //  Error: 0.0608% (0.0006080995 Decimal)
    // Jitter: 0.0290% (0.0002903811 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 256) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 2048);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 237UL
    //  Error: 0.0237% (0.0002371862 Decimal)
    // Jitter: 0.0301% (0.0003012704 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 256) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 2048) + (ovrf / 8192);
  #else
    //  Error: 0.0115% (0.0001145359 Decimal)
    // Jitter: 0.0387% (0.0003865699 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 256) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 32768) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 600000UL
  // 600 kHz
  //     Best Error Possible: 0.0091%  (0.0000913535 Decimal)
  //    Worst Error Possible: 41.4068% (0.4140676903 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 414067UL
    //  Error: 41.4068% (0.4140676903 Decimal)
    // Jitter: 0.0039% (0.0000390023 Decimal)
    x = (ovrf * 0)  + (ovrf / 4);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 121112UL
    //  Error: 12.1112% (0.1211119719 Decimal)
    // Jitter: 0.0103% (0.0001027377 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 8);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 47881UL
    //  Error: 4.7881% (0.0478809068 Decimal)
    // Jitter: 0.0144% (0.00014398660000000001 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 32);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11270UL
    //  Error: 1.1270% (0.011270414199999999 Decimal)
    // Jitter: 0.0218% (0.000217923 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 32) + (ovrf / 64);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2126UL
    //  Error: 0.2126% (0.0021257019 Decimal)
    // Jitter: 0.0281% (0.0002812586 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 32) + (ovrf / 64) + (ovrf / 256);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 992UL
    //  Error: 0.0992% (0.0009915898999999998 Decimal)
    // Jitter: 0.0329% (0.0003288179 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 32) + (ovrf / 64) + (ovrf / 256) + (ovrf / 2048);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 429UL
    //  Error: 0.0429% (0.00042937520000000003 Decimal)
    // Jitter: 0.0374% (0.0003740969 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 32) + (ovrf / 64) + (ovrf / 256) + (ovrf / 2048) + (ovrf / 4096);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 153UL
    //  Error: 0.0153% (0.00015276230000000002 Decimal)
    // Jitter: 0.0410% (0.0004097833 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 32) + (ovrf / 64) + (ovrf / 256) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 8192);
  #else
    //  Error: 0.0091% (0.0000913535 Decimal)
    // Jitter: 0.0480% (0.0004795937 Decimal)
    x = (ovrf * 0)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 32) + (ovrf / 64) + (ovrf / 256) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 8192) + (ovrf / 32768);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 500000UL
  // 500 kHz
  //     Best Error Possible: 0.0066%  (0.0000656635 Decimal)
  //    Worst Error Possible: 2.3448% (0.0234479327 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23448UL
    //  Error: 2.3448% (0.0234479327 Decimal)
    // Jitter: 0.0062% (0.000062373 Decimal)
    x = (ovrf * 0)  + (ovrf / 2);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8200UL
    //  Error: 0.8200% (0.008199769 Decimal)
    // Jitter: 0.0120% (0.0001195449 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 128);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 581UL
    //  Error: 0.0581% (0.0005809696 Decimal)
    // Jitter: 0.0176% (0.0001760291 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 128) + (ovrf / 256);
  #else
    //  Error: 0.0066% (0.0000656635 Decimal)
    // Jitter: 0.0278% (0.0002779503 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 128) + (ovrf / 256) + (ovrf / 4096) + (ovrf / 32768);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 375000UL
  // 375 kHz
  //     Best Error Possible: 0.0092%  (0.0000915869 Decimal)
  //    Worst Error Possible: 26.7583% (0.26758349670000003 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 267583UL
    //  Error: 26.7583% (0.26758349670000003 Decimal)
    // Jitter: 0.0074% (0.00007386360000000001 Decimal)
    x = (ovrf * 0)  + (ovrf / 2);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 84487UL
    //  Error: 8.4487% (0.0844867312 Decimal)
    // Jitter: 0.0087% (0.0000871645 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 8);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 38720UL
    //  Error: 3.8720% (0.0387204621 Decimal)
    // Jitter: 0.0127% (0.0001273622 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 15843UL
    //  Error: 1.5843% (0.0158427259 Decimal)
    // Jitter: 0.0196% (0.000196471 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 64);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 4409UL
    //  Error: 0.4409% (0.0044087425 Decimal)
    // Jitter: 0.0275% (0.0002754948 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 64) + (ovrf / 128);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 1558UL
    //  Error: 0.1558% (0.0015579837 Decimal)
    // Jitter: 0.0318% (0.0003182651 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 64) + (ovrf / 128) + (ovrf / 512);
  #else
    //  Error: 0.0092% (0.0000915869 Decimal)
    // Jitter: 0.0435% (0.0004345473 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 64) + (ovrf / 128) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 32768) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 312500UL
  // 312.5 kHz
  //     Best Error Possible: 0.0098%  (0.000098003 Decimal)
  //    Worst Error Possible: 38.9654% (0.3896535267 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 389654UL
    //  Error: 38.9654% (0.3896535267 Decimal)
    // Jitter: 0.0064% (0.0000636364 Decimal)
    x = (ovrf * 0)  + (ovrf / 2);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 84486UL
    //  Error: 8.4486% (0.0844855181 Decimal)
    // Jitter: 0.0145% (0.0001454545 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 4);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8202UL
    //  Error: 0.8202% (0.0082016394 Decimal)
    // Jitter: 0.0227% (0.00022715299999999998 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 3444UL
    //  Error: 0.3444% (0.0034437723 Decimal)
    // Jitter: 0.0290% (0.0002903556 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16) + (ovrf / 256);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 1070UL
    //  Error: 0.1070% (0.001070239 Decimal)
    // Jitter: 0.0375% (0.000374543 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16) + (ovrf / 256) + (ovrf / 512);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 485UL
    //  Error: 0.0485% (0.0004848923 Decimal)
    // Jitter: 0.0464% (0.00046418079999999997 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16) + (ovrf / 256) + (ovrf / 512) + (ovrf / 2048);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 198UL
    //  Error: 0.0198% (0.0001975922 Decimal)
    // Jitter: 0.0557% (0.0005571742000000001 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16) + (ovrf / 256) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 4096);
  #else
    //  Error: 0.0098% (0.000098003 Decimal)
    // Jitter: 0.0678% (0.0006777827 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16) + (ovrf / 256) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 32768) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 300000UL
  // 300 kHz
  //     Best Error Possible: 0.0082%  (0.00008233490000000001 Decimal)
  //    Worst Error Possible: 41.4062% (0.4140624052 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 414062UL
    //  Error: 41.4062% (0.4140624052 Decimal)
    // Jitter: 0.0046% (0.0000456935 Decimal)
    x = (ovrf * 0)  + (ovrf / 2);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 121104UL
    //  Error: 12.1104% (0.12110404429999999 Decimal)
    // Jitter: 0.0072% (0.0000723293 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 4);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 47872UL
    //  Error: 4.7872% (0.0478723188 Decimal)
    // Jitter: 0.0147% (0.0001471747 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11261UL
    //  Error: 1.1261% (0.011261496 Decimal)
    // Jitter: 0.0185% (0.0001845974 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16) + (ovrf / 32);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2117UL
    //  Error: 0.2117% (0.0021167014 Decimal)
    // Jitter: 0.0252% (0.0002515538 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16) + (ovrf / 32) + (ovrf / 128);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 983UL
    //  Error: 0.0983% (0.0009825792 Decimal)
    // Jitter: 0.0310% (0.00031023520000000003 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16) + (ovrf / 32) + (ovrf / 128) + (ovrf / 1024);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 420UL
    //  Error: 0.0420% (0.00042035950000000004 Decimal)
    // Jitter: 0.0382% (0.0003816122 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16) + (ovrf / 32) + (ovrf / 128) + (ovrf / 1024) + (ovrf / 2048);
  #else
    //  Error: 0.0082% (0.00008233490000000001 Decimal)
    // Jitter: 0.0485% (0.0004849638 Decimal)
    x = (ovrf * 0)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16) + (ovrf / 32) + (ovrf / 128) + (ovrf / 1024) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 16384);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 250000UL
  // 250 kHz
  //     Best Error Possible: 0.0055%  (0.0000546916 Decimal)
  //    Worst Error Possible: 2.3437% (0.0234372169 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23437UL
    //  Error: 2.3437% (0.0234372169 Decimal)
    // Jitter: 0.0061% (0.000061198 Decimal)
    x = (ovrf * 1) ;
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8189UL
    //  Error: 0.8189% (0.008188886000000001 Decimal)
    // Jitter: 0.0106% (0.0001060606 Decimal)
    x = (ovrf * 1)  + (ovrf / 64);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 570UL
    //  Error: 0.0570% (0.0005700031000000001 Decimal)
    // Jitter: 0.0184% (0.0001839716 Decimal)
    x = (ovrf * 1)  + (ovrf / 64) + (ovrf / 128);
  #else
    //  Error: 0.0055% (0.0000546916 Decimal)
    // Jitter: 0.0255% (0.0002551896 Decimal)
    x = (ovrf * 1)  + (ovrf / 64) + (ovrf / 128) + (ovrf / 2048) + (ovrf / 16384);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 187500UL
  // 187.5 kHz
  //     Best Error Possible: 0.0085%  (0.0000847758 Decimal)
  //    Worst Error Possible: 26.7578% (0.2675778189 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 267578UL
    //  Error: 26.7578% (0.2675778189 Decimal)
    // Jitter: 0.0055% (0.000054545499999999995 Decimal)
    x = (ovrf * 1) ;
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 84480UL
    //  Error: 8.4480% (0.08447986 Decimal)
    // Jitter: 0.0071% (0.0000709122 Decimal)
    x = (ovrf * 1)  + (ovrf / 4);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 38713UL
    //  Error: 3.8713% (0.0387132482 Decimal)
    // Jitter: 0.0119% (0.0001193403 Decimal)
    x = (ovrf * 1)  + (ovrf / 4) + (ovrf / 16);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 15835UL
    //  Error: 1.5836% (0.0158355798 Decimal)
    // Jitter: 0.0208% (0.00020789609999999998 Decimal)
    x = (ovrf * 1)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 32);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 4402UL
    //  Error: 0.4402% (0.0044019472 Decimal)
    // Jitter: 0.0238% (0.00023807089999999998 Decimal)
    x = (ovrf * 1)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 32) + (ovrf / 64);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 1550UL
    //  Error: 0.1551% (0.0015511828 Decimal)
    // Jitter: 0.0293% (0.00029315179999999996 Decimal)
    x = (ovrf * 1)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 32) + (ovrf / 64) + (ovrf / 256);
  #else
    //  Error: 0.0085% (0.0000847758 Decimal)
    // Jitter: 0.0385% (0.0003851959 Decimal)
    x = (ovrf * 1)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 32) + (ovrf / 64) + (ovrf / 256) + (ovrf / 512) + (ovrf / 16384) + (ovrf / 32768);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 150000UL
  // 150 kHz
  //     Best Error Possible: 0.0082%  (0.00008233490000000001 Decimal)
  //    Worst Error Possible: 41.4062% (0.4140624052 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 414062UL
    //  Error: 41.4062% (0.4140624052 Decimal)
    // Jitter: 0.0046% (0.0000456935 Decimal)
    x = (ovrf * 1) ;
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 121104UL
    //  Error: 12.1104% (0.12110404429999999 Decimal)
    // Jitter: 0.0072% (0.0000723293 Decimal)
    x = (ovrf * 1)  + (ovrf / 2);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 47872UL
    //  Error: 4.7872% (0.0478723188 Decimal)
    // Jitter: 0.0147% (0.0001471747 Decimal)
    x = (ovrf * 1)  + (ovrf / 2) + (ovrf / 8);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11261UL
    //  Error: 1.1261% (0.011261496 Decimal)
    // Jitter: 0.0185% (0.0001845974 Decimal)
    x = (ovrf * 1)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 16);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2117UL
    //  Error: 0.2117% (0.0021167014 Decimal)
    // Jitter: 0.0252% (0.0002515538 Decimal)
    x = (ovrf * 1)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 16) + (ovrf / 64);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 983UL
    //  Error: 0.0983% (0.0009825792 Decimal)
    // Jitter: 0.0310% (0.00031023520000000003 Decimal)
    x = (ovrf * 1)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 16) + (ovrf / 64) + (ovrf / 512);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 420UL
    //  Error: 0.0420% (0.00042035950000000004 Decimal)
    // Jitter: 0.0382% (0.0003816122 Decimal)
    x = (ovrf * 1)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 16) + (ovrf / 64) + (ovrf / 512) + (ovrf / 1024);
  #else
    //  Error: 0.0082% (0.00008233490000000001 Decimal)
    // Jitter: 0.0485% (0.0004849638 Decimal)
    x = (ovrf * 1)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 16) + (ovrf / 64) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 2048) + (ovrf / 8192);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 128000UL
  // 128 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 2) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 125000UL
  // 125 kHz
  //     Best Error Possible: 0.0055%  (0.0000545291 Decimal)
  //    Worst Error Possible: 2.3437% (0.0234373939 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23437UL
    //  Error: 2.3437% (0.0234373939 Decimal)
    // Jitter: 0.0055% (0.0000547528 Decimal)
    x = (ovrf * 2) ;
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8189UL
    //  Error: 0.8189% (0.0081890157 Decimal)
    // Jitter: 0.0107% (0.00010674110000000001 Decimal)
    x = (ovrf * 2)  + (ovrf / 32);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 570UL
    //  Error: 0.0570% (0.0005699991999999999 Decimal)
    // Jitter: 0.0182% (0.0001824126 Decimal)
    x = (ovrf * 2)  + (ovrf / 32) + (ovrf / 64);
  #else
    //  Error: 0.0055% (0.0000545291 Decimal)
    // Jitter: 0.0258% (0.00025814839999999996 Decimal)
    x = (ovrf * 2)  + (ovrf / 32) + (ovrf / 64) + (ovrf / 1024) + (ovrf / 8192) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 100000UL
  // 100 kHz
  //     Best Error Possible: 0.0087%  (0.00008692650000000001 Decimal)
  //    Worst Error Possible: 21.8749% (0.21874940969999998 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 218749UL
    //  Error: 21.8749% (0.21874940969999998 Decimal)
    // Jitter: 0.0040% (0.0000402168 Decimal)
    x = (ovrf * 2) ;
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 23442UL
    //  Error: 2.3442% (0.0234418821 Decimal)
    // Jitter: 0.0066% (0.0000663896 Decimal)
    x = (ovrf * 2)  + (ovrf / 2);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11245UL
    //  Error: 1.1245% (0.0112446471 Decimal)
    // Jitter: 0.0105% (0.0001048328 Decimal)
    x = (ovrf * 2)  + (ovrf / 2) + (ovrf / 32);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 5151UL
    //  Error: 0.5151% (0.0051506819 Decimal)
    // Jitter: 0.0133% (0.00013293700000000002 Decimal)
    x = (ovrf * 2)  + (ovrf / 2) + (ovrf / 32) + (ovrf / 64);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2110UL
    //  Error: 0.2110% (0.0021097311 Decimal)
    // Jitter: 0.0160% (0.0001601772 Decimal)
    x = (ovrf * 2)  + (ovrf / 2) + (ovrf / 32) + (ovrf / 64) + (ovrf / 128);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 594UL
    //  Error: 0.0594% (0.0005942365 Decimal)
    // Jitter: 0.0217% (0.0002168587 Decimal)
    x = (ovrf * 2)  + (ovrf / 2) + (ovrf / 32) + (ovrf / 64) + (ovrf / 128) + (ovrf / 256);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 223UL
    //  Error: 0.0223% (0.0002234391 Decimal)
    // Jitter: 0.0288% (0.00028770170000000004 Decimal)
    x = (ovrf * 2)  + (ovrf / 2) + (ovrf / 32) + (ovrf / 64) + (ovrf / 128) + (ovrf / 256) + (ovrf / 1024);
  #else
    //  Error: 0.0087% (0.00008692650000000001 Decimal)
    // Jitter: 0.0373% (0.0003726614 Decimal)
    x = (ovrf * 2)  + (ovrf / 2) + (ovrf / 32) + (ovrf / 64) + (ovrf / 128) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 8192) + (ovrf / 16384);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 75000UL
  // 75 kHz
  //     Best Error Possible: 0.0068%  (0.00006818429999999999 Decimal)
  //    Worst Error Possible: 12.1094% (0.12109413129999999 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 121094UL
    //  Error: 12.1094% (0.12109413129999999 Decimal)
    // Jitter: 0.0051% (0.000051157400000000006 Decimal)
    x = (ovrf * 3) ;
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 47859UL
    //  Error: 4.7860% (0.0478598407 Decimal)
    // Jitter: 0.0095% (0.00009549769999999999 Decimal)
    x = (ovrf * 3)  + (ovrf / 4);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11248UL
    //  Error: 1.1248% (0.011247735699999999 Decimal)
    // Jitter: 0.0144% (0.00014372999999999998 Decimal)
    x = (ovrf * 3)  + (ovrf / 4) + (ovrf / 8);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2103UL
    //  Error: 0.2103% (0.0021026210000000003 Decimal)
    // Jitter: 0.0207% (0.0002065198 Decimal)
    x = (ovrf * 3)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 32);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 968UL
    //  Error: 0.0968% (0.0009684594 Decimal)
    // Jitter: 0.0249% (0.000248762 Decimal)
    x = (ovrf * 3)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 32) + (ovrf / 256);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 406UL
    //  Error: 0.0406% (0.00040622019999999996 Decimal)
    // Jitter: 0.0303% (0.00030316500000000004 Decimal)
    x = (ovrf * 3)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 32) + (ovrf / 256) + (ovrf / 512);
  #else
    //  Error: 0.0068% (0.00006818429999999999 Decimal)
    // Jitter: 0.0407% (0.00040653280000000003 Decimal)
    x = (ovrf * 3)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 32) + (ovrf / 256) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 4096);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 62500UL
  // 62.5 kHz
  //     Best Error Possible: 0.0053%  (0.000053184600000000006 Decimal)
  //    Worst Error Possible: 2.3437% (0.023436712800000002 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23437UL
    //  Error: 2.3437% (0.023436712800000002 Decimal)
    // Jitter: 0.0055% (0.000055436900000000005 Decimal)
    x = (ovrf * 4) ;
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8188UL
    //  Error: 0.8188% (0.0081878192 Decimal)
    // Jitter: 0.0116% (0.00011633599999999999 Decimal)
    x = (ovrf * 4)  + (ovrf / 16);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 569UL
    //  Error: 0.0569% (0.0005687699 Decimal)
    // Jitter: 0.0188% (0.0001879552 Decimal)
    x = (ovrf * 4)  + (ovrf / 16) + (ovrf / 32);
  #else
    //  Error: 0.0053% (0.000053184600000000006 Decimal)
    // Jitter: 0.0258% (0.0002582281 Decimal)
    x = (ovrf * 4)  + (ovrf / 16) + (ovrf / 32) + (ovrf / 512) + (ovrf / 4096) + (ovrf / 32768) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 37500UL
  // 37.5 kHz
  //     Best Error Possible: 0.0064%  (0.0000640909 Decimal)
  //    Worst Error Possible: 12.1093% (0.1210933981 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 121093UL
    //  Error: 12.1093% (0.1210933981 Decimal)
    // Jitter: 0.0040% (0.000040286 Decimal)
    x = (ovrf * 6) ;
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 47855UL
    //  Error: 4.7856% (0.047856293700000004 Decimal)
    // Jitter: 0.0089% (0.000089106 Decimal)
    x = (ovrf * 6)  + (ovrf / 2);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11243UL
    //  Error: 1.1243% (0.011242939 Decimal)
    // Jitter: 0.0159% (0.0001589788 Decimal)
    x = (ovrf * 6)  + (ovrf / 2) + (ovrf / 4);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2098UL
    //  Error: 0.2098% (0.0020975313000000002 Decimal)
    // Jitter: 0.0220% (0.0002200943 Decimal)
    x = (ovrf * 6)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 963UL
    //  Error: 0.0963% (0.0009634308 Decimal)
    // Jitter: 0.0254% (0.000253992 Decimal)
    x = (ovrf * 6)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16) + (ovrf / 128);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 400UL
    //  Error: 0.0401% (0.0004014773 Decimal)
    // Jitter: 0.0280% (0.0002795405 Decimal)
    x = (ovrf * 6)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16) + (ovrf / 128) + (ovrf / 256);
  #else
    //  Error: 0.0064% (0.0000640909 Decimal)
    // Jitter: 0.0364% (0.0003637025 Decimal)
    x = (ovrf * 6)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16) + (ovrf / 128) + (ovrf / 256) + (ovrf / 512) + (ovrf / 2048);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 32768UL
  // 32.768 kHz
  //     Best Error Possible: 0.0000%  (0 Decimal)
  //    Worst Error Possible: 10.4000% (0.104 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 104000UL
    //  Error: 10.4000% (0.104 Decimal)
    // Jitter: 0.0000% (0 Decimal)
    x = (ovrf * 7) ;
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 40000UL
    //  Error: 4.0000% (0.04 Decimal)
    // Jitter: 0.0000% (0 Decimal)
    x = (ovrf * 7)  + (ovrf / 2);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8000UL
    //  Error: 0.8000% (0.008 Decimal)
    // Jitter: 0.0000% (0 Decimal)
    x = (ovrf * 7)  + (ovrf / 2) + (ovrf / 4);
  #else
    //  Error: 0.0000% (0 Decimal)
    // Jitter: 0.0000% (0 Decimal)
    x = (ovrf * 7)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 16);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 31250UL
  // 31.25 kHz
  //     Best Error Possible: 0.0052%  (0.0000522177 Decimal)
  //    Worst Error Possible: 2.3437% (0.0234370909 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23437UL
    //  Error: 2.3437% (0.0234370909 Decimal)
    // Jitter: 0.0064% (0.00006404550000000001 Decimal)
    x = (ovrf * 8) ;
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8188UL
    //  Error: 0.8188% (0.0081875548 Decimal)
    // Jitter: 0.0111% (0.0001106772 Decimal)
    x = (ovrf * 8)  + (ovrf / 8);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 568UL
    //  Error: 0.0568% (0.0005678393 Decimal)
    // Jitter: 0.0187% (0.0001868103 Decimal)
    x = (ovrf * 8)  + (ovrf / 8) + (ovrf / 16);
  #else
    //  Error: 0.0052% (0.0000522177 Decimal)
    // Jitter: 0.0258% (0.0002581808 Decimal)
    x = (ovrf * 8)  + (ovrf / 8) + (ovrf / 16) + (ovrf / 256) + (ovrf / 2048) + (ovrf / 16384) + (ovrf / 32768);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 18750UL
  // 18.75 kHz
  //     Best Error Possible: 0.0050%  (0.000049767899999999995 Decimal)
  //    Worst Error Possible: 4.7852% (0.047851534099999996 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 47852UL
    //  Error: 4.7852% (0.047851534099999996 Decimal)
    // Jitter: 0.0059% (0.0000586267 Decimal)
    x = (ovrf * 13) ;
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11230UL
    //  Error: 1.1230% (0.011230439200000001 Decimal)
    // Jitter: 0.0061% (0.000060881599999999997 Decimal)
    x = (ovrf * 13)  + (ovrf / 2);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2083UL
    //  Error: 0.2083% (0.0020830862999999997 Decimal)
    // Jitter: 0.0126% (0.00012552049999999999 Decimal)
    x = (ovrf * 13)  + (ovrf / 2) + (ovrf / 8);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 949UL
    //  Error: 0.0949% (0.0009487892000000001 Decimal)
    // Jitter: 0.0168% (0.00016777330000000002 Decimal)
    x = (ovrf * 13)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 64);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 387UL
    //  Error: 0.0387% (0.0003868662 Decimal)
    // Jitter: 0.0190% (0.0001903451 Decimal)
    x = (ovrf * 13)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 64) + (ovrf / 128);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 111UL
    //  Error: 0.0111% (0.0001108822 Decimal)
    // Jitter: 0.0229% (0.0002292078 Decimal)
    x = (ovrf * 13)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 64) + (ovrf / 128) + (ovrf / 256);
  #else
    //  Error: 0.0050% (0.000049767899999999995 Decimal)
    // Jitter: 0.0300% (0.00030018009999999997 Decimal)
    x = (ovrf * 13)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 64) + (ovrf / 128) + (ovrf / 256) + (ovrf / 1024);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 16000UL
  // 16 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 16) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 15625UL
  // 15.625 kHz
  //     Best Error Possible: 0.0046%  (0.0000461119 Decimal)
  //    Worst Error Possible: 2.3437% (0.023437407599999998 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23437UL
    //  Error: 2.3437% (0.023437407599999998 Decimal)
    // Jitter: 0.0058% (0.0000579497 Decimal)
    x = (ovrf * 16) ;
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8184UL
    //  Error: 0.8184% (0.0081838807 Decimal)
    // Jitter: 0.0072% (0.00007187699999999999 Decimal)
    x = (ovrf * 16)  + (ovrf / 4);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 562UL
    //  Error: 0.0562% (0.000562095 Decimal)
    // Jitter: 0.0134% (0.000134475 Decimal)
    x = (ovrf * 16)  + (ovrf / 4) + (ovrf / 8);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 95UL
    //  Error: 0.0095% (0.00009518080000000001 Decimal)
    // Jitter: 0.0183% (0.0001831157 Decimal)
    x = (ovrf * 16)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 128);
  #else
    //  Error: 0.0046% (0.0000461119 Decimal)
    // Jitter: 0.0242% (0.0002421586 Decimal)
    x = (ovrf * 16)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 128) + (ovrf / 1024) + (ovrf / 8192) + (ovrf / 16384) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 12500UL
  // 12.5 kHz
  //     Best Error Possible: 0.0078%  (0.0000782744 Decimal)
  //    Worst Error Possible: 2.3437% (0.023437109 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23437UL
    //  Error: 2.3437% (0.023437109 Decimal)
    // Jitter: 0.0045% (0.000044762300000000003 Decimal)
    x = (ovrf * 20) ;
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11238UL
    //  Error: 1.1238% (0.0112376471 Decimal)
    // Jitter: 0.0097% (0.00009662080000000001 Decimal)
    x = (ovrf * 20)  + (ovrf / 4);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 5143UL
    //  Error: 0.5143% (0.0051427518 Decimal)
    // Jitter: 0.0126% (0.0001263876 Decimal)
    x = (ovrf * 20)  + (ovrf / 4) + (ovrf / 8);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2101UL
    //  Error: 0.2101% (0.0021012148000000004 Decimal)
    // Jitter: 0.0180% (0.00017975239999999998 Decimal)
    x = (ovrf * 20)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 16);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 586UL
    //  Error: 0.0586% (0.0005856277 Decimal)
    // Jitter: 0.0225% (0.00022505439999999998 Decimal)
    x = (ovrf * 20)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 16) + (ovrf / 32);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 214UL
    //  Error: 0.0215% (0.000214637 Decimal)
    // Jitter: 0.0295% (0.0002951753 Decimal)
    x = (ovrf * 20)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 16) + (ovrf / 32) + (ovrf / 128);
  #else
    //  Error: 0.0078% (0.0000782744 Decimal)
    // Jitter: 0.0369% (0.00036935969999999995 Decimal)
    x = (ovrf * 20)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 16) + (ovrf / 32) + (ovrf / 128) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 2048) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 9375UL
  // 9.375 kHz
  //     Best Error Possible: 0.0050%  (0.000049767899999999995 Decimal)
  //    Worst Error Possible: 1.1230% (0.011230439200000001 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 11230UL
    //  Error: 1.1230% (0.011230439200000001 Decimal)
    // Jitter: 0.0061% (0.000060881599999999997 Decimal)
    x = (ovrf * 27) ;
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2083UL
    //  Error: 0.2083% (0.0020830862999999997 Decimal)
    // Jitter: 0.0126% (0.00012552049999999999 Decimal)
    x = (ovrf * 27)  + (ovrf / 4);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 949UL
    //  Error: 0.0949% (0.0009487892000000001 Decimal)
    // Jitter: 0.0168% (0.00016777330000000002 Decimal)
    x = (ovrf * 27)  + (ovrf / 4) + (ovrf / 32);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 387UL
    //  Error: 0.0387% (0.0003868662 Decimal)
    // Jitter: 0.0190% (0.0001903451 Decimal)
    x = (ovrf * 27)  + (ovrf / 4) + (ovrf / 32) + (ovrf / 64);
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 111UL
    //  Error: 0.0111% (0.0001108822 Decimal)
    // Jitter: 0.0229% (0.0002292078 Decimal)
    x = (ovrf * 27)  + (ovrf / 4) + (ovrf / 32) + (ovrf / 64) + (ovrf / 128);
  #else
    //  Error: 0.0050% (0.000049767899999999995 Decimal)
    // Jitter: 0.0300% (0.00030018009999999997 Decimal)
    x = (ovrf * 27)  + (ovrf / 4) + (ovrf / 32) + (ovrf / 64) + (ovrf / 128) + (ovrf / 512) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 4096UL
  // 4.096 kHz
  //     Best Error Possible: 0.0000%  (0 Decimal)
  //    Worst Error Possible: 0.8000% (0.008 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 8000UL
    //  Error: 0.8000% (0.008 Decimal)
    // Jitter: 0.0000% (0 Decimal)
    x = (ovrf * 62) ;
  #else
    //  Error: 0.0000% (0 Decimal)
    // Jitter: 0.0000% (0 Decimal)
    x = (ovrf * 62)  + (ovrf / 2);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2000UL
  // 2 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 128) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1953UL
  // 1.953 kHz
  //     Best Error Possible: 0.0031%  (0.0000310192 Decimal)
  //    Worst Error Possible: 0.0613% (0.0006130248 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 613UL
    //  Error: 0.0613% (0.0006130248 Decimal)
    // Jitter: 0.0055% (0.000055248600000000005 Decimal)
    x = (ovrf * 131) ;
  #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 146UL
    //  Error: 0.0146% (0.0001464549 Decimal)
    // Jitter: 0.0085% (0.00008475830000000001 Decimal)
    x = (ovrf * 131)  + (ovrf / 16);
  #else
    //  Error: 0.0031% (0.0000310192 Decimal)
    // Jitter: 0.0141% (0.0001412729 Decimal)
    x = (ovrf * 131)  + (ovrf / 16) + (ovrf / 64) + (ovrf / 512) + (ovrf / 4096) + (ovrf / 16384);
  #endif


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
  //     Best Error Possible: 0.0074%  (0.0000739307 Decimal)
  //    Worst Error Possible: 6.2500% (0.0625000838 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 62500UL
    //  Error: 6.2500% (0.0625000838 Decimal)
    // Jitter: 0.0060% (0.000059682499999999997 Decimal)
    x = (ovrf * 10) ;
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 15636UL
    //  Error: 1.5636% (0.0156355095 Decimal)
    // Jitter: 0.0082% (0.0000816024 Decimal)
    x = (ovrf * 10)  + (ovrf / 2);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 3927UL
    //  Error: 0.3927% (0.0039271237 Decimal)
    // Jitter: 0.0094% (0.00009356999999999999 Decimal)
    x = (ovrf * 10)  + (ovrf / 2) + (ovrf / 8);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 1007UL
    //  Error: 0.1007% (0.0010074724 Decimal)
    // Jitter: 0.0153% (0.0001532351 Decimal)
    x = (ovrf * 10)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 285UL
    //  Error: 0.0285% (0.000285344 Decimal)
    // Jitter: 0.0213% (0.0002133005 Decimal)
    x = (ovrf * 10)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 128);
  #else
    //  Error: 0.0074% (0.0000739307 Decimal)
    // Jitter: 0.0315% (0.00031501690000000003 Decimal)
    x = (ovrf * 10)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 32768);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 20000000UL
  // 20 MHz
  //     Best Error Possible: 0.0068%  (0.000067535 Decimal)
  //    Worst Error Possible: 6.2500% (0.0625000152 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 62500UL
    //  Error: 6.2500% (0.0625000152 Decimal)
    // Jitter: 0.0055% (0.0000554853 Decimal)
    x = (ovrf * 12) ;
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 23443UL
    //  Error: 2.3443% (0.0234427938 Decimal)
    // Jitter: 0.0100% (0.000099531 Decimal)
    x = (ovrf * 12)  + (ovrf / 2);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 3919UL
    //  Error: 0.3919% (0.0039191754 Decimal)
    // Jitter: 0.0112% (0.0001121674 Decimal)
    x = (ovrf * 12)  + (ovrf / 2) + (ovrf / 4);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 1487UL
    //  Error: 0.1488% (0.001487974 Decimal)
    // Jitter: 0.0139% (0.00013947550000000002 Decimal)
    x = (ovrf * 12)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 32);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 277UL
    //  Error: 0.0277% (0.0002772847 Decimal)
    // Jitter: 0.0176% (0.00017571119999999999 Decimal)
    x = (ovrf * 12)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 32) + (ovrf / 64);
  #else
    //  Error: 0.0068% (0.000067535 Decimal)
    // Jitter: 0.0318% (0.0003179226 Decimal)
    x = (ovrf * 12)  + (ovrf / 2) + (ovrf / 4) + (ovrf / 32) + (ovrf / 64) + (ovrf / 512) + (ovrf / 1024) + (ovrf / 8192) + (ovrf / 16384);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 19200000UL
  // 19.2 MHz
  //     Best Error Possible: 0.0065%  (0.0000651144 Decimal)
  //    Worst Error Possible: 2.5000% (0.025 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 25000UL
    //  Error: 2.5000% (0.025 Decimal)
    // Jitter: 0.0000% (0 Decimal)
    x = (ovrf * 13) ;
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 6258UL
    //  Error: 0.6258% (0.0062577905 Decimal)
    // Jitter: 0.0058% (0.0000576923 Decimal)
    x = (ovrf * 13)  + (ovrf / 4);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 1580UL
    //  Error: 0.1580% (0.0015803789 Decimal)
    // Jitter: 0.0138% (0.0001375 Decimal)
    x = (ovrf * 13)  + (ovrf / 4) + (ovrf / 16);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 419UL
    //  Error: 0.0419% (0.0004190177 Decimal)
    // Jitter: 0.0209% (0.000209375 Decimal)
    x = (ovrf * 13)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 64);
  #else
    //  Error: 0.0065% (0.0000651144 Decimal)
    // Jitter: 0.0384% (0.000384375 Decimal)
    x = (ovrf * 13)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 16000000UL
  // 16 MHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 16) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 12000000UL
  // 12 MHz
  //     Best Error Possible: 0.0060%  (0.0000600665 Decimal)
  //    Worst Error Possible: 1.5625% (0.0156250974 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 15625UL
    //  Error: 1.5625% (0.0156250974 Decimal)
    // Jitter: 0.0057% (0.0000572337 Decimal)
    x = (ovrf * 21) ;
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 3914UL
    //  Error: 0.3914% (0.0039141093 Decimal)
    // Jitter: 0.0117% (0.0001169239 Decimal)
    x = (ovrf * 21)  + (ovrf / 4);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 994UL
    //  Error: 0.0994% (0.0009938101 Decimal)
    // Jitter: 0.0127% (0.00012672400000000002 Decimal)
    x = (ovrf * 21)  + (ovrf / 4) + (ovrf / 16);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 272UL
    //  Error: 0.0272% (0.0002715227 Decimal)
    // Jitter: 0.0167% (0.0001670341 Decimal)
    x = (ovrf * 21)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 64);
  #else
    //  Error: 0.0060% (0.0000600665 Decimal)
    // Jitter: 0.0258% (0.00025756650000000004 Decimal)
    x = (ovrf * 21)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 9600000UL
  // 9.6 MHz
  //     Best Error Possible: 0.0068%  (0.0000677557 Decimal)
  //    Worst Error Possible: 2.5000% (0.0249999037 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 25000UL
    //  Error: 2.5000% (0.0249999037 Decimal)
    // Jitter: 0.0057% (0.000056533099999999996 Decimal)
    x = (ovrf * 26) ;
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 6260UL
    //  Error: 0.6260% (0.0062602516 Decimal)
    // Jitter: 0.0068% (0.00006789860000000001 Decimal)
    x = (ovrf * 26)  + (ovrf / 2);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 1583UL
    //  Error: 0.1583% (0.0015833386 Decimal)
    // Jitter: 0.0135% (0.0001353933 Decimal)
    x = (ovrf * 26)  + (ovrf / 2) + (ovrf / 8);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 422UL
    //  Error: 0.0422% (0.0004219071 Decimal)
    // Jitter: 0.0207% (0.0002065505 Decimal)
    x = (ovrf * 26)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32);
  #else
    //  Error: 0.0068% (0.0000677557 Decimal)
    // Jitter: 0.0384% (0.0003841938 Decimal)
    x = (ovrf * 26)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 32768);
  #endif


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
  //     Best Error Possible: 0.0054%  (0.0000540151 Decimal)
  //    Worst Error Possible: 0.6250% (0.0062499106 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 6250UL
    //  Error: 0.6250% (0.0062499106 Decimal)
    // Jitter: 0.0063% (0.0000633639 Decimal)
    x = (ovrf * 53) ;
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 1570UL
    //  Error: 0.1570% (0.0015704186 Decimal)
    // Jitter: 0.0103% (0.0001027703 Decimal)
    x = (ovrf * 53)  + (ovrf / 4);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 408UL
    //  Error: 0.0408% (0.00040835000000000003 Decimal)
    // Jitter: 0.0162% (0.0001622073 Decimal)
    x = (ovrf * 53)  + (ovrf / 4) + (ovrf / 16);
  #else
    //  Error: 0.0054% (0.0000540151 Decimal)
    // Jitter: 0.0292% (0.0002917454 Decimal)
    x = (ovrf * 53)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 4000000UL
  // 4 MHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 64) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 3000000UL
  // 3 MHz
  //     Best Error Possible: 0.0046%  (0.0000462079 Decimal)
  //    Worst Error Possible: 0.3906% (0.00390625 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 3906UL
    //  Error: 0.3906% (0.00390625 Decimal)
    // Jitter: 0.0000% (0 Decimal)
    x = (ovrf * 85) ;
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 982UL
    //  Error: 0.0982% (0.000981706 Decimal)
    // Jitter: 0.0023% (0.0000232548 Decimal)
    x = (ovrf * 85)  + (ovrf / 4);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 258UL
    //  Error: 0.0258% (0.00025814269999999996 Decimal)
    // Jitter: 0.0086% (0.0000860414 Decimal)
    x = (ovrf * 85)  + (ovrf / 4) + (ovrf / 16);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 85UL
    //  Error: 0.0085% (0.00008495610000000001 Decimal)
    // Jitter: 0.0137% (0.0001366749 Decimal)
    x = (ovrf * 85)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 64);
  #else
    //  Error: 0.0046% (0.0000462079 Decimal)
    // Jitter: 0.0191% (0.0001911724 Decimal)
    x = (ovrf * 85)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2500000UL
  // 2.5 MHz
  //     Best Error Possible: 0.0059%  (0.0000587247 Decimal)
  //    Worst Error Possible: 0.3910% (0.0039103564 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 3910UL
    //  Error: 0.3910% (0.0039103564 Decimal)
    // Jitter: 0.0019% (0.0000186331 Decimal)
    x = (ovrf * 102) ;
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 1479UL
    //  Error: 0.1479% (0.0014791701 Decimal)
    // Jitter: 0.0083% (0.00008343949999999999 Decimal)
    x = (ovrf * 102)  + (ovrf / 4);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 269UL
    //  Error: 0.0269% (0.00026853840000000005 Decimal)
    // Jitter: 0.0136% (0.0001361274 Decimal)
    x = (ovrf * 102)  + (ovrf / 4) + (ovrf / 8);
  #else
    //  Error: 0.0059% (0.0000587247 Decimal)
    // Jitter: 0.0293% (0.0002925152 Decimal)
    x = (ovrf * 102)  + (ovrf / 4) + (ovrf / 8) + (ovrf / 64) + (ovrf / 128) + (ovrf / 1024) + (ovrf / 2048) + (ovrf / 16384) + (ovrf / 32768);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2400000UL
  // 2.4 MHz
  //     Best Error Possible: 0.0056%  (0.0000559561 Decimal)
  //    Worst Error Possible: 0.6250% (0.0062500981 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 6250UL
    //  Error: 0.6250% (0.0062500981 Decimal)
    // Jitter: 0.0058% (0.0000576321 Decimal)
    x = (ovrf * 106) ;
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 1573UL
    //  Error: 0.1573% (0.0015726446 Decimal)
    // Jitter: 0.0075% (0.0000746327 Decimal)
    x = (ovrf * 106)  + (ovrf / 2);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 411UL
    //  Error: 0.0411% (0.00041096 Decimal)
    // Jitter: 0.0153% (0.00015339150000000002 Decimal)
    x = (ovrf * 106)  + (ovrf / 2) + (ovrf / 8);
  #else
    //  Error: 0.0056% (0.0000559561 Decimal)
    // Jitter: 0.0288% (0.00028813900000000003 Decimal)
    x = (ovrf * 106)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 32768);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2000000UL
  // 2 MHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 128) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1500000UL
  // 1.5 MHz
  //     Best Error Possible: 0.0046%  (0.0000463401 Decimal)
  //    Worst Error Possible: 0.3913% (0.0039131025 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 3913UL
    //  Error: 0.3913% (0.0039131025 Decimal)
    // Jitter: 0.0031% (0.0000311693 Decimal)
    x = (ovrf * 170) ;
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 983UL
    //  Error: 0.0983% (0.0009834352 Decimal)
    // Jitter: 0.0031% (0.000031261 Decimal)
    x = (ovrf * 170)  + (ovrf / 2);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 259UL
    //  Error: 0.0259% (0.0002586137 Decimal)
    // Jitter: 0.0088% (0.0000882548 Decimal)
    x = (ovrf * 170)  + (ovrf / 2) + (ovrf / 8);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 85UL
    //  Error: 0.0085% (0.0000851365 Decimal)
    // Jitter: 0.0138% (0.0001383519 Decimal)
    x = (ovrf * 170)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32);
  #else
    //  Error: 0.0046% (0.0000463401 Decimal)
    // Jitter: 0.0193% (0.0001928854 Decimal)
    x = (ovrf * 170)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 32768);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1200000UL
  // 1.2 MHz
  //     Best Error Possible: 0.0042%  (0.0000424244 Decimal)
  //    Worst Error Possible: 0.1562% (0.0015623082 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 1562UL
    //  Error: 0.1562% (0.0015623082 Decimal)
    // Jitter: 0.0062% (0.0000617729 Decimal)
    x = (ovrf * 213) ;
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 398UL
    //  Error: 0.0398% (0.00039814280000000004 Decimal)
    // Jitter: 0.0092% (0.0000920643 Decimal)
    x = (ovrf * 213)  + (ovrf / 4);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 115UL
    //  Error: 0.0115% (0.0001147225 Decimal)
    // Jitter: 0.0112% (0.000112258 Decimal)
    x = (ovrf * 213)  + (ovrf / 4) + (ovrf / 16);
  #else
    //  Error: 0.0042% (0.0000424244 Decimal)
    // Jitter: 0.0204% (0.0002038113 Decimal)
    x = (ovrf * 213)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 65536);
  #endif


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
  //     Best Error Possible: 0.0030%  (0.0000295508 Decimal)
  //    Worst Error Possible: 0.1557% (0.0015565354 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 1557UL
    //  Error: 0.1557% (0.0015565354 Decimal)
    // Jitter: 0.0032% (0.0000319847 Decimal)
    x = (ovrf * 426) ;
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 388UL
    //  Error: 0.0389% (0.0003892342 Decimal)
    // Jitter: 0.0047% (0.0000470348 Decimal)
    x = (ovrf * 426)  + (ovrf / 2);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 104UL
    //  Error: 0.0104% (0.000104255 Decimal)
    // Jitter: 0.0131% (0.0001305673 Decimal)
    x = (ovrf * 426)  + (ovrf / 2) + (ovrf / 8);
  #else
    //  Error: 0.0030% (0.0000295508 Decimal)
    // Jitter: 0.0204% (0.0002038113 Decimal)
    x = (ovrf * 426)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 32768);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 500000UL
  // 500 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 512) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 375000UL
  // 375 kHz
  //     Best Error Possible: 0.0026%  (0.0000256368 Decimal)
  //    Worst Error Possible: 0.0976% (0.0009764631 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 976UL
    //  Error: 0.0976% (0.0009764631 Decimal)
    // Jitter: 0.0065% (0.0000653312 Decimal)
    x = (ovrf * 682) ;
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 244UL
    //  Error: 0.0244% (0.0002440412 Decimal)
    // Jitter: 0.0065% (0.00006537909999999999 Decimal)
    x = (ovrf * 682)  + (ovrf / 2);
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 68UL
    //  Error: 0.0068% (0.0000675028 Decimal)
    // Jitter: 0.0087% (0.00008738760000000001 Decimal)
    x = (ovrf * 682)  + (ovrf / 2) + (ovrf / 8);
  #else
    //  Error: 0.0026% (0.0000256368 Decimal)
    // Jitter: 0.0133% (0.0001331647 Decimal)
    x = (ovrf * 682)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 32768);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 312500UL
  // 312.5 kHz
  //     Best Error Possible: 0.0024%  (0.0000237285 Decimal)
  //    Worst Error Possible: 0.0244% (0.0002439502 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 244UL
    //  Error: 0.0244% (0.0002439502 Decimal)
    // Jitter: 0.0068% (0.0000678012 Decimal)
    x = (ovrf * 819) ;
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 97UL
    //  Error: 0.0097% (0.0000972144 Decimal)
    // Jitter: 0.0098% (0.000098088 Decimal)
    x = (ovrf * 819)  + (ovrf / 8);
  #else
    //  Error: 0.0024% (0.0000237285 Decimal)
    // Jitter: 0.0174% (0.00017438310000000002 Decimal)
    x = (ovrf * 819)  + (ovrf / 8) + (ovrf / 16) + (ovrf / 128) + (ovrf / 256) + (ovrf / 2048) + (ovrf / 4096) + (ovrf / 32768) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 300000UL
  // 300 kHz
  //     Best Error Possible: 0.0028%  (0.0000281963 Decimal)
  //    Worst Error Possible: 0.0391% (0.0003907308 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 391UL
    //  Error: 0.0391% (0.0003907308 Decimal)
    // Jitter: 0.0066% (0.0000655335 Decimal)
    x = (ovrf * 853) ;
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 105UL
    //  Error: 0.0105% (0.0001053768 Decimal)
    // Jitter: 0.0123% (0.00012291370000000002 Decimal)
    x = (ovrf * 853)  + (ovrf / 4);
  #else
    //  Error: 0.0028% (0.0000281963 Decimal)
    // Jitter: 0.0209% (0.00020863839999999998 Decimal)
    x = (ovrf * 853)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 250000UL
  // 250 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 1024) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 187500UL
  // 187.5 kHz
  //     Best Error Possible: 0.0026%  (0.0000256368 Decimal)
  //    Worst Error Possible: 0.0244% (0.0002440412 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 244UL
    //  Error: 0.0244% (0.0002440412 Decimal)
    // Jitter: 0.0065% (0.00006537909999999999 Decimal)
    x = (ovrf * 1365) ;
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 68UL
    //  Error: 0.0068% (0.0000675028 Decimal)
    // Jitter: 0.0087% (0.00008738760000000001 Decimal)
    x = (ovrf * 1365)  + (ovrf / 4);
  #else
    //  Error: 0.0026% (0.0000256368 Decimal)
    // Jitter: 0.0133% (0.0001331647 Decimal)
    x = (ovrf * 1365)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 150000UL
  // 150 kHz
  //     Best Error Possible: 0.0023%  (0.000023453 Decimal)
  //    Worst Error Possible: 0.0390% (0.0003903993 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 390UL
    //  Error: 0.0390% (0.0003903993 Decimal)
    // Jitter: 0.0067% (0.00006693890000000001 Decimal)
    x = (ovrf * 1706) ;
  #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 101UL
    //  Error: 0.0101% (0.00010142050000000001 Decimal)
    // Jitter: 0.0094% (0.00009416209999999999 Decimal)
    x = (ovrf * 1706)  + (ovrf / 2);
  #else
    //  Error: 0.0023% (0.000023453 Decimal)
    // Jitter: 0.0167% (0.00016740600000000002 Decimal)
    x = (ovrf * 1706)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 32768);
  #endif


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
  //     Best Error Possible: 0.0013%  (0.00001288 Decimal)
  //    Worst Error Possible: 0.0098% (0.0000979598 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 98UL
    //  Error: 0.0098% (0.0000979598 Decimal)
    // Jitter: 0.0073% (0.0000732332 Decimal)
    x = (ovrf * 3413) ;
  #else
    //  Error: 0.0013% (0.00001288 Decimal)
    // Jitter: 0.0146% (0.00014647720000000001 Decimal)
    x = (ovrf * 3413)  + (ovrf / 4) + (ovrf / 16) + (ovrf / 64) + (ovrf / 256) + (ovrf / 1024) + (ovrf / 4096) + (ovrf / 16384) + (ovrf / 65536);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 62500UL
  // 62.5 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  x = (ovrf * 4096) ;


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 37500UL
  // 37.5 kHz
  //     Best Error Possible: 0.0008%  (0.0000076401 Decimal)
  //    Worst Error Possible: 0.0098% (0.00009788590000000001 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 98UL
    //  Error: 0.0098% (0.00009788590000000001 Decimal)
    // Jitter: 0.0073% (0.0000732332 Decimal)
    x = (ovrf * 6826) ;
  #else
    //  Error: 0.0008% (0.0000076401 Decimal)
    // Jitter: 0.0146% (0.00014647720000000001 Decimal)
    x = (ovrf * 6826)  + (ovrf / 2) + (ovrf / 8) + (ovrf / 32) + (ovrf / 128) + (ovrf / 512) + (ovrf / 2048) + (ovrf / 8192) + (ovrf / 32768);
  #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 32768UL
  // 32.768 kHz
  //     Best Error Possible: 0.0003%  (0.0000030611000000000002 Decimal)
  //    Worst Error Possible: 0.0066% (0.0000655305 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 66UL
    //  Error: 0.0066% (0.0000655305 Decimal)
    // Jitter: 0.0064% (0.00006399179999999999 Decimal)
    x = (ovrf * 7812) ;
  #else
    //  Error: 0.0003% (0.0000030611000000000002 Decimal)
    // Jitter: 0.0128% (0.0001279918 Decimal)
    x = (ovrf * 7812)  + (ovrf / 2);
  #endif


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
