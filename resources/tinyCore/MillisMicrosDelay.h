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
  //     Best Error Possible: 0.0146%  (0.0001455417 Decimal)
  //    Worst Error Possible: 26.7588% (0.267588149 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 267588UL
    //  Error: 26.7588% (0.267588149 Decimal)
    // Jitter: 0.0065% (0.0000647321 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.267588))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 732412ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 732412ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 732412ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 84492UL
    //  Error: 8.4492% (0.0844921373 Decimal)
    // Jitter: 0.0098% (0.0000977444 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.084492))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 915508ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 915508ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 915508ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 38726UL
    //  Error: 3.8726% (0.038726386 Decimal)
    // Jitter: 0.0160% (0.0001597744 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.038726))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 961274ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 961274ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 961274ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 15849UL
    //  Error: 1.5849% (0.015849382 Decimal)
    // Jitter: 0.0188% (0.0001879699 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.015849))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 984151ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 984151ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 984151ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 4417UL
    //  Error: 0.4417% (0.0044165132000000005 Decimal)
    // Jitter: 0.0268% (0.0002679426 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.004417))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 995583ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 995583ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 995583ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 1566UL
    //  Error: 0.1566% (0.0015659282000000001 Decimal)
    // Jitter: 0.0307% (0.00030690359999999996 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.001566))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 998434ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 998434ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 998434ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0146% (0.0001455417 Decimal)
    // Jitter: 0.0372% (0.0003718387 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000146))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999854ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999854ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999854ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 20000000UL
  // 20 MHz
  //     Best Error Possible: 0.1080%  (0.0010804776999999998 Decimal)
  //    Worst Error Possible: 38.9659% (0.38965908730000004 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 389659UL
    //  Error: 38.9659% (0.38965908730000004 Decimal)
    // Jitter: 0.0079% (0.0000788352 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.389659))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 610341ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 610341ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 610341ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 84494UL
    //  Error: 8.4494% (0.0844941297 Decimal)
    // Jitter: 0.0163% (0.00016339070000000001 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.084494))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 915506ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 915506ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 915506ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8211UL
    //  Error: 0.8211% (0.00821125 Decimal)
    // Jitter: 0.0218% (0.0002175182 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.008211))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 991789ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 991789ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 991789ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 3454UL
    //  Error: 0.3454% (0.0034536073999999997 Decimal)
    // Jitter: 0.0284% (0.0002839416 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.003454))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 996546ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 996546ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 996546ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.1080% (0.0010804776999999998 Decimal)
    // Jitter: 0.0367% (0.0003666667 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.00108))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 998920ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 998920ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 998920ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 19200000UL
  // 19.2 MHz
  //     Best Error Possible: 0.0992%  (0.0009916172 Decimal)
  //    Worst Error Possible: 41.4072% (0.4140718057 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 414072UL
    //  Error: 41.4072% (0.4140718057 Decimal)
    // Jitter: 0.0055% (0.0000551471 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.414072))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 585928ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 585928ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 585928ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 121113UL
    //  Error: 12.1113% (0.1211131569 Decimal)
    // Jitter: 0.0106% (0.00010625 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.121113))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 878887ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 878887ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 878887ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 47881UL
    //  Error: 4.7881% (0.0478814326 Decimal)
    // Jitter: 0.0148% (0.0001484375 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.047881))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 952119ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 952119ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 952119ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11271UL
    //  Error: 1.1271% (0.011270545 Decimal)
    // Jitter: 0.0223% (0.0002229665 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.011271))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 988729ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 988729ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 988729ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2126UL
    //  Error: 0.2126% (0.0021257332 Decimal)
    // Jitter: 0.0283% (0.0002829912 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.002126))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 997874ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 997874ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 997874ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0992% (0.0009916172 Decimal)
    // Jitter: 0.0329% (0.0003291789 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000992))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999008ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999008ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999008ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 16000000UL
  // 16 MHz
  //     Best Error Possible: 0.0580%  (0.0005804791 Decimal)
  //    Worst Error Possible: 2.3447% (0.0234473254 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23447UL
    //  Error: 2.3447% (0.0234473254 Decimal)
    // Jitter: 0.0062% (0.0000625 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.023447))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 976553ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 976553ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 976553ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8199UL
    //  Error: 0.8199% (0.008199243200000001 Decimal)
    // Jitter: 0.0120% (0.0001203125 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.008199))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 991801ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 991801ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 991801ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0580% (0.0005804791 Decimal)
    // Jitter: 0.0177% (0.00017671090000000002 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.00058))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999420ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999420ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999420ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 12000000UL
  // 12 MHz
  //     Best Error Possible: 0.0146%  (0.0001455417 Decimal)
  //    Worst Error Possible: 26.7588% (0.267588149 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 267588UL
    //  Error: 26.7588% (0.267588149 Decimal)
    // Jitter: 0.0065% (0.0000647321 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.267588))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 732412ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 732412ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 732412ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 84492UL
    //  Error: 8.4492% (0.0844921373 Decimal)
    // Jitter: 0.0098% (0.0000977444 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.084492))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 915508ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 915508ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 915508ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 38726UL
    //  Error: 3.8726% (0.038726386 Decimal)
    // Jitter: 0.0160% (0.0001597744 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.038726))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 961274ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 961274ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 961274ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 15849UL
    //  Error: 1.5849% (0.015849382 Decimal)
    // Jitter: 0.0188% (0.0001879699 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.015849))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 984151ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 984151ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 984151ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 4417UL
    //  Error: 0.4417% (0.0044165132000000005 Decimal)
    // Jitter: 0.0268% (0.0002679426 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.004417))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 995583ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 995583ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 995583ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 1566UL
    //  Error: 0.1566% (0.0015659282000000001 Decimal)
    // Jitter: 0.0307% (0.00030690359999999996 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.001566))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 998434ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 998434ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 998434ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0146% (0.0001455417 Decimal)
    // Jitter: 0.0372% (0.0003718387 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000146))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999854ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999854ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999854ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 9600000UL
  // 9.6 MHz
  //     Best Error Possible: 0.0429%  (0.0004293574 Decimal)
  //    Worst Error Possible: 41.4072% (0.4140718057 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 414072UL
    //  Error: 41.4072% (0.4140718057 Decimal)
    // Jitter: 0.0055% (0.0000551471 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.414072))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 585928ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 585928ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 585928ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 121113UL
    //  Error: 12.1113% (0.1211131569 Decimal)
    // Jitter: 0.0106% (0.00010625 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.121113))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 878887ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 878887ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 878887ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 47881UL
    //  Error: 4.7881% (0.0478814326 Decimal)
    // Jitter: 0.0148% (0.0001484375 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.047881))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 952119ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 952119ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 952119ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11271UL
    //  Error: 1.1271% (0.011270545 Decimal)
    // Jitter: 0.0223% (0.0002229665 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.011271))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 988729ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 988729ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 988729ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2126UL
    //  Error: 0.2126% (0.0021257332 Decimal)
    // Jitter: 0.0283% (0.0002829912 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.002126))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 997874ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 997874ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 997874ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 992UL
    //  Error: 0.0992% (0.0009916172 Decimal)
    // Jitter: 0.0329% (0.0003291789 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000992))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999008ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999008ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999008ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0429% (0.0004293574 Decimal)
    // Jitter: 0.0377% (0.0003769841 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000429))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999571ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999571ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999571ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 8000000UL
  // 8 MHz
  //     Best Error Possible: 0.0114%  (0.0001138951 Decimal)
  //    Worst Error Possible: 2.3447% (0.0234473254 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23447UL
    //  Error: 2.3447% (0.0234473254 Decimal)
    // Jitter: 0.0062% (0.0000625 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.023447))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 976553ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 976553ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 976553ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8199UL
    //  Error: 0.8199% (0.008199243200000001 Decimal)
    // Jitter: 0.0120% (0.0001203125 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.008199))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 991801ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 991801ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 991801ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 580UL
    //  Error: 0.0580% (0.0005804791 Decimal)
    // Jitter: 0.0177% (0.00017671090000000002 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.00058))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999420ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999420ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999420ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0114% (0.0001138951 Decimal)
    // Jitter: 0.0224% (0.0002238095 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000114))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999886ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999886ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999886ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 6400000UL
  // 6.4 MHz
  //     Best Error Possible: 0.0237%  (0.0002371862 Decimal)
  //    Worst Error Possible: 21.8758% (0.21875796979999998 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 218758UL
    //  Error: 21.8758% (0.21875796979999998 Decimal)
    // Jitter: 0.0068% (0.00006818179999999999 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.218758))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 781242ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 781242ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 781242ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 23455UL
    //  Error: 2.3455% (0.023454928400000002 Decimal)
    // Jitter: 0.0108% (0.0001079545 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.023455))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 976545ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 976545ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 976545ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11258UL
    //  Error: 1.1258% (0.0112578935 Decimal)
    // Jitter: 0.0136% (0.0001362782 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.011258))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 988742ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 988742ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 988742ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 5164UL
    //  Error: 0.5164% (0.0051639588 Decimal)
    // Jitter: 0.0186% (0.00018647909999999998 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.005164))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 994836ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 994836ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 994836ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2123UL
    //  Error: 0.2123% (0.0021232221 Decimal)
    // Jitter: 0.0238% (0.0002382033 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.002123))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 997877ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 997877ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 997877ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 608UL
    //  Error: 0.0608% (0.0006080995 Decimal)
    // Jitter: 0.0290% (0.0002903811 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000608))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999392ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999392ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999392ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0237% (0.0002371862 Decimal)
    // Jitter: 0.0301% (0.0003012704 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000237))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999763ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999763ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999763ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 4800000UL
  // 4.8 MHz
  //     Best Error Possible: 0.0153%  (0.00015272219999999998 Decimal)
  //    Worst Error Possible: 41.4072% (0.4140718057 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 414072UL
    //  Error: 41.4072% (0.4140718057 Decimal)
    // Jitter: 0.0055% (0.0000551471 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.414072))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 585928ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 585928ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 585928ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 121113UL
    //  Error: 12.1113% (0.1211131569 Decimal)
    // Jitter: 0.0106% (0.00010625 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.121113))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 878887ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 878887ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 878887ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 47881UL
    //  Error: 4.7881% (0.0478814326 Decimal)
    // Jitter: 0.0148% (0.0001484375 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.047881))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 952119ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 952119ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 952119ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11271UL
    //  Error: 1.1271% (0.011270545 Decimal)
    // Jitter: 0.0223% (0.0002229665 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.011271))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 988729ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 988729ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 988729ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2126UL
    //  Error: 0.2126% (0.0021257332 Decimal)
    // Jitter: 0.0283% (0.0002829912 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.002126))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 997874ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 997874ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 997874ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 992UL
    //  Error: 0.0992% (0.0009916172 Decimal)
    // Jitter: 0.0329% (0.0003291789 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000992))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999008ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999008ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999008ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 429UL
    //  Error: 0.0429% (0.0004293574 Decimal)
    // Jitter: 0.0377% (0.0003769841 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000429))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999571ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999571ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999571ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0153% (0.00015272219999999998 Decimal)
    // Jitter: 0.0413% (0.0004126984 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000153))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999847ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999847ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999847ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 4000000UL
  // 4 MHz
  //     Best Error Possible: 0.0114%  (0.0001138951 Decimal)
  //    Worst Error Possible: 2.3447% (0.0234473254 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23447UL
    //  Error: 2.3447% (0.0234473254 Decimal)
    // Jitter: 0.0062% (0.0000625 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.023447))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 976553ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 976553ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 976553ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8199UL
    //  Error: 0.8199% (0.008199243200000001 Decimal)
    // Jitter: 0.0120% (0.0001203125 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.008199))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 991801ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 991801ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 991801ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 580UL
    //  Error: 0.0580% (0.0005804791 Decimal)
    // Jitter: 0.0177% (0.00017671090000000002 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.00058))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999420ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999420ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999420ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0114% (0.0001138951 Decimal)
    // Jitter: 0.0224% (0.0002238095 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000114))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999886ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999886ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999886ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 3000000UL
  // 3 MHz
  //     Best Error Possible: 0.0145%  (0.0001452576 Decimal)
  //    Worst Error Possible: 26.7587% (0.2675870743 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 267587UL
    //  Error: 26.7587% (0.2675870743 Decimal)
    // Jitter: 0.0071% (0.00007142860000000001 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.267587))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 732413ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 732413ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 732413ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 84491UL
    //  Error: 8.4491% (0.0844910299 Decimal)
    // Jitter: 0.0107% (0.0001071429 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.084491))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 915509ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 915509ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 915509ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 38726UL
    //  Error: 3.8726% (0.0387255216 Decimal)
    // Jitter: 0.0160% (0.0001602836 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.038726))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 961274ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 961274ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 961274ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 15849UL
    //  Error: 1.5849% (0.0158488616 Decimal)
    // Jitter: 0.0186% (0.0001862917 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.015849))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 984151ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 984151ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 984151ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 4416UL
    //  Error: 0.4416% (0.0044161647 Decimal)
    // Jitter: 0.0265% (0.00026546640000000004 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.004416))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 995584ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 995584ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 995584ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 1566UL
    //  Error: 0.1566% (0.0015656226999999998 Decimal)
    // Jitter: 0.0307% (0.0003065773 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.001566))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 998434ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 998434ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 998434ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0145% (0.0001452576 Decimal)
    // Jitter: 0.0369% (0.0003692755 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000145))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999855ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999855ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999855ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2500000UL
  // 2.5 MHz
  //     Best Error Possible: 0.0205%  (0.0002048921 Decimal)
  //    Worst Error Possible: 38.9657% (0.3896574804 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 389657UL
    //  Error: 38.9657% (0.3896574804 Decimal)
    // Jitter: 0.0091% (0.0000909091 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.389657))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 610343ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 610343ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 610343ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 84492UL
    //  Error: 8.4492% (0.0844916438 Decimal)
    // Jitter: 0.0182% (0.0001818182 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.084492))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 915508ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 915508ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 915508ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8208UL
    //  Error: 0.8208% (0.008208097599999999 Decimal)
    // Jitter: 0.0224% (0.0002235294 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.008208))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 991792ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 991792ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 991792ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 3449UL
    //  Error: 0.3450% (0.0034504404 Decimal)
    // Jitter: 0.0288% (0.0002882353 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.00345))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 996550ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 996550ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 996550ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 1077UL
    //  Error: 0.1077% (0.0010773807 Decimal)
    // Jitter: 0.0368% (0.00036803300000000003 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.001077))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 998923ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 998923ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 998923ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 492UL
    //  Error: 0.0492% (0.0004921641 Decimal)
    // Jitter: 0.0460% (0.0004600141 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000492))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999508ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999508ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999508ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0205% (0.0002048921 Decimal)
    // Jitter: 0.0556% (0.0005560046 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000205))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999795ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999795ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999795ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2400000UL
  // 2.4 MHz
  //     Best Error Possible: 0.0153%  (0.00015272219999999998 Decimal)
  //    Worst Error Possible: 41.4072% (0.4140718057 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 414072UL
    //  Error: 41.4072% (0.4140718057 Decimal)
    // Jitter: 0.0055% (0.0000551471 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.414072))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 585928ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 585928ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 585928ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 121113UL
    //  Error: 12.1113% (0.1211131569 Decimal)
    // Jitter: 0.0106% (0.00010625 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.121113))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 878887ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 878887ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 878887ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 47881UL
    //  Error: 4.7881% (0.0478814326 Decimal)
    // Jitter: 0.0148% (0.0001484375 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.047881))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 952119ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 952119ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 952119ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11271UL
    //  Error: 1.1271% (0.011270545 Decimal)
    // Jitter: 0.0223% (0.0002229665 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.011271))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 988729ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 988729ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 988729ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2126UL
    //  Error: 0.2126% (0.0021257332 Decimal)
    // Jitter: 0.0283% (0.0002829912 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.002126))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 997874ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 997874ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 997874ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 992UL
    //  Error: 0.0992% (0.0009916172 Decimal)
    // Jitter: 0.0329% (0.0003291789 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000992))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999008ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999008ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999008ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 429UL
    //  Error: 0.0429% (0.0004293574 Decimal)
    // Jitter: 0.0377% (0.0003769841 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000429))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999571ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999571ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999571ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0153% (0.00015272219999999998 Decimal)
    // Jitter: 0.0413% (0.0004126984 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000153))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999847ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999847ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999847ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2000000UL
  // 2 MHz
  //     Best Error Possible: 0.0113%  (0.0001126104 Decimal)
  //    Worst Error Possible: 2.3446% (0.0234464846 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23445UL
    //  Error: 2.3446% (0.0234464846 Decimal)
    // Jitter: 0.0088% (0.00008823529999999999 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.023446))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 976554ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 976554ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 976554ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8198UL
    //  Error: 0.8198% (0.008198212299999999 Decimal)
    // Jitter: 0.0124% (0.0001235294 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.008198))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 991802ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 991802ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 991802ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 579UL
    //  Error: 0.0579% (0.0005791708 Decimal)
    // Jitter: 0.0179% (0.00017891220000000002 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000579))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999421ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999421ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999421ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0113% (0.0001126104 Decimal)
    // Jitter: 0.0224% (0.0002238139 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000113))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999887ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999887ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999887ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1500000UL
  // 1.5 MHz
  //     Best Error Possible: 0.0141%  (0.0001406642 Decimal)
  //    Worst Error Possible: 26.7586% (0.2675857411 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 267586UL
    //  Error: 26.7586% (0.2675857411 Decimal)
    // Jitter: 0.0065% (0.00006493510000000001 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.267586))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 732414ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 732414ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 732414ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 84490UL
    //  Error: 8.4490% (0.08448953660000001 Decimal)
    // Jitter: 0.0118% (0.0001176401 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.08449))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 915510ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 915510ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 915510ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 38723UL
    //  Error: 3.8723% (0.038723407700000004 Decimal)
    // Jitter: 0.0125% (0.0001247285 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.038723))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 961277ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 961277ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 961277ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 15846UL
    //  Error: 1.5846% (0.0158457416 Decimal)
    // Jitter: 0.0191% (0.00019142279999999999 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.015846))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 984154ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 984154ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 984154ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 4412UL
    //  Error: 0.4412% (0.0044117933 Decimal)
    // Jitter: 0.0270% (0.0002700851 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.004412))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 995588ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 995588ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 995588ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 1561UL
    //  Error: 0.1561% (0.0015610432 Decimal)
    // Jitter: 0.0309% (0.00030906229999999997 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.001561))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 998439ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 998439ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 998439ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0141% (0.0001406642 Decimal)
    // Jitter: 0.0375% (0.0003745527 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000141))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999859ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999859ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999859ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1200000UL
  // 1.2 MHz
  //     Best Error Possible: 0.0096%  (0.0000955207 Decimal)
  //    Worst Error Possible: 41.4070% (0.4140701323 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 414069UL
    //  Error: 41.4070% (0.4140701323 Decimal)
    // Jitter: 0.0042% (0.0000424882 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.41407))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 585930ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 585930ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 585930ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 121115UL
    //  Error: 12.1116% (0.1211156348 Decimal)
    // Jitter: 0.0101% (0.00010128149999999999 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.121116))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 878884ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 878884ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 878884ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 47885UL
    //  Error: 4.7885% (0.047884874900000006 Decimal)
    // Jitter: 0.0141% (0.0001412517 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.047885))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 952115ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 952115ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 952115ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11275UL
    //  Error: 1.1275% (0.0112745348 Decimal)
    // Jitter: 0.0216% (0.00021628579999999998 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.011275))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 988725ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 988725ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 988725ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2130UL
    //  Error: 0.2130% (0.0021298607 Decimal)
    // Jitter: 0.0278% (0.0002783623 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.00213))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 997870ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 997870ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 997870ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 995UL
    //  Error: 0.0996% (0.0009957534 Decimal)
    // Jitter: 0.0329% (0.0003288179 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000996))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999004ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999004ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999004ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 434UL
    //  Error: 0.0434% (0.000433541 Decimal)
    // Jitter: 0.0374% (0.0003740969 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000434))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999566ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999566ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999566ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 157UL
    //  Error: 0.0157% (0.0001569292 Decimal)
    // Jitter: 0.0410% (0.0004097833 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000157))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999843ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999843ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999843ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0096% (0.0000955207 Decimal)
    // Jitter: 0.0480% (0.0004795937 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000096))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999904ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999904ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999904ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1000000UL
  // 1 MHz
  //     Best Error Possible: 0.0063%  (0.0000628935 Decimal)
  //    Worst Error Possible: 2.3445% (0.023445183300000002 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23445UL
    //  Error: 2.3445% (0.023445183300000002 Decimal)
    // Jitter: 0.0083% (0.00008333329999999999 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.023445))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 976555ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 976555ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 976555ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8197UL
    //  Error: 0.8197% (0.0081970502 Decimal)
    // Jitter: 0.0133% (0.0001333333 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.008197))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 991803ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 991803ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 991803ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 578UL
    //  Error: 0.0578% (0.0005781245999999999 Decimal)
    // Jitter: 0.0178% (0.00017781720000000002 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000578))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999422ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999422ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999422ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0063% (0.0000628935 Decimal)
    // Jitter: 0.0278% (0.0002777817 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000063))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999937ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999937ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999937ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 800000UL
  // 800 kHz
  //     Best Error Possible: 0.0115%  (0.0001145359 Decimal)
  //    Worst Error Possible: 21.8758% (0.21875796979999998 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 218758UL
    //  Error: 21.8758% (0.21875796979999998 Decimal)
    // Jitter: 0.0068% (0.00006818179999999999 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.218758))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 781242ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 781242ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 781242ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 23455UL
    //  Error: 2.3455% (0.023454928400000002 Decimal)
    // Jitter: 0.0108% (0.0001079545 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.023455))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 976545ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 976545ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 976545ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11258UL
    //  Error: 1.1258% (0.0112578935 Decimal)
    // Jitter: 0.0136% (0.0001362782 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.011258))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 988742ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 988742ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 988742ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 5164UL
    //  Error: 0.5164% (0.0051639588 Decimal)
    // Jitter: 0.0186% (0.00018647909999999998 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.005164))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 994836ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 994836ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 994836ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2123UL
    //  Error: 0.2123% (0.0021232221 Decimal)
    // Jitter: 0.0238% (0.0002382033 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.002123))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 997877ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 997877ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 997877ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 608UL
    //  Error: 0.0608% (0.0006080995 Decimal)
    // Jitter: 0.0290% (0.0002903811 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000608))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999392ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999392ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999392ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 237UL
    //  Error: 0.0237% (0.0002371862 Decimal)
    // Jitter: 0.0301% (0.0003012704 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000237))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999763ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999763ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999763ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0115% (0.0001145359 Decimal)
    // Jitter: 0.0387% (0.0003865699 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000115))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999885ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999885ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999885ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 600000UL
  // 600 kHz
  //     Best Error Possible: 0.0091%  (0.0000913535 Decimal)
  //    Worst Error Possible: 41.4068% (0.4140676903 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 414067UL
    //  Error: 41.4068% (0.4140676903 Decimal)
    // Jitter: 0.0039% (0.0000390023 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.414068))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 585932ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 585932ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 585932ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 121112UL
    //  Error: 12.1112% (0.1211119719 Decimal)
    // Jitter: 0.0103% (0.0001027377 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.121112))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 878888ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 878888ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 878888ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 47881UL
    //  Error: 4.7881% (0.0478809068 Decimal)
    // Jitter: 0.0144% (0.00014398660000000001 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.047881))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 952119ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 952119ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 952119ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11270UL
    //  Error: 1.1270% (0.011270414199999999 Decimal)
    // Jitter: 0.0218% (0.000217923 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.01127))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 988730ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 988730ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 988730ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2126UL
    //  Error: 0.2126% (0.0021257019 Decimal)
    // Jitter: 0.0281% (0.0002812586 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.002126))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 997874ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 997874ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 997874ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 992UL
    //  Error: 0.0992% (0.0009915898999999998 Decimal)
    // Jitter: 0.0329% (0.0003288179 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000992))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999008ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999008ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999008ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 429UL
    //  Error: 0.0429% (0.00042937520000000003 Decimal)
    // Jitter: 0.0374% (0.0003740969 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000429))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999571ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999571ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999571ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 153UL
    //  Error: 0.0153% (0.00015276230000000002 Decimal)
    // Jitter: 0.0410% (0.0004097833 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000153))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999847ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999847ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999847ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0091% (0.0000913535 Decimal)
    // Jitter: 0.0480% (0.0004795937 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000091))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999909ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999909ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999909ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 500000UL
  // 500 kHz
  //     Best Error Possible: 0.0066%  (0.0000656635 Decimal)
  //    Worst Error Possible: 2.3448% (0.0234479327 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23448UL
    //  Error: 2.3448% (0.0234479327 Decimal)
    // Jitter: 0.0062% (0.000062373 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.023448))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 976552ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 976552ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 976552ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8200UL
    //  Error: 0.8200% (0.008199769 Decimal)
    // Jitter: 0.0120% (0.0001195449 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.0082))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 991800ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 991800ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 991800ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 581UL
    //  Error: 0.0581% (0.0005809696 Decimal)
    // Jitter: 0.0176% (0.0001760291 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000581))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999419ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999419ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999419ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0066% (0.0000656635 Decimal)
    // Jitter: 0.0278% (0.0002779503 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000066))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999934ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999934ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999934ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 375000UL
  // 375 kHz
  //     Best Error Possible: 0.0092%  (0.0000915869 Decimal)
  //    Worst Error Possible: 26.7583% (0.26758349670000003 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 267583UL
    //  Error: 26.7583% (0.26758349670000003 Decimal)
    // Jitter: 0.0074% (0.00007386360000000001 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.267583))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 732417ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 732417ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 732417ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 84487UL
    //  Error: 8.4487% (0.0844867312 Decimal)
    // Jitter: 0.0087% (0.0000871645 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.084487))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 915513ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 915513ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 915513ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 38720UL
    //  Error: 3.8720% (0.0387204621 Decimal)
    // Jitter: 0.0127% (0.0001273622 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.03872))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 961280ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 961280ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 961280ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 15843UL
    //  Error: 1.5843% (0.0158427259 Decimal)
    // Jitter: 0.0196% (0.000196471 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.015843))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 984157ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 984157ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 984157ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 4409UL
    //  Error: 0.4409% (0.0044087425 Decimal)
    // Jitter: 0.0275% (0.0002754948 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.004409))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 995591ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 995591ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 995591ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 1558UL
    //  Error: 0.1558% (0.0015579837 Decimal)
    // Jitter: 0.0318% (0.0003182651 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.001558))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 998442ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 998442ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 998442ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0092% (0.0000915869 Decimal)
    // Jitter: 0.0435% (0.0004345473 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000092))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999908ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999908ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999908ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 312500UL
  // 312.5 kHz
  //     Best Error Possible: 0.0098%  (0.000098003 Decimal)
  //    Worst Error Possible: 38.9654% (0.3896535267 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 389654UL
    //  Error: 38.9654% (0.3896535267 Decimal)
    // Jitter: 0.0064% (0.0000636364 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.389654))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 610346ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 610346ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 610346ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 84486UL
    //  Error: 8.4486% (0.0844855181 Decimal)
    // Jitter: 0.0145% (0.0001454545 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.084486))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 915514ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 915514ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 915514ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8202UL
    //  Error: 0.8202% (0.0082016394 Decimal)
    // Jitter: 0.0227% (0.00022715299999999998 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.008202))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 991798ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 991798ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 991798ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 3444UL
    //  Error: 0.3444% (0.0034437723 Decimal)
    // Jitter: 0.0290% (0.0002903556 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.003444))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 996556ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 996556ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 996556ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 1070UL
    //  Error: 0.1070% (0.001070239 Decimal)
    // Jitter: 0.0375% (0.000374543 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.00107))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 998930ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 998930ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 998930ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 485UL
    //  Error: 0.0485% (0.0004848923 Decimal)
    // Jitter: 0.0464% (0.00046418079999999997 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000485))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999515ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999515ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999515ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 198UL
    //  Error: 0.0198% (0.0001975922 Decimal)
    // Jitter: 0.0557% (0.0005571742000000001 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000198))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999802ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999802ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999802ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0098% (0.000098003 Decimal)
    // Jitter: 0.0678% (0.0006777827 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000098))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999902ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999902ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999902ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 300000UL
  // 300 kHz
  //     Best Error Possible: 0.0082%  (0.00008233490000000001 Decimal)
  //    Worst Error Possible: 41.4062% (0.4140624052 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 414062UL
    //  Error: 41.4062% (0.4140624052 Decimal)
    // Jitter: 0.0046% (0.0000456935 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.414062))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 585938ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 585938ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 585938ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 121104UL
    //  Error: 12.1104% (0.12110404429999999 Decimal)
    // Jitter: 0.0072% (0.0000723293 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.121104))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 878896ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 878896ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 878896ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 47872UL
    //  Error: 4.7872% (0.0478723188 Decimal)
    // Jitter: 0.0147% (0.0001471747 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.047872))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 952128ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 952128ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 952128ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11261UL
    //  Error: 1.1261% (0.011261496 Decimal)
    // Jitter: 0.0185% (0.0001845974 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.011261))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 988739ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 988739ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 988739ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2117UL
    //  Error: 0.2117% (0.0021167014 Decimal)
    // Jitter: 0.0252% (0.0002515538 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.002117))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 997883ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 997883ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 997883ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 983UL
    //  Error: 0.0983% (0.0009825792 Decimal)
    // Jitter: 0.0310% (0.00031023520000000003 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000983))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999017ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999017ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999017ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 420UL
    //  Error: 0.0420% (0.00042035950000000004 Decimal)
    // Jitter: 0.0382% (0.0003816122 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.00042))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999580ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999580ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999580ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0082% (0.00008233490000000001 Decimal)
    // Jitter: 0.0485% (0.0004849638 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000082))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999918ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999918ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999918ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 250000UL
  // 250 kHz
  //     Best Error Possible: 0.0055%  (0.0000546916 Decimal)
  //    Worst Error Possible: 2.3437% (0.0234372169 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23437UL
    //  Error: 2.3437% (0.0234372169 Decimal)
    // Jitter: 0.0061% (0.000061198 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.023437))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 976563ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 976563ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 976563ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8189UL
    //  Error: 0.8189% (0.008188886000000001 Decimal)
    // Jitter: 0.0106% (0.0001060606 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.008189))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 991811ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 991811ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 991811ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 570UL
    //  Error: 0.0570% (0.0005700031000000001 Decimal)
    // Jitter: 0.0184% (0.0001839716 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.00057))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999430ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999430ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999430ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0055% (0.0000546916 Decimal)
    // Jitter: 0.0255% (0.0002551896 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000055))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999945ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999945ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999945ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 187500UL
  // 187.5 kHz
  //     Best Error Possible: 0.0085%  (0.0000847758 Decimal)
  //    Worst Error Possible: 26.7578% (0.2675778189 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 267578UL
    //  Error: 26.7578% (0.2675778189 Decimal)
    // Jitter: 0.0055% (0.000054545499999999995 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.267578))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 732422ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 732422ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 732422ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 84480UL
    //  Error: 8.4480% (0.08447986 Decimal)
    // Jitter: 0.0071% (0.0000709122 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.08448))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 915520ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 915520ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 915520ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 38713UL
    //  Error: 3.8713% (0.0387132482 Decimal)
    // Jitter: 0.0119% (0.0001193403 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.038713))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 961287ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 961287ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 961287ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 15835UL
    //  Error: 1.5836% (0.0158355798 Decimal)
    // Jitter: 0.0208% (0.00020789609999999998 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.015836))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 984164ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 984164ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 984164ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 4402UL
    //  Error: 0.4402% (0.0044019472 Decimal)
    // Jitter: 0.0238% (0.00023807089999999998 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.004402))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 995598ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 995598ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 995598ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 1550UL
    //  Error: 0.1551% (0.0015511828 Decimal)
    // Jitter: 0.0293% (0.00029315179999999996 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.001551))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 998449ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 998449ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 998449ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0085% (0.0000847758 Decimal)
    // Jitter: 0.0385% (0.0003851959 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000085))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999915ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999915ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999915ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 150000UL
  // 150 kHz
  //     Best Error Possible: 0.0082%  (0.00008233490000000001 Decimal)
  //    Worst Error Possible: 41.4062% (0.4140624052 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 414062UL
    //  Error: 41.4062% (0.4140624052 Decimal)
    // Jitter: 0.0046% (0.0000456935 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.414062))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 585938ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 585938ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 585938ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 121104UL
    //  Error: 12.1104% (0.12110404429999999 Decimal)
    // Jitter: 0.0072% (0.0000723293 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.121104))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 878896ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 878896ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 878896ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 47872UL
    //  Error: 4.7872% (0.0478723188 Decimal)
    // Jitter: 0.0147% (0.0001471747 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.047872))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 952128ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 952128ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 952128ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11261UL
    //  Error: 1.1261% (0.011261496 Decimal)
    // Jitter: 0.0185% (0.0001845974 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.011261))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 988739ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 988739ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 988739ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2117UL
    //  Error: 0.2117% (0.0021167014 Decimal)
    // Jitter: 0.0252% (0.0002515538 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.002117))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 997883ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 997883ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 997883ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 983UL
    //  Error: 0.0983% (0.0009825792 Decimal)
    // Jitter: 0.0310% (0.00031023520000000003 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000983))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999017ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999017ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999017ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 420UL
    //  Error: 0.0420% (0.00042035950000000004 Decimal)
    // Jitter: 0.0382% (0.0003816122 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.00042))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999580ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999580ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999580ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0082% (0.00008233490000000001 Decimal)
    // Jitter: 0.0485% (0.0004849638 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000082))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999918ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999918ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999918ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 128000UL
  // 128 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MILLIS(AVR_MILLIS) ((uint32_t) (AVR_MILLIS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 125000UL
  // 125 kHz
  //     Best Error Possible: 0.0055%  (0.0000545291 Decimal)
  //    Worst Error Possible: 2.3437% (0.0234373939 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23437UL
    //  Error: 2.3437% (0.0234373939 Decimal)
    // Jitter: 0.0055% (0.0000547528 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.023437))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 976563ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 976563ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 976563ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8189UL
    //  Error: 0.8189% (0.0081890157 Decimal)
    // Jitter: 0.0107% (0.00010674110000000001 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.008189))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 991811ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 991811ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 991811ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 570UL
    //  Error: 0.0570% (0.0005699991999999999 Decimal)
    // Jitter: 0.0182% (0.0001824126 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.00057))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999430ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999430ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999430ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0055% (0.0000545291 Decimal)
    // Jitter: 0.0258% (0.00025814839999999996 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000055))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999945ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999945ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999945ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 100000UL
  // 100 kHz
  //     Best Error Possible: 0.0087%  (0.00008692650000000001 Decimal)
  //    Worst Error Possible: 21.8749% (0.21874940969999998 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 218749UL
    //  Error: 21.8749% (0.21874940969999998 Decimal)
    // Jitter: 0.0040% (0.0000402168 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.218749))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 781251ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 781251ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 781251ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 23442UL
    //  Error: 2.3442% (0.0234418821 Decimal)
    // Jitter: 0.0066% (0.0000663896 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.023442))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 976558ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 976558ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 976558ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11245UL
    //  Error: 1.1245% (0.0112446471 Decimal)
    // Jitter: 0.0105% (0.0001048328 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.011245))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 988755ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 988755ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 988755ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 5151UL
    //  Error: 0.5151% (0.0051506819 Decimal)
    // Jitter: 0.0133% (0.00013293700000000002 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.005151))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 994849ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 994849ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 994849ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2110UL
    //  Error: 0.2110% (0.0021097311 Decimal)
    // Jitter: 0.0160% (0.0001601772 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.00211))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 997890ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 997890ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 997890ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 594UL
    //  Error: 0.0594% (0.0005942365 Decimal)
    // Jitter: 0.0217% (0.0002168587 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000594))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999406ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999406ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999406ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 223UL
    //  Error: 0.0223% (0.0002234391 Decimal)
    // Jitter: 0.0288% (0.00028770170000000004 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000223))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999777ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999777ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999777ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0087% (0.00008692650000000001 Decimal)
    // Jitter: 0.0373% (0.0003726614 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000087))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999913ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999913ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999913ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 75000UL
  // 75 kHz
  //     Best Error Possible: 0.0068%  (0.00006818429999999999 Decimal)
  //    Worst Error Possible: 12.1094% (0.12109413129999999 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 121094UL
    //  Error: 12.1094% (0.12109413129999999 Decimal)
    // Jitter: 0.0051% (0.000051157400000000006 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.121094))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 878906ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 878906ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 878906ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 47859UL
    //  Error: 4.7860% (0.0478598407 Decimal)
    // Jitter: 0.0095% (0.00009549769999999999 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.04786))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 952140ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 952140ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 952140ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11248UL
    //  Error: 1.1248% (0.011247735699999999 Decimal)
    // Jitter: 0.0144% (0.00014372999999999998 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.011248))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 988752ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 988752ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 988752ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2103UL
    //  Error: 0.2103% (0.0021026210000000003 Decimal)
    // Jitter: 0.0207% (0.0002065198 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.002103))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 997897ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 997897ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 997897ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 968UL
    //  Error: 0.0968% (0.0009684594 Decimal)
    // Jitter: 0.0249% (0.000248762 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000968))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999032ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999032ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999032ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 406UL
    //  Error: 0.0406% (0.00040622019999999996 Decimal)
    // Jitter: 0.0303% (0.00030316500000000004 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000406))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999594ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999594ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999594ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0068% (0.00006818429999999999 Decimal)
    // Jitter: 0.0407% (0.00040653280000000003 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000068))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999932ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999932ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999932ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 62500UL
  // 62.5 kHz
  //     Best Error Possible: 0.0053%  (0.000053184600000000006 Decimal)
  //    Worst Error Possible: 2.3437% (0.023436712800000002 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23437UL
    //  Error: 2.3437% (0.023436712800000002 Decimal)
    // Jitter: 0.0055% (0.000055436900000000005 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.023437))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 976563ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 976563ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 976563ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8188UL
    //  Error: 0.8188% (0.0081878192 Decimal)
    // Jitter: 0.0116% (0.00011633599999999999 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.008188))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 991812ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 991812ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 991812ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 569UL
    //  Error: 0.0569% (0.0005687699 Decimal)
    // Jitter: 0.0188% (0.0001879552 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000569))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999431ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999431ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999431ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0053% (0.000053184600000000006 Decimal)
    // Jitter: 0.0258% (0.0002582281 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000053))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999947ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999947ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999947ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 37500UL
  // 37.5 kHz
  //     Best Error Possible: 0.0064%  (0.0000640909 Decimal)
  //    Worst Error Possible: 12.1093% (0.1210933981 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 121093UL
    //  Error: 12.1093% (0.1210933981 Decimal)
    // Jitter: 0.0040% (0.000040286 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.121093))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 878907ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 878907ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 878907ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 47855UL
    //  Error: 4.7856% (0.047856293700000004 Decimal)
    // Jitter: 0.0089% (0.000089106 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.047856))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 952144ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 952144ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 952144ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11243UL
    //  Error: 1.1243% (0.011242939 Decimal)
    // Jitter: 0.0159% (0.0001589788 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.011243))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 988757ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 988757ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 988757ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2098UL
    //  Error: 0.2098% (0.0020975313000000002 Decimal)
    // Jitter: 0.0220% (0.0002200943 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.002098))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 997902ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 997902ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 997902ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 963UL
    //  Error: 0.0963% (0.0009634308 Decimal)
    // Jitter: 0.0254% (0.000253992 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000963))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999037ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999037ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999037ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 400UL
    //  Error: 0.0401% (0.0004014773 Decimal)
    // Jitter: 0.0280% (0.0002795405 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000401))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999599ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999599ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999599ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0064% (0.0000640909 Decimal)
    // Jitter: 0.0364% (0.0003637025 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000064))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999936ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999936ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999936ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 32768UL
  // 32.768 kHz
  //     Best Error Possible: 0.0000%  (0 Decimal)
  //    Worst Error Possible: 10.4000% (0.104 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 104000UL
    //  Error: 10.4000% (0.104 Decimal)
    // Jitter: 0.0000% (0 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.104))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 896000ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 896000ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 896000ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 40000UL
    //  Error: 4.0000% (0.04 Decimal)
    // Jitter: 0.0000% (0 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.04))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 960000ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 960000ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 960000ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8000UL
    //  Error: 0.8000% (0.008 Decimal)
    // Jitter: 0.0000% (0 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.008))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 992000ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 992000ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 992000ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0000% (0 Decimal)
    // Jitter: 0.0000% (0 Decimal)

    #define REAL_MILLIS(AVR_MILLIS) ((uint32_t) (AVR_MILLIS))
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 31250UL
  // 31.25 kHz
  //     Best Error Possible: 0.0052%  (0.0000522177 Decimal)
  //    Worst Error Possible: 2.3437% (0.0234370909 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23437UL
    //  Error: 2.3437% (0.0234370909 Decimal)
    // Jitter: 0.0064% (0.00006404550000000001 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.023437))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 976563ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 976563ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 976563ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8188UL
    //  Error: 0.8188% (0.0081875548 Decimal)
    // Jitter: 0.0111% (0.0001106772 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.008188))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 991812ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 991812ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 991812ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 568UL
    //  Error: 0.0568% (0.0005678393 Decimal)
    // Jitter: 0.0187% (0.0001868103 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000568))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999432ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999432ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999432ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0052% (0.0000522177 Decimal)
    // Jitter: 0.0258% (0.0002581808 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000052))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999948ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999948ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999948ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 18750UL
  // 18.75 kHz
  //     Best Error Possible: 0.0050%  (0.000049767899999999995 Decimal)
  //    Worst Error Possible: 4.7852% (0.047851534099999996 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 47852UL
    //  Error: 4.7852% (0.047851534099999996 Decimal)
    // Jitter: 0.0059% (0.0000586267 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.047852))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 952148ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 952148ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 952148ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11230UL
    //  Error: 1.1230% (0.011230439200000001 Decimal)
    // Jitter: 0.0061% (0.000060881599999999997 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.01123))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 988770ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 988770ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 988770ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2083UL
    //  Error: 0.2083% (0.0020830862999999997 Decimal)
    // Jitter: 0.0126% (0.00012552049999999999 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.002083))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 997917ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 997917ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 997917ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 949UL
    //  Error: 0.0949% (0.0009487892000000001 Decimal)
    // Jitter: 0.0168% (0.00016777330000000002 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000949))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999051ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999051ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999051ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 387UL
    //  Error: 0.0387% (0.0003868662 Decimal)
    // Jitter: 0.0190% (0.0001903451 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000387))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999613ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999613ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999613ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 111UL
    //  Error: 0.0111% (0.0001108822 Decimal)
    // Jitter: 0.0229% (0.0002292078 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000111))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999889ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999889ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999889ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0050% (0.000049767899999999995 Decimal)
    // Jitter: 0.0300% (0.00030018009999999997 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.00005))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999950ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999950ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999950ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 16000UL
  // 16 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MILLIS(AVR_MILLIS) ((uint32_t) (AVR_MILLIS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 15625UL
  // 15.625 kHz
  //     Best Error Possible: 0.0046%  (0.0000461119 Decimal)
  //    Worst Error Possible: 2.3437% (0.023437407599999998 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23437UL
    //  Error: 2.3437% (0.023437407599999998 Decimal)
    // Jitter: 0.0058% (0.0000579497 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.023437))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 976563ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 976563ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 976563ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 8184UL
    //  Error: 0.8184% (0.0081838807 Decimal)
    // Jitter: 0.0072% (0.00007187699999999999 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.008184))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 991816ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 991816ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 991816ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 562UL
    //  Error: 0.0562% (0.000562095 Decimal)
    // Jitter: 0.0134% (0.000134475 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000562))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999438ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999438ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999438ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 95UL
    //  Error: 0.0095% (0.00009518080000000001 Decimal)
    // Jitter: 0.0183% (0.0001831157 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000095))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999905ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999905ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999905ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0046% (0.0000461119 Decimal)
    // Jitter: 0.0242% (0.0002421586 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000046))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999954ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999954ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999954ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 12500UL
  // 12.5 kHz
  //     Best Error Possible: 0.0078%  (0.0000782744 Decimal)
  //    Worst Error Possible: 2.3437% (0.023437109 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 23437UL
    //  Error: 2.3437% (0.023437109 Decimal)
    // Jitter: 0.0045% (0.000044762300000000003 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.023437))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 976563ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 976563ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 976563ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 11238UL
    //  Error: 1.1238% (0.0112376471 Decimal)
    // Jitter: 0.0097% (0.00009662080000000001 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.011238))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 988762ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 988762ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 988762ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 5143UL
    //  Error: 0.5143% (0.0051427518 Decimal)
    // Jitter: 0.0126% (0.0001263876 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.005143))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 994857ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 994857ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 994857ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2101UL
    //  Error: 0.2101% (0.0021012148000000004 Decimal)
    // Jitter: 0.0180% (0.00017975239999999998 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.002101))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 997899ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 997899ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 997899ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 586UL
    //  Error: 0.0586% (0.0005856277 Decimal)
    // Jitter: 0.0225% (0.00022505439999999998 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000586))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999414ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999414ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999414ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 214UL
    //  Error: 0.0215% (0.000214637 Decimal)
    // Jitter: 0.0295% (0.0002951753 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000215))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999785ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999785ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999785ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0078% (0.0000782744 Decimal)
    // Jitter: 0.0369% (0.00036935969999999995 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000078))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999922ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999922ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999922ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 9375UL
  // 9.375 kHz
  //     Best Error Possible: 0.0050%  (0.000049767899999999995 Decimal)
  //    Worst Error Possible: 1.1230% (0.011230439200000001 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 11230UL
    //  Error: 1.1230% (0.011230439200000001 Decimal)
    // Jitter: 0.0061% (0.000060881599999999997 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.01123))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 988770ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 988770ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 988770ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 2083UL
    //  Error: 0.2083% (0.0020830862999999997 Decimal)
    // Jitter: 0.0126% (0.00012552049999999999 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.002083))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 997917ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 997917ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 997917ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 949UL
    //  Error: 0.0949% (0.0009487892000000001 Decimal)
    // Jitter: 0.0168% (0.00016777330000000002 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000949))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999051ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999051ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999051ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 387UL
    //  Error: 0.0387% (0.0003868662 Decimal)
    // Jitter: 0.0190% (0.0001903451 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000387))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999613ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999613ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999613ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 111UL
    //  Error: 0.0111% (0.0001108822 Decimal)
    // Jitter: 0.0229% (0.0002292078 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000111))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999889ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999889ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999889ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0050% (0.000049767899999999995 Decimal)
    // Jitter: 0.0300% (0.00030018009999999997 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.00005))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999950ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999950ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999950ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 4096UL
  // 4.096 kHz
  //     Best Error Possible: 0.0000%  (0 Decimal)
  //    Worst Error Possible: 0.8000% (0.008 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 8000UL
    //  Error: 0.8000% (0.008 Decimal)
    // Jitter: 0.0000% (0 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.008))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 992000ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 992000ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 992000ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0000% (0 Decimal)
    // Jitter: 0.0000% (0 Decimal)

    #define REAL_MILLIS(AVR_MILLIS) ((uint32_t) (AVR_MILLIS))
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2000UL
  // 2 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MILLIS(AVR_MILLIS) ((uint32_t) (AVR_MILLIS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1953UL
  // 1.953 kHz
  //     Best Error Possible: 0.0031%  (0.0000310192 Decimal)
  //    Worst Error Possible: 0.0613% (0.0006130248 Decimal)

  #if      ACCEPTABLE_MILLIS_ERROR_PPM >= 613UL
    //  Error: 0.0613% (0.0006130248 Decimal)
    // Jitter: 0.0055% (0.000055248600000000005 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000613))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999387ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999387ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999387ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MILLIS_ERROR_PPM >= 146UL
    //  Error: 0.0146% (0.0001464549 Decimal)
    // Jitter: 0.0085% (0.00008475830000000001 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000146))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999854ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999854ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999854ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0031% (0.0000310192 Decimal)
    // Jitter: 0.0141% (0.0001412729 Decimal)

    // The below define is equivalent to ROUND(AVR_MILLIS * (1-0.000031))
    #define REAL_MILLIS(AVR_MILLIS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MILLIS)) * 999969ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MILLIS)) * 999969ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MILLIS)) * 999969ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 512UL
  // 0.512 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MILLIS(AVR_MILLIS) ((uint32_t) (AVR_MILLIS))
#endif

  // REAL_MICROS(x)
#if (F_CPU / MILLIS_TIMER_PRESCALE) >= 24000000UL
  // 24 MHz
  //     Best Error Possible: 0.0074%  (0.0000739307 Decimal)
  //    Worst Error Possible: 6.2500% (0.0625000838 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 62500UL
    //  Error: 6.2500% (0.0625000838 Decimal)
    // Jitter: 0.0060% (0.000059682499999999997 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.0625))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 937500ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 937500ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 937500ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 15636UL
    //  Error: 1.5636% (0.0156355095 Decimal)
    // Jitter: 0.0082% (0.0000816024 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.015636))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 984364ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 984364ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 984364ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 3927UL
    //  Error: 0.3927% (0.0039271237 Decimal)
    // Jitter: 0.0094% (0.00009356999999999999 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.003927))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 996073ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 996073ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 996073ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 1007UL
    //  Error: 0.1007% (0.0010074724 Decimal)
    // Jitter: 0.0153% (0.0001532351 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.001007))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 998993ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 998993ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 998993ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 285UL
    //  Error: 0.0285% (0.000285344 Decimal)
    // Jitter: 0.0213% (0.0002133005 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000285))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999715ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999715ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999715ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0074% (0.0000739307 Decimal)
    // Jitter: 0.0315% (0.00031501690000000003 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000074))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999926ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999926ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999926ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 20000000UL
  // 20 MHz
  //     Best Error Possible: 0.0068%  (0.000067535 Decimal)
  //    Worst Error Possible: 6.2500% (0.0625000152 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 62500UL
    //  Error: 6.2500% (0.0625000152 Decimal)
    // Jitter: 0.0055% (0.0000554853 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.0625))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 937500ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 937500ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 937500ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 23443UL
    //  Error: 2.3443% (0.0234427938 Decimal)
    // Jitter: 0.0100% (0.000099531 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.023443))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 976557ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 976557ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 976557ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 3919UL
    //  Error: 0.3919% (0.0039191754 Decimal)
    // Jitter: 0.0112% (0.0001121674 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.003919))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 996081ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 996081ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 996081ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 1487UL
    //  Error: 0.1488% (0.001487974 Decimal)
    // Jitter: 0.0139% (0.00013947550000000002 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.001488))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 998512ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 998512ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 998512ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 277UL
    //  Error: 0.0277% (0.0002772847 Decimal)
    // Jitter: 0.0176% (0.00017571119999999999 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000277))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999723ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999723ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999723ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0068% (0.000067535 Decimal)
    // Jitter: 0.0318% (0.0003179226 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000068))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999932ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999932ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999932ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 19200000UL
  // 19.2 MHz
  //     Best Error Possible: 0.0065%  (0.0000651144 Decimal)
  //    Worst Error Possible: 2.5000% (0.025 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 25000UL
    //  Error: 2.5000% (0.025 Decimal)
    // Jitter: 0.0000% (0 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.025))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 975000ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 975000ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 975000ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 6258UL
    //  Error: 0.6258% (0.0062577905 Decimal)
    // Jitter: 0.0058% (0.0000576923 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.006258))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 993742ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 993742ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 993742ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 1580UL
    //  Error: 0.1580% (0.0015803789 Decimal)
    // Jitter: 0.0138% (0.0001375 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.00158))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 998420ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 998420ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 998420ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 419UL
    //  Error: 0.0419% (0.0004190177 Decimal)
    // Jitter: 0.0209% (0.000209375 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000419))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999581ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999581ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999581ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0065% (0.0000651144 Decimal)
    // Jitter: 0.0384% (0.000384375 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000065))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999935ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999935ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999935ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 16000000UL
  // 16 MHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 12000000UL
  // 12 MHz
  //     Best Error Possible: 0.0060%  (0.0000600665 Decimal)
  //    Worst Error Possible: 1.5625% (0.0156250974 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 15625UL
    //  Error: 1.5625% (0.0156250974 Decimal)
    // Jitter: 0.0057% (0.0000572337 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.015625))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 984375ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 984375ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 984375ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 3914UL
    //  Error: 0.3914% (0.0039141093 Decimal)
    // Jitter: 0.0117% (0.0001169239 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.003914))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 996086ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 996086ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 996086ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 994UL
    //  Error: 0.0994% (0.0009938101 Decimal)
    // Jitter: 0.0127% (0.00012672400000000002 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000994))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999006ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999006ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999006ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 272UL
    //  Error: 0.0272% (0.0002715227 Decimal)
    // Jitter: 0.0167% (0.0001670341 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000272))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999728ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999728ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999728ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0060% (0.0000600665 Decimal)
    // Jitter: 0.0258% (0.00025756650000000004 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.00006))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999940ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999940ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999940ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 9600000UL
  // 9.6 MHz
  //     Best Error Possible: 0.0068%  (0.0000677557 Decimal)
  //    Worst Error Possible: 2.5000% (0.0249999037 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 25000UL
    //  Error: 2.5000% (0.0249999037 Decimal)
    // Jitter: 0.0057% (0.000056533099999999996 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.025))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 975000ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 975000ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 975000ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 6260UL
    //  Error: 0.6260% (0.0062602516 Decimal)
    // Jitter: 0.0068% (0.00006789860000000001 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.00626))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 993740ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 993740ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 993740ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 1583UL
    //  Error: 0.1583% (0.0015833386 Decimal)
    // Jitter: 0.0135% (0.0001353933 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.001583))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 998417ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 998417ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 998417ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 422UL
    //  Error: 0.0422% (0.0004219071 Decimal)
    // Jitter: 0.0207% (0.0002065505 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000422))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999578ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999578ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999578ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0068% (0.0000677557 Decimal)
    // Jitter: 0.0384% (0.0003841938 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000068))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999932ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999932ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999932ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


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
  //     Best Error Possible: 0.0054%  (0.0000540151 Decimal)
  //    Worst Error Possible: 0.6250% (0.0062499106 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 6250UL
    //  Error: 0.6250% (0.0062499106 Decimal)
    // Jitter: 0.0063% (0.0000633639 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.00625))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 993750ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 993750ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 993750ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 1570UL
    //  Error: 0.1570% (0.0015704186 Decimal)
    // Jitter: 0.0103% (0.0001027703 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.00157))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 998430ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 998430ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 998430ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 408UL
    //  Error: 0.0408% (0.00040835000000000003 Decimal)
    // Jitter: 0.0162% (0.0001622073 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000408))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999592ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999592ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999592ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0054% (0.0000540151 Decimal)
    // Jitter: 0.0292% (0.0002917454 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000054))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999946ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999946ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999946ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 4000000UL
  // 4 MHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 3000000UL
  // 3 MHz
  //     Best Error Possible: 0.0046%  (0.0000462079 Decimal)
  //    Worst Error Possible: 0.3906% (0.00390625 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 3906UL
    //  Error: 0.3906% (0.00390625 Decimal)
    // Jitter: 0.0000% (0 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.003906))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 996094ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 996094ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 996094ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 982UL
    //  Error: 0.0982% (0.000981706 Decimal)
    // Jitter: 0.0023% (0.0000232548 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000982))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999018ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999018ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999018ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 258UL
    //  Error: 0.0258% (0.00025814269999999996 Decimal)
    // Jitter: 0.0086% (0.0000860414 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000258))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999742ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999742ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999742ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 85UL
    //  Error: 0.0085% (0.00008495610000000001 Decimal)
    // Jitter: 0.0137% (0.0001366749 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000085))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999915ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999915ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999915ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0046% (0.0000462079 Decimal)
    // Jitter: 0.0191% (0.0001911724 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000046))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999954ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999954ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999954ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2500000UL
  // 2.5 MHz
  //     Best Error Possible: 0.0059%  (0.0000587247 Decimal)
  //    Worst Error Possible: 0.3910% (0.0039103564 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 3910UL
    //  Error: 0.3910% (0.0039103564 Decimal)
    // Jitter: 0.0019% (0.0000186331 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.00391))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 996090ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 996090ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 996090ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 1479UL
    //  Error: 0.1479% (0.0014791701 Decimal)
    // Jitter: 0.0083% (0.00008343949999999999 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.001479))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 998521ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 998521ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 998521ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 269UL
    //  Error: 0.0269% (0.00026853840000000005 Decimal)
    // Jitter: 0.0136% (0.0001361274 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000269))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999731ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999731ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999731ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0059% (0.0000587247 Decimal)
    // Jitter: 0.0293% (0.0002925152 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000059))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999941ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999941ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999941ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2400000UL
  // 2.4 MHz
  //     Best Error Possible: 0.0056%  (0.0000559561 Decimal)
  //    Worst Error Possible: 0.6250% (0.0062500981 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 6250UL
    //  Error: 0.6250% (0.0062500981 Decimal)
    // Jitter: 0.0058% (0.0000576321 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.00625))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 993750ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 993750ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 993750ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 1573UL
    //  Error: 0.1573% (0.0015726446 Decimal)
    // Jitter: 0.0075% (0.0000746327 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.001573))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 998427ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 998427ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 998427ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 411UL
    //  Error: 0.0411% (0.00041096 Decimal)
    // Jitter: 0.0153% (0.00015339150000000002 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000411))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999589ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999589ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999589ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0056% (0.0000559561 Decimal)
    // Jitter: 0.0288% (0.00028813900000000003 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000056))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999944ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999944ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999944ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 2000000UL
  // 2 MHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1500000UL
  // 1.5 MHz
  //     Best Error Possible: 0.0046%  (0.0000463401 Decimal)
  //    Worst Error Possible: 0.3913% (0.0039131025 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 3913UL
    //  Error: 0.3913% (0.0039131025 Decimal)
    // Jitter: 0.0031% (0.0000311693 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.003913))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 996087ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 996087ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 996087ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 983UL
    //  Error: 0.0983% (0.0009834352 Decimal)
    // Jitter: 0.0031% (0.000031261 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000983))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999017ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999017ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999017ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 259UL
    //  Error: 0.0259% (0.0002586137 Decimal)
    // Jitter: 0.0088% (0.0000882548 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000259))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999741ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999741ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999741ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 85UL
    //  Error: 0.0085% (0.0000851365 Decimal)
    // Jitter: 0.0138% (0.0001383519 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000085))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999915ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999915ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999915ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0046% (0.0000463401 Decimal)
    // Jitter: 0.0193% (0.0001928854 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000046))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999954ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999954ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999954ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 1200000UL
  // 1.2 MHz
  //     Best Error Possible: 0.0042%  (0.0000424244 Decimal)
  //    Worst Error Possible: 0.1562% (0.0015623082 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 1562UL
    //  Error: 0.1562% (0.0015623082 Decimal)
    // Jitter: 0.0062% (0.0000617729 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.001562))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 998438ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 998438ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 998438ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 398UL
    //  Error: 0.0398% (0.00039814280000000004 Decimal)
    // Jitter: 0.0092% (0.0000920643 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000398))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999602ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999602ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999602ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 115UL
    //  Error: 0.0115% (0.0001147225 Decimal)
    // Jitter: 0.0112% (0.000112258 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000115))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999885ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999885ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999885ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0042% (0.0000424244 Decimal)
    // Jitter: 0.0204% (0.0002038113 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000042))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999958ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999958ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999958ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


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
  //     Best Error Possible: 0.0030%  (0.0000295508 Decimal)
  //    Worst Error Possible: 0.1557% (0.0015565354 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 1557UL
    //  Error: 0.1557% (0.0015565354 Decimal)
    // Jitter: 0.0032% (0.0000319847 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.001557))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 998443ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 998443ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 998443ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 388UL
    //  Error: 0.0389% (0.0003892342 Decimal)
    // Jitter: 0.0047% (0.0000470348 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000389))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999611ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999611ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999611ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 104UL
    //  Error: 0.0104% (0.000104255 Decimal)
    // Jitter: 0.0131% (0.0001305673 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000104))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999896ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999896ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999896ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0030% (0.0000295508 Decimal)
    // Jitter: 0.0204% (0.0002038113 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.00003))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999970ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999970ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999970ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 500000UL
  // 500 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 375000UL
  // 375 kHz
  //     Best Error Possible: 0.0026%  (0.0000256368 Decimal)
  //    Worst Error Possible: 0.0976% (0.0009764631 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 976UL
    //  Error: 0.0976% (0.0009764631 Decimal)
    // Jitter: 0.0065% (0.0000653312 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000976))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999024ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999024ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999024ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 244UL
    //  Error: 0.0244% (0.0002440412 Decimal)
    // Jitter: 0.0065% (0.00006537909999999999 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000244))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999756ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999756ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999756ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 68UL
    //  Error: 0.0068% (0.0000675028 Decimal)
    // Jitter: 0.0087% (0.00008738760000000001 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000068))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999932ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999932ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999932ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0026% (0.0000256368 Decimal)
    // Jitter: 0.0133% (0.0001331647 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000026))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999974ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999974ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999974ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 312500UL
  // 312.5 kHz
  //     Best Error Possible: 0.0024%  (0.0000237285 Decimal)
  //    Worst Error Possible: 0.0244% (0.0002439502 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 244UL
    //  Error: 0.0244% (0.0002439502 Decimal)
    // Jitter: 0.0068% (0.0000678012 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000244))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999756ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999756ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999756ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 97UL
    //  Error: 0.0097% (0.0000972144 Decimal)
    // Jitter: 0.0098% (0.000098088 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000097))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999903ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999903ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999903ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0024% (0.0000237285 Decimal)
    // Jitter: 0.0174% (0.00017438310000000002 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000024))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999976ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999976ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999976ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 300000UL
  // 300 kHz
  //     Best Error Possible: 0.0028%  (0.0000281963 Decimal)
  //    Worst Error Possible: 0.0391% (0.0003907308 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 391UL
    //  Error: 0.0391% (0.0003907308 Decimal)
    // Jitter: 0.0066% (0.0000655335 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000391))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999609ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999609ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999609ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 105UL
    //  Error: 0.0105% (0.0001053768 Decimal)
    // Jitter: 0.0123% (0.00012291370000000002 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000105))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999895ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999895ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999895ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0028% (0.0000281963 Decimal)
    // Jitter: 0.0209% (0.00020863839999999998 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000028))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999972ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999972ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999972ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 250000UL
  // 250 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 187500UL
  // 187.5 kHz
  //     Best Error Possible: 0.0026%  (0.0000256368 Decimal)
  //    Worst Error Possible: 0.0244% (0.0002440412 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 244UL
    //  Error: 0.0244% (0.0002440412 Decimal)
    // Jitter: 0.0065% (0.00006537909999999999 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000244))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999756ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999756ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999756ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 68UL
    //  Error: 0.0068% (0.0000675028 Decimal)
    // Jitter: 0.0087% (0.00008738760000000001 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000068))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999932ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999932ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999932ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0026% (0.0000256368 Decimal)
    // Jitter: 0.0133% (0.0001331647 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000026))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999974ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999974ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999974ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 150000UL
  // 150 kHz
  //     Best Error Possible: 0.0023%  (0.000023453 Decimal)
  //    Worst Error Possible: 0.0390% (0.0003903993 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 390UL
    //  Error: 0.0390% (0.0003903993 Decimal)
    // Jitter: 0.0067% (0.00006693890000000001 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.00039))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999610ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999610ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999610ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #elif    ACCEPTABLE_MICROS_ERROR_PPM >= 101UL
    //  Error: 0.0101% (0.00010142050000000001 Decimal)
    // Jitter: 0.0094% (0.00009416209999999999 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000101))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999899ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999899ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999899ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0023% (0.000023453 Decimal)
    // Jitter: 0.0167% (0.00016740600000000002 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000023))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999977ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999977ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999977ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


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
  //     Best Error Possible: 0.0013%  (0.00001288 Decimal)
  //    Worst Error Possible: 0.0098% (0.0000979598 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 98UL
    //  Error: 0.0098% (0.0000979598 Decimal)
    // Jitter: 0.0073% (0.0000732332 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000098))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999902ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999902ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999902ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0013% (0.00001288 Decimal)
    // Jitter: 0.0146% (0.00014647720000000001 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000013))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999987ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999987ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999987ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 62500UL
  // 62.5 kHz
  //  Error: 0.0000% (0 Decimal)
  // Jitter: 0.0000% (0 Decimal)

  #define REAL_MICROS(AVR_MICROS) ((uint32_t) (AVR_MICROS))


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 37500UL
  // 37.5 kHz
  //     Best Error Possible: 0.0008%  (0.0000076401 Decimal)
  //    Worst Error Possible: 0.0098% (0.00009788590000000001 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 98UL
    //  Error: 0.0098% (0.00009788590000000001 Decimal)
    // Jitter: 0.0073% (0.0000732332 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000098))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999902ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999902ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999902ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0008% (0.0000076401 Decimal)
    // Jitter: 0.0146% (0.00014647720000000001 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000008))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999992ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999992ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999992ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


#elif (F_CPU / MILLIS_TIMER_PRESCALE) >= 32768UL
  // 32.768 kHz
  //     Best Error Possible: 0.0003%  (0.0000030611000000000002 Decimal)
  //    Worst Error Possible: 0.0066% (0.0000655305 Decimal)

  #if      ACCEPTABLE_MICROS_ERROR_PPM >= 66UL
    //  Error: 0.0066% (0.0000655305 Decimal)
    // Jitter: 0.0064% (0.00006399179999999999 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000066))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999934ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999934ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999934ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #else
    //  Error: 0.0003% (0.0000030611000000000002 Decimal)
    // Jitter: 0.0128% (0.0001279918 Decimal)

    // The below define is equivalent to ROUND(AVR_MICROS * (1-0.000003))
    #define REAL_MICROS(AVR_MICROS)                                               \
    ((uint32_t)(                                                                  \
       (   ( ((uint64_t)(AVR_MICROS)) * 999997ULL )%1000000ULL >= 500000ULL )     \
       ? ((( ((uint64_t)(AVR_MICROS)) * 999997ULL )/1000000ULL)+1)                \
       : ( ( ((uint64_t)(AVR_MICROS)) * 999997ULL )/1000000ULL   )                \
    ))                                                                            \
  
  
    #endif


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