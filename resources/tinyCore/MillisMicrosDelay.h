#include "Arduino.h"
#ifndef MillisMicrosDelay_h
#define MillisMicrosDelay_h
#if defined(USE_NEW_MILLIS) && USE_NEW_MILLIS
/**
  * millis(), micros(), delay(), delayMicroseconds(), and REAL_MILLIS(), REAL_MICROS()
  * 
  *  Authored by James Sleeman ( james@gogo.co.nz , http://sparks.gogo.co.nz/ ) 
  *  to replace the functions provided by wiring.c (wiring_lite.c) with the following
  *  goals...
  * 
  *  1. Handle more frequencies properly. 
  *  2. Have better accuracy vs real world for millis() and micros()
  *  3. Be able to configure the accuracy you want, versus using less code space.
  *  4. Be able to easily compensate for low accuracy when doing common types of operation.
  *  5. Simplify/standardize operations.
  *
  *  Defined In This File
  *  ----------------------------------------------------------------------
  *  millis() - the number of milliseconds since boot/overflow
  *  micros() - the number of microseconds since boot/overflow
  *  delay()  - delay for a number of milliseconds
  * 
  *  delayMicroseconds()                  - delay for a number of microseconds
  *       Note that, like the Arduino core itself, delayMicroseconds() does not
  *       disable interrupts during it's execution, nor does it attempt to 
  *       compensate for interrupts happening during it.  If you call this function
  *       with interrupts enabled (and without NO_MILLIS), the timing will be wrong.
  * 
  *  delayMicrosecondsAdjustedForMillisInterrupt() - delay for a number of microseconds
  *       This function produces a microseconds delay and compensates for
  *       the time taken in the millis interrupt which may fire one or more times
  *       during the delay period.  If you call this function with interrupts
  *       disabled, then the timing will be wrong. The compensation is by no means
  *       perfect but it's pretty close.
  * 
  *  REAL_MILLIS(x) - for x milliseconds in the real world, return the number of millis() that pass in the microcontroller
  *  REAL_MICROS(x) - for x microseconds in the real world, return the number of micros() that pass in the microcontroller
  * 
  *  ACCEPTABLE_MILLIS_ERROR_PPM - the error expressed in PPM which is acceptable in a return from millis()
  *                              - NB: The error will always be on the "slow" side, real world is ahead.
  * 
  *  ACCEPTABLE_MICROS_ERROR_PPM - the error expressed in PPM which is acceptable in a return from micros()
  *                              - NB: The error will always be on the "slow" side, real world is ahead.
  * 
  *  Usage of REAL_MICROS() and REAL_MILLIS()
  *  ----------------------------------------------------------------------
  *
  *  Quite often you will have some code that looks like this...
  *   
  *     unsigned long lastTime = millis();
  *     while(millis() - lastTime < 1000);
  *
  *  which in a perfect world would get you 1000ms of delay but the world 
  *  is not perfect and there are errors induced, even if you configure
  *  (as below) for a high accuracy, there is almost always some error
  *  due to rounding (of course this assumes a perfect clock signal too..)
  *
  *  To help with this two macros are available, for example...
  *
  *     unsigned long lastTime = millis();
  *     while(millis() - lastTime < REAL_MILLIS(1000));
  *
  *  (and similar for REAL_MICROS()) which will return the number of millis()
  *  which corresponds to that many millis() in the real world.  For example
  *  if for your frequency and accuracy configuration 990 millis() pass for each
  *  real world millisecond, then REAL_MILLIS(1000) would return 1010 so you will
  *  wait for 1010 millis() which is 1000 real world milliseconds.
  *
  *  Configuring Your Timing Accuracy vs Code Size Trade Off
  *  ----------------------------------------------------------------------
  *
  *  The accuracy of the millis() values can be increased/decreased 
  *  with more accuracy requiring more code (flash usage)
  *
  *  Accuracy is defined as an error level, you can set the acceptable 
  *  error for both millis() and micros(), expressed in parts per million 
  *  (because floats are not supported in preprocessor macros).
  *
  *  The functions will never give a result with error worse than 5% out
  *  (50000 ppm).
  *
  *  Anyway, long story short, set ACCEPTABLE_MILLIS_ERROR_PPM 
  *   and ACCEPTABLE_MICROS_ERROR_PPM to the maximum error you can tolerate
  *   expressed in parts per million from 0 to 1000000.
  *
  *    Example 1: You are OK with losing 1 second every hour
  *      1/3600 = 0.0002777' fraction
  *             * 100
  *             = 0.0278     percent
  *             * 10000
  *             = 278        ppm
  *      #define ACCEPTABLE_MILLIS_ERROR_PPM 278
  *
  *   Example 2: You are ok with losing 5 minutes a day
  *      5/(60 mins * 24 hours) = 0.00347 fraction
  *                             * 100
  *                             = 0.3472  percent
  *                             * 10000
  *                             = 3472    ppm
  *     #define ACCEPTABLE_MILLIS_ERROR_PPM 3472
  *
  *   Example 3: You are ok with losing 10 microseconds for every 1000 microseconds
  *      10/1000 = 0.01   fraction
  *              * 100  
  *              = 1      percent
  *              * 10000
  *              = 10000  ppm
  *    #define ACCEPTABLE_MICROS_ERROR_PPM 10000
  *
  *  Aside: Of course youc an go straight from your fraction to ppm by multiplying by 
  *  a million, but most people just instinctivly understand percent rather than ppm.
  *  Blame the C preprocessor for your mental gymnastics because if it supported
  *  floats in it's expressions I'd have used percent!
  *
  *  Generally speaking a lower acceptable error will require more flash memory
  *  due to extra calculations required.
  *  
  *  Not all frequencies will show a differenceat all however
  *    for example 16kHz and 128kHz are exact and do not exhibit any error.
  *
  *  Finally note of course that this error is under the assumption that you have
  *  an ideal clock source for your AVR... which if you are using the internal
  *  oscillator is very far from the truth.  If your clock wanders, your accuracy
  *  wanders with it.  The internal oscillator wanders.
*/

typedef uint32_t MillisMicrosTime_t;
typedef uint16_t DelayTime_t             ;
typedef uint16_t DelayMicrosecondsTime_t;

// NB DelayMicrosecondsTime_t can't be less than uint16_t.  
//    Firstly because millis() will use delayMicroseconds(1000) if NO_MILLIS
//    is defined.
//
//    Secondly because it just doesn't give enough realistic resolution.
//
// Example:
//    9.6MHz, consult the delayMicroseconds() code and you will find the multiplier  
//    of 2.4 loops per us, at 16bit, 2.4 * 27306us = 65535 loop, 2.4 * 106 = 254 loop
//    so an 8 bit DelayMicrosecondsTime_t (which is also used as the loop counter)
//    would allow only allow 106uS maximum input
//
//   For frequencies <= 4 MHz we get to use the full width of DelayMicrosecondsTime_t
//   as the us input, so
//   2.4Mhz, 16bit: 65535 us maximum delayMicroseconds(), 8 bit: 255us 
//
//   For frequencies < 260kHz however the overhead in delayMicroseconds() is more than
//   255us itself, and if you try to request less than the overhead, you get nothing.
//
//  So realistically there is the limited band 260kHz <= x <= 4Mhz where you might want
//  to use uint8_t here, if you are OK with only being able to delayMicroseconds(255) 
//  at a maximum and you have millis() available for delay() to use (or don't use delay()).

void delay(DelayTime_t);
void delayMicrosecondsWithoutMillisInterruptAdjustment(DelayMicrosecondsTime_t);
void delayMicrosecondsAdjustedForMillisInterrupt(DelayMicrosecondsTime_t);

#ifdef NO_MILLIS
  // If the millis interrupt handler isn't being used at all
  // then we do not need to adjust for it in delayMicroseconds
  #define delayMicroseconds(...) delayMicrosecondsWithoutMillisInterruptAdjustment(__VA_ARGS__)
#else
  #if !defined(LTO_ENABLED)    
    // Conversely if we do have millis() running then we are probably
    // best to use our adjusted one instead of the normal one
    #define delayMicroseconds(...) delayMicrosecondsAdjustedForMillisInterrupt(__VA_ARGS__)
  #else
    // Except if LTO is enabled, in which case the clock counts for that adjustment 
    // are probably invalid, unfortunately there does not presently appear to be 
    // a way to disable LTO for that specific function, it would require having different
    // compilation options for MillisMicrosDelay.c specifically which is not possible
    // (I think)    
    #define delayMicroseconds(...) delayMicrosecondsWithoutMillisInterruptAdjustment(__VA_ARGS__)
  #endif
#endif

#ifndef NO_MILLIS
MillisMicrosTime_t millis();
MillisMicrosTime_t micros();
  
#ifndef ACCEPTABLE_MILLIS_ERROR_PPM
  #ifdef ACCEPTABLE_MICROS_ERROR_PPM
    #define ACCEPTABLE_MILLIS_ERROR_PPM ACCEPTABLE_MICROS_ERROR_PPM 
  #else
  //#define ACCEPTABLE_MILLIS_ERROR_PPM 1
  #define ACCEPTABLE_MILLIS_ERROR_PPM 1000000
  #endif
#endif

#ifndef ACCEPTABLE_MICROS_ERROR_PPM
  #define ACCEPTABLE_MICROS_ERROR_PPM ACCEPTABLE_MILLIS_ERROR_PPM
#endif
  
// The prescaler which is being used on the overflow vector, you CAN set this in
// pins_arduino.h if your specific chip can't handle 1/8/64 prescaling, otherwise
// just leave it as the default's below which should work.
//
// The prescale is passed into turnOnMillis() for you to use when setting 
// up your timer.
#ifndef MILLIS_TIMER_PRESCALE
  #if   F_CPU >  16000000UL
    // Down to 16MHz /64 gets at worst ~1ms per tick
    #define MILLIS_TIMER_PRESCALE 64  
  #elif F_CPU >   2000000UL
    // Down to 2MHz /8 Prescale gets at worst ~1ms per tick
    // Best = 128us at 16MHz    
    #define MILLIS_TIMER_PRESCALE 8
  #else
    // 2Mhz or below we really need to run without prescaling
    // Best  @ 2Mhz, 128us per tick
    // Worst @ 16kHz, 16000us (16ms) per tick
    #define MILLIS_TIMER_PRESCALE 1
  #endif
#endif    

  // REAL_MILLIS(x)
#if (F_CPU / MILLIS_TIMER_PRESCALE) >= 24000000UL
  // 24 MHz
    //  Error: 0.0146% (0.0001455417 Decimal)
    // Jitter: 0.0372% (0.0003718387 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000146))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999854ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999854ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999854ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 20000000UL
  // 20 MHz
    //  Error: 0.1080% (0.0010804776999999998 Decimal)
    // Jitter: 0.0367% (0.0003666667 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.00108))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 998920ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 998920ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 998920ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 19200000UL
  // 19.2 MHz
    //  Error: 0.0992% (0.0009916172 Decimal)
    // Jitter: 0.0329% (0.0003291789 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000992))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999008ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999008ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999008ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 16000000UL
  // 16 MHz
    //  Error: 0.0580% (0.0005804791 Decimal)
    // Jitter: 0.0177% (0.00017671090000000002 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.00058))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999420ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999420ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999420ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 12000000UL
  // 12 MHz
    //  Error: 0.0146% (0.0001455417 Decimal)
    // Jitter: 0.0372% (0.0003718387 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000146))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999854ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999854ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999854ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 9600000UL
  // 9.6 MHz
    //  Error: 0.0429% (0.0004293574 Decimal)
    // Jitter: 0.0377% (0.0003769841 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000429))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999571ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999571ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999571ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 8000000UL
  // 8 MHz
    //  Error: 0.0114% (0.0001138951 Decimal)
    // Jitter: 0.0224% (0.0002238095 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000114))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999886ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999886ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999886ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 6400000UL
  // 6.4 MHz
    //  Error: 0.0237% (0.0002371862 Decimal)
    // Jitter: 0.0301% (0.0003012704 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000237))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999763ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999763ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999763ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 4800000UL
  // 4.8 MHz
    //  Error: 0.0153% (0.00015272219999999998 Decimal)
    // Jitter: 0.0413% (0.0004126984 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000153))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999847ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999847ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999847ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 4000000UL
  // 4 MHz
    //  Error: 0.0114% (0.0001138951 Decimal)
    // Jitter: 0.0224% (0.0002238095 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000114))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999886ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999886ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999886ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 3000000UL
  // 3 MHz
    //  Error: 0.0145% (0.0001452576 Decimal)
    // Jitter: 0.0369% (0.0003692755 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000145))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999855ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999855ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999855ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2500000UL
  // 2.5 MHz
    //  Error: 0.0205% (0.0002048921 Decimal)
    // Jitter: 0.0556% (0.0005560046 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000205))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999795ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999795ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999795ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2400000UL
  // 2.4 MHz
    //  Error: 0.0153% (0.00015272219999999998 Decimal)
    // Jitter: 0.0413% (0.0004126984 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000153))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999847ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999847ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999847ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2000000UL
  // 2 MHz
    //  Error: 0.0113% (0.0001126104 Decimal)
    // Jitter: 0.0224% (0.0002238139 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000113))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999887ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999887ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999887ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1500000UL
  // 1.5 MHz
    //  Error: 0.0141% (0.0001406642 Decimal)
    // Jitter: 0.0375% (0.0003745527 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000141))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999859ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999859ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999859ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1200000UL
  // 1.2 MHz
    //  Error: 0.0096% (0.0000955207 Decimal)
    // Jitter: 0.0480% (0.0004795937 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000096))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999904ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999904ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999904ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1000000UL
  // 1 MHz
    //  Error: 0.0063% (0.0000628935 Decimal)
    // Jitter: 0.0278% (0.0002777817 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000063))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999937ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999937ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999937ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 800000UL
  // 800 kHz
    //  Error: 0.0115% (0.0001145359 Decimal)
    // Jitter: 0.0387% (0.0003865699 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000115))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999885ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999885ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999885ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 600000UL
  // 600 kHz
    //  Error: 0.0091% (0.0000913535 Decimal)
    // Jitter: 0.0480% (0.0004795937 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000091))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999909ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999909ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999909ULL )/1000000ULL   )                \
    ))                                                                            \

    #elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 500000UL
  // 500 kHz
    //  Error: 0.0066% (0.0000656635 Decimal)
    // Jitter: 0.0278% (0.0002779503 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000066))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999934ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999934ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999934ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 375000UL
  // 375 kHz
    //  Error: 0.0092% (0.0000915869 Decimal)
    // Jitter: 0.0435% (0.0004345473 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000092))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999908ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999908ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999908ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 312500UL
  // 312.5 kHz
    //  Error: 0.0098% (0.000098003 Decimal)
    // Jitter: 0.0678% (0.0006777827 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000098))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999902ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999902ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999902ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 300000UL
  // 300 kHz
    //  Error: 0.0082% (0.00008233490000000001 Decimal)
    // Jitter: 0.0485% (0.0004849638 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000082))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999918ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999918ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999918ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 250000UL
  // 250 kHz
    //  Error: 0.0055% (0.0000546916 Decimal)
    // Jitter: 0.0255% (0.0002551896 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000055))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999945ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999945ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999945ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 187500UL
  // 187.5 kHz
    //  Error: 0.0085% (0.0000847758 Decimal)
    // Jitter: 0.0385% (0.0003851959 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000085))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999915ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999915ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999915ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 150000UL
  // 150 kHz
  //     Best Error Possible: 0.0082%  (0.00008233490000000001 Decimal)
    //  Error: 0.0082% (0.00008233490000000001 Decimal)
    // Jitter: 0.0485% (0.0004849638 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000082))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999918ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999918ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999918ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 128000UL
  // 128 kHz
    //  Error: 0.0055% (0.0000545291 Decimal)
    // Jitter: 0.0258% (0.00025814839999999996 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000055))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999945ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999945ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999945ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 100000UL
  // 100 kHz
    //  Error: 0.0087% (0.00008692650000000001 Decimal)
    // Jitter: 0.0373% (0.0003726614 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000087))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999913ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999913ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999913ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 75000UL
  // 75 kHz
    //  Error: 0.0068% (0.00006818429999999999 Decimal)
    // Jitter: 0.0407% (0.00040653280000000003 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000068))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999932ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999932ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999932ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 62500UL
  // 62.5 kHz
  //     Best Error Possible: 0.0053%  (0.000053184600000000006 Decimal)
    //  Error: 0.0053% (0.000053184600000000006 Decimal)
    // Jitter: 0.0258% (0.0002582281 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000053))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999947ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999947ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999947ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 37500UL
  // 37.5 kHz
    //  Error: 0.0064% (0.0000640909 Decimal)
    // Jitter: 0.0364% (0.0003637025 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000064))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999936ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999936ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999936ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 32768UL
  // 32.768 kHz
    //  Error: 0.0000% (0 Decimal)
    // Jitter: 0.0000% (0 Decimal)

    #define REAL_MILLIS(AVR_MILLIS) ((uint32_t) (AVR_MILLIS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 31250UL
  // 31.25 kHz
    //  Error: 0.0052% (0.0000522177 Decimal)
    // Jitter: 0.0258% (0.0002581808 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000052))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999948ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999948ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999948ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 18750UL
  // 18.75 kHz
    //  Error: 0.0050% (0.000049767899999999995 Decimal)
    // Jitter: 0.0300% (0.00030018009999999997 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.00005))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999950ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999950ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999950ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 16000UL
  // 16 kHz
    //  Error: 0.0046% (0.0000461119 Decimal)
    // Jitter: 0.0242% (0.0002421586 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000046))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999954ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999954ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999954ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 12500UL
  // 12.5 kHz
    //  Error: 0.0078% (0.0000782744 Decimal)
    // Jitter: 0.0369% (0.00036935969999999995 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000078))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999922ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999922ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999922ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 9375UL
  // 9.375 kHz
    //  Error: 0.0050% (0.000049767899999999995 Decimal)
    // Jitter: 0.0300% (0.00030018009999999997 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.00005))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999950ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999950ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999950ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 4096UL
  // 4.096 kHz
    //  Error: 0.0000% (0 Decimal)
    // Jitter: 0.0000% (0 Decimal)

    #define REAL_MILLIS(AVR_MILLIS) ((uint32_t) (AVR_MILLIS))

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2000UL
  // 2 kHz
    //  Error: 0.0031% (0.0000310192 Decimal)
    // Jitter: 0.0141% (0.0001412729 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000031))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999969ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999969ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999969ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 512UL
  // 0.512 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MILLIS(AVR_MILLIS) ((uint32_t) (AVR_MILLIS))
#endif

  // REAL_MICROS(x)
#if (F_CPU / MILLIS_TIMER_PRESCALE) >= 24000000UL
  // 24 MHz
    //  Error: 0.0074% (0.0000739307 Decimal)
    // Jitter: 0.0315% (0.00031501690000000003 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000074))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999926ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999926ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999926ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 20000000UL
  // 20 MHz
    //  Error: 0.0068% (0.000067535 Decimal)
    // Jitter: 0.0318% (0.0003179226 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000068))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999932ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999932ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999932ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 19200000UL
  // 19.2 MHz
    //  Error: 0.0065% (0.0000651144 Decimal)
    // Jitter: 0.0384% (0.000384375 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000065))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999935ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999935ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999935ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 16000000UL
  // 16 MHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 12000000UL
  // 12 MHz
    //  Error: 0.0060% (0.0000600665 Decimal)
    // Jitter: 0.0258% (0.00025756650000000004 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.00006))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999940ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999940ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999940ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 9600000UL
  // 9.6 MHz
    //  Error: 0.0068% (0.0000677557 Decimal)
    // Jitter: 0.0384% (0.0003841938 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000068))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999932ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999932ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999932ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 8000000UL
  // 8 MHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 6400000UL
  // 6.4 MHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 4800000UL
  // 4.8 MHz
    //  Error: 0.0054% (0.0000540151 Decimal)
    // Jitter: 0.0292% (0.0002917454 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000054))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999946ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999946ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999946ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 4000000UL
  // 4 MHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 3000000UL
  // 3 MHz
    //  Error: 0.0046% (0.0000462079 Decimal)
    // Jitter: 0.0191% (0.0001911724 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000046))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999954ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999954ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999954ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2500000UL
  // 2.5 MHz
    //  Error: 0.0059% (0.0000587247 Decimal)
    // Jitter: 0.0293% (0.0002925152 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000059))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999941ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999941ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999941ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2400000UL
  // 2.4 MHz
    //  Error: 0.0056% (0.0000559561 Decimal)
    // Jitter: 0.0288% (0.00028813900000000003 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000056))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999944ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999944ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999944ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2000000UL
  // 2 MHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1500000UL
  // 1.5 MHz
    //  Error: 0.0046% (0.0000463401 Decimal)
    // Jitter: 0.0193% (0.0001928854 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000046))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999954ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999954ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999954ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1200000UL
  // 1.2 MHz
    //  Error: 0.0042% (0.0000424244 Decimal)
    // Jitter: 0.0204% (0.0002038113 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000042))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999958ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999958ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999958ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1000000UL
  // 1 MHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 800000UL
  // 800 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 600000UL
  // 600 kHz
    //  Error: 0.0030% (0.0000295508 Decimal)
    // Jitter: 0.0204% (0.0002038113 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.00003))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999970ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999970ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999970ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 500000UL
  // 500 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 375000UL
  // 375 kHz
    //  Error: 0.0026% (0.0000256368 Decimal)
    // Jitter: 0.0133% (0.0001331647 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000026))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999974ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999974ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999974ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 312500UL
  // 312.5 kHz
    //  Error: 0.0024% (0.0000237285 Decimal)
    // Jitter: 0.0174% (0.00017438310000000002 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000024))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999976ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999976ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999976ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 300000UL
  // 300 kHz
    //  Error: 0.0028% (0.0000281963 Decimal)
    // Jitter: 0.0209% (0.00020863839999999998 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000028))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999972ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999972ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999972ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 250000UL
  // 250 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 187500UL
  // 187.5 kHz
    //  Error: 0.0026% (0.0000256368 Decimal)
    // Jitter: 0.0133% (0.0001331647 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000026))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999974ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999974ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999974ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 150000UL
  // 150 kHz
    //  Error: 0.0023% (0.000023453 Decimal)
    // Jitter: 0.0167% (0.00016740600000000002 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000023))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999977ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999977ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999977ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 128000UL
  // 128 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 125000UL
  // 125 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 100000UL
  // 100 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 75000UL
  // 75 kHz
    //  Error: 0.0013% (0.00001288 Decimal)
    // Jitter: 0.0146% (0.00014647720000000001 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000013))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999987ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999987ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999987ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 62500UL
  // 62.5 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 37500UL
  // 37.5 kHz
    //  Error: 0.0008% (0.0000076401 Decimal)
    // Jitter: 0.0146% (0.00014647720000000001 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000008))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999992ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999992ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999992ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 32768UL
  // 32.768 kHz
  //     Best Error Possible: 0.0003%  (0.0000030611000000000002 Decimal)
    //  Error: 0.0003% (0.0000030611000000000002 Decimal)
    // Jitter: 0.0128% (0.0001279918 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000003))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999997ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999997ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999997ULL )/1000000ULL   )                \
    ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 31250UL
  // 31.25 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 18750UL
  // 18.75 kHz
  //  Error: 0.0003% (0.0000032111 Decimal)
  // Jitter: 0.0037% (0.000036620600000000006 Decimal)

  // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000003))
  #define REAL_MICROS(AVR_MICROS)                                               \
  ((uint32_t)(                                                                  \
     (   ( ((uint64_t)(AVR_MICROS)) * 999997ULL )%1000000ULL >= 500000ULL )     \
     ? ((( ((uint64_t)(AVR_MICROS)) * 999997ULL )/1000000ULL)+1)                \
     : ( ( ((uint64_t)(AVR_MICROS)) * 999997ULL )/1000000ULL   )                \
  ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 16000UL
  // 16 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 15625UL
  // 15.625 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 12500UL
  // 12.5 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 9375UL
  // 9.375 kHz
  //  Error: 0.0002% (0.00000191 Decimal)
  // Jitter: 0.0037% (0.000036620600000000006 Decimal)

  // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000002))
  #define REAL_MICROS(AVR_MICROS)                                               \
  ((uint32_t)(                                                                  \
     (   ( ((uint64_t)(AVR_MICROS)) * 999998ULL )%1000000ULL >= 500000ULL )     \
     ? ((( ((uint64_t)(AVR_MICROS)) * 999998ULL )/1000000ULL)+1)                \
     : ( ( ((uint64_t)(AVR_MICROS)) * 999998ULL )/1000000ULL   )                \
  ))                                                                            \

#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 4096UL
  // 4.096 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2000UL
  // 2 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1953UL
  // 1.953 kHz
  //  Error: 0.0000% (3.3800000000000004e-7 Decimal)
  // Jitter: 0.0004% (0.0000038144 Decimal)

  // The below define is equivalent to ROUND(AVR_MICROS * (1-0))
  #define REAL_MICROS(AVR_MICROS)                                                \
  ((uint32_t)(                                                                   \
     (   ( ((uint64_t)(AVR_MICROS)) * 1000000ULL )%1000000ULL >= 500000ULL )     \
     ? ((( ((uint64_t)(AVR_MICROS)) * 1000000ULL )/1000000ULL)+1)                \
     : ( ( ((uint64_t)(AVR_MICROS)) * 1000000ULL )/1000000ULL   )                \
  ))                                                                             \


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 512UL
  // 0.512 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))
#endif

#endif

#endif
#endif