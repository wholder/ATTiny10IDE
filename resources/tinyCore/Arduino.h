#ifndef Arduino_h
#define Arduino_h

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "binary.h"

#ifdef __cplusplus
extern "C"{
#endif

#define ATTINY_CORE 1

#ifdef NO_MILLIS
#define ATTINY_CORE_NM 1
#endif
  
void yield(void);

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2


#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

#define SERIAL  0x0
#define DISPLAY 0x1

#define LSBFIRST 0
#define MSBFIRST 1

#define CHANGE 1
#define FALLING 2
#define RISING 3

#define NOT_AN_INTERRUPT -1

// undefine stdlib's abs if encountered
#ifdef abs
#undef abs
#endif

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define abs(x) ((x)>0?(x):-(x))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define round(x)     ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define radians(deg) ((deg)*DEG_TO_RAD)
#define degrees(rad) ((rad)*RAD_TO_DEG)
#define sq(x) ((x)*(x))

#define interrupts() sei()
#define noInterrupts() cli()

#if F_CPU < 1000000L
//Prevent a divide by 0 is 
#warning Clocks per microsecond < 1. To prevent divide by 0, it is rounded up to 1.
//static inline unsigned long clockCyclesPerMicrosecond() __attribute__ ((always_inline));
//static inline unsigned long clockCyclesPerMicrosecond()
//{//
//Inline function will be optimised out.
//  return 1;
//}
  //WTF were they thinking?! 
#define clockCyclesPerMicrosecond() 1L
#else
#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
#endif

#define clockCyclesToMicroseconds(a) ( ((a) * 1000L) / (F_CPU / 1000L) )
#define microsecondsToClockCycles(a) ( ((a) * (F_CPU / 1000L)) / 1000L )

#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))


typedef unsigned int word;

#define bit(b) (1UL << (b))

typedef uint8_t boolean;
typedef uint8_t byte;

void init(void);

void analogReference(uint8_t mode);


unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout);
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);

void attachInterrupt(uint8_t, void (*)(void), int mode);
void detachInterrupt(uint8_t);

void setup(void);
void loop(void);

// Get the bit location within the hardware port of the given virtual pin.
// This comes from the pins_*.c file for the active board configuration.

#define analogInPinToBit(P) (P)

extern const uint16_t PROGMEM port_to_mode_PGM[];
extern const uint16_t PROGMEM port_to_input_PGM[];
extern const uint16_t PROGMEM port_to_output_PGM[];

extern const uint8_t PROGMEM digital_pin_to_port_PGM[];
extern const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[];
extern const uint8_t PROGMEM digital_pin_to_timer_PGM[];

// Get the bit location within the hardware port of the given virtual pin.
// This comes from the pins_*.c file for the active board configuration.
// 
// These perform slightly better as macros compared to inline functions
//
#define digitalPinToPort(P) ( pgm_read_byte( digital_pin_to_port_PGM + (P) ) )
#define digitalPinToBitMask(P) ( pgm_read_byte( digital_pin_to_bit_mask_PGM + (P) ) )
#define digitalPinToTimer(P) ( pgm_read_byte( digital_pin_to_timer_PGM + (P) ) )
#define analogInPinToBit(P) (P)
#define portOutputRegister(P) ( (volatile uint8_t *)( pgm_read_word( port_to_output_PGM + (P))) )
#define portInputRegister(P) ( (volatile uint8_t *)( pgm_read_word( port_to_input_PGM + (P))) )
#define portModeRegister(P) ( (volatile uint8_t *)( pgm_read_word( port_to_mode_PGM + (P))) )

// Some ATTiny have separate pull up registers from the output register, but most don't
// so if yours does, pins_arduino.h will do something useful here.
#define pullupEnableRegister(P) portOutputRegister(P)
#define NOT_A_PIN 0
#define NOT_A_PORT 0

#define PA 1
#define PB 2
#define PC 3
#define PD 4

#define NOT_ON_TIMER 0
#define TIMER0A 1
#define TIMER0B 2
#define TIMER1A 3
#define TIMER1B 4
#define TIMER1D 5

#define SERIAL_TYPE_NONE      0x00
#define SERIAL_TYPE_HARDWARE  0x01
#define SERIAL_TYPE_SOFTWARE  0x02
#define SERIAL_TYPE_HALF_DUPLEX 0x04

#include "pins_arduino.h"

#ifndef turnOffPWM
 void turnOffPWM(uint8_t timer);
#endif

void _pinMode(uint8_t, uint8_t); 
#ifndef pinMode

//  If the user calls pinMode(x, OUTPUT) where x is a compile time constant
//  then ideally we want to have that optimize to simply sbi [port] [bit]

//  This is the purpose of the static inlined pinMode definition.  
//  it checks to see that the pin and mode are constant and output and if so
//  then just sets the appropriate bit (it will optimize to an sbi)
//  Otherwise it will fall-back to using _pinMode() which will work for 
//  non-constant and for non-output.
//
//  For INPUT and INPUT_PULLUP we need to be able to disable and enable 
//  interrupts because both DDR and PORT registers need to be manipulated
//  atomically, this may wind up to be more costly than using 
//  _pinMode() even though we get to use the sbi/cbi to do it
//  if you are setting more than 2 or 3 pins to INPUT/INPUT_PULLUP
//  but that doesn't seem THAT likely (given that INPUT is the default state
//  anyway), but if you really need to, you can define OPTIMIZE_CONSTANT_PINMODE_INPUT 
//  (before including Arduino.h, or in your pins_arduino.h) 0 to disable that from happening.

#ifndef OPTIMIZE_CONSTANT_PINMODE_INPUT
  #define OPTIMIZE_CONSTANT_PINMODE_INPUT 1
#endif

static inline void pinMode(uint8_t , uint8_t ) __attribute__((always_inline, unused));
static inline void pinMode(uint8_t pin, uint8_t mode)
{
  // Pretty sure we can allow non-constant mode here without costing more flash than we
  //  would use in falling through to _pinMode()
  if(__builtin_constant_p(pin))// && __builtin_constant_p(mode))
  {
    // If we are passed a pin greater than 127 that means we got an analog pin number
    // which is 0b10000000 | [ADC_REF]
    // strip off the top bit to get the Reference and convert to digital pin number
    if( pin & 0b10000000 )
    {
      pin = analogInputToDigitalPin( pin & 0b01111111 );    
    }
     
    if(digitalPinToPort(pin) == NOT_A_PIN)
    {
      return;
      // NOP
    }
    else if(mode == OUTPUT)
    {
      ((void)(*((volatile uint8_t *)portModeRegister(digitalPinToPort(pin))) |= digitalPinToBitMask(pin)));
      return;
    }
    #if OPTIMIZE_CONSTANT_PINMODE_INPUT
    else if(mode == INPUT)
    {
      uint8_t oldSREG = SREG;
      cli();
      ((void)(*((volatile uint8_t *)portModeRegister(digitalPinToPort(pin)))   &= ~digitalPinToBitMask(pin)));
      ((void)(*((volatile uint8_t *)pullupEnableRegister(digitalPinToPort(pin))) &= ~digitalPinToBitMask(pin)));    
      SREG = oldSREG;
      return;
    }
    else if(mode == INPUT_PULLUP)
    {
      uint8_t oldSREG = SREG;
      cli();
      ((void)(*((volatile uint8_t *)portModeRegister(digitalPinToPort(pin)))   &= ~digitalPinToBitMask(pin)));      
      ((void)(*((volatile uint8_t *)pullupEnableRegister(digitalPinToPort(pin))) |= digitalPinToBitMask(pin)));    
      SREG = oldSREG;
      return;
    }
    #endif
  }

  _pinMode(pin,mode);  
}
#endif

void _digitalWrite(uint8_t, uint8_t);
#ifndef digitalWrite

//  If the user calls digitalWrite(x, y) where x is a compile time constant
//  then ideally we want to have that optimize to simply sbi [port] [bit]
//  or cbi [port] [bit]
//
//  This is the purpose of the static inlined digitalWrite definition.  
//  it checks to see that the pin and val are constant and if so
//  then just fiddles the appropriate bit (it will optimize to an sbi/cbi)
//  Otherwise it will fall-back to using _pinMode() which will work for 
//  non-constant and for non-output.
//
// For pins which have a timer attached, you can optionally choose not 
// to optimize that as it may be more space efficient not to, if you 
// do more than a couple of digitalWrite([constant],....) for such
// a timer attached pin.
//
// For non-constant pins, we fall through to the normal implementation 
// as there is a fair overhead involved and interrupts have to be disabled
// also.
#ifndef OPTIMIZE_DIGITAL_WRITE_ON_TIMER
  #define OPTIMIZE_DIGITAL_WRITE_ON_TIMER 1
#endif

static inline void digitalWrite(uint8_t , uint8_t ) __attribute__((always_inline, unused));
static inline void digitalWrite(uint8_t pin, uint8_t val)
{
  // We can allow non-constant val I think, the comparison overhead below will probably be no worse than calling _digitalWrite
  if(__builtin_constant_p(pin))// && __builtin_constant_p(val)) 
  {
    // If we are passed a pin greater than 127 that means we got an analog pin number
    // which is 0b10000000 | [ADC_REF]
    // strip off the top bit to get the Reference and convert to digital pin number
    if( pin & 0b10000000 ) 
    {
      pin = analogInputToDigitalPin( pin & 0b01111111 );    
    }
    
    if(digitalPinToPort(pin) == NOT_A_PIN)
    {
      return;
      // NOP
    }
    
    // If on a timer, turn it off (or fall through if we don't optimize that)
    if(digitalPinToTimer(pin) != NOT_ON_TIMER )
    {
      #if OPTIMIZE_DIGITAL_WRITE_ON_TIMER
        turnOffPWM(digitalPinToTimer(pin));
      #else
        _digitalWrite(pin,val);
        return;
      #endif
    }

    if(val == HIGH)
    {
      ((void)(*((volatile uint8_t *)portOutputRegister(digitalPinToPort(pin))) |= digitalPinToBitMask(pin)));
      return;
    }
    else // Can only be LOW here, no need to check
    {
      ((void)(*((volatile uint8_t *)portOutputRegister(digitalPinToPort(pin))) &= ~digitalPinToBitMask(pin)));
      return;
    }
  }

  _digitalWrite(pin,val);
}
#endif

void _analogWrite(uint8_t, uint8_t);
#ifndef analogWrite
// This is broken out to an inline only because we need to do pinMode() due the fact
// people don't use pinMode before analogWrite() owing to the fact that the Arduino
// docs say...
//
//  "You do not need to call pinMode() to set the pin as an output before 
//   calling analogWrite()."
//
// So every time you call analogWrite we have to do a pinMode for you, even if 
// we had already done it, or you had done it yourself.  For crying out loud.
static inline void analogWrite(uint8_t , uint8_t ) __attribute__((always_inline, unused));
static inline void analogWrite(uint8_t pin, uint8_t val)
{
  pinMode(pin, OUTPUT);
  _analogWrite(pin,val);
}
#endif


uint8_t _digitalRead(uint8_t);
#ifndef digitalRead

// This static inline allows us primarily to optimize away some checks and 
// code for constant pins.  it doesn't seem like much but in certain
// situations it can save you well over 50 bytes, the only time it might
// cost more to do it this way would be for pins which are on a timer
// and then only if you are doing a lot of distinct 
//   digitalRead([constnt pin on a timer])
// which seems really quite unlikely, anyway if it is a problem then you 
// can set OPTIMIZE_DIGITAL_READ_ON_TIMER 0

#ifndef OPTIMIZE_DIGITAL_READ_ON_TIMER
  #define OPTIMIZE_DIGITAL_READ_ON_TIMER 1
#endif

static inline uint8_t digitalRead(uint8_t ) __attribute__((always_inline, unused));
static inline uint8_t digitalRead(uint8_t pin)
{  
  if(__builtin_constant_p(pin))
  {
    // If we are passed a pin greater than 127 that means we got an analog pin number
    // which is 0b10000000 | [ADC_REF]
    // strip off the top bit to get the Reference and convert to digital pin number
    if( pin & 0b10000000 ) 
    {
      pin = analogInputToDigitalPin( pin & 0b01111111 );    
    }
    
    if(digitalPinToPort(pin) == NOT_A_PIN)
    {
      return LOW;
    }
    
    // If on a timer, turn it off (or fall through if we don't optimize that)
    if(digitalPinToTimer(pin) != NOT_ON_TIMER )
    {
      #if OPTIMIZE_DIGITAL_READ_ON_TIMER
        turnOffPWM(digitalPinToTimer(pin));
      #else
        return _digitalRead(pin);        
      #endif
    }

    return (*((volatile uint8_t *)portInputRegister(digitalPinToPort(pin))) & digitalPinToBitMask(pin)) ? HIGH : LOW;
  }

  return _digitalRead(pin);
}
#endif

uint16_t _analogRead(uint8_t pin);
#ifndef analogRead
// analogRead() is almost always going to be getting a constant pin
// (especially since we pretty much DEMAND that people pass Ax constants to it)
// so by declaring some of the pin specific setup of doing an analogRead here
// we can save some bytes when said constant pin doesn't need any setup
// I think that this will be a net reduction in code for most purposes
// and shouldn't have much effect otherwise.
// Especially because some of this pin specific stuff is just sanity checking!
static inline uint16_t analogRead(uint8_t ) __attribute__((always_inline, unused));
static inline uint16_t analogRead(uint8_t pin)
{
  // If we are passed a pin greater than 127 that means we got an analog pin number
  // which is 0b10000000 | [ADC_REF]
  // strip off the top bit to get the Reference
  pin = pin & 0b01111111;
  
  #if !defined(ADCSRA) || NUM_ANALOG_INPUTS < 1
  return digitalRead(analogInputToDigitalPin(pin)) ? 1023 : 0; //No ADC, so read as a digital pin instead.
  #endif
  
  return _analogRead(pin);
}
#endif

#ifndef USE_NEW_MILLIS
#ifndef NO_MILLIS
unsigned long millis(void);
unsigned long micros(void);
#endif
void delay(unsigned long);
void delayMicroseconds(unsigned int us);
#else
  #include "MillisMicrosDelay.h"
#endif

/*=============================================================================
 * We have different types of serial capability. 
 * 
 * SERIAL_TYPE_HARDWARE : Chip has hardware uart, use it.
 * SERIAL_TYPE_SOFTWARE : Use a software serial (TinySoftwareSerial) instead
 * SERIAL_TYPE_HALF_DUPLEX  : For very small size chips, use a half duplex
 *    bufferless implementation.  Note that this has a fixed-at-compile-time
 *    baud rate, which you can set by defining BAUD_RATE.
 * SERIAL_TYPE_NONE     : No serial at all.  Note that if you don't use the 
 *    `Serial` object that the compiler should optimise it away even if you 
 *    don't set SERIAL_TYPE_NONE, so it's probably not too useful.
 * 
 * The USE_SOFTWARE_SERIAL define (set in some variants) is deprecated,  
 *   better to set USE_SERIAL_TYPE to one of the values above specifically.
 * 
 * For SERIAL_TYPE_HALF_DUPLEX there are two additional defines you can make
 *   SERIAL_TYPE_HALF_DUPLEX_DISABLE_READ
 *   SERIAL_TYPE_HALF_DUPLEX_DISABLE_WRITE
 * which do as you expect and serve to reduce flash usage if you don't need
 * those functions (they are not optimised out if you don't use Serial
 * at all because they are declared as virtual by Stream)
 *===========================================================================*/
 
#if (!defined(USE_SERIAL_TYPE)) && defined(USE_SOFTWARE_SERIAL) && USE_SOFTWARE_SERIAL
  #define USE_SERIAL_TYPE       SERIAL_TYPE_SOFTWARE
#elif !defined(USE_SERIAL_TYPE)
  #define USE_SERIAL_TYPE       SERIAL_TYPE_HARDWARE  
#endif

#undef USE_SOFTWARE_SERIAL
#if USE_SERIAL_TYPE == SERIAL_TYPE_SOFTWARE
  #define USE_SOFTWARE_SERIAL 1
#else
  #define USE_SOFTWARE_SERIAL 0
#endif

/*=============================================================================
  Allow the ADC to be optional for low-power applications
=============================================================================*/

#ifndef TIMER_TO_USE_FOR_MILLIS
#define TIMER_TO_USE_FOR_MILLIS                     0
#endif
/*
  Tone goes on whichever timer was not used for millis.
*/
#if TIMER_TO_USE_FOR_MILLIS == 1
#define TIMER_TO_USE_FOR_TONE                     0
#else
#define TIMER_TO_USE_FOR_TONE                     1
#endif

#if NUM_ANALOG_INPUTS > 0
	#define HAVE_ADC    						  1
	#ifndef INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER 
		#define INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER   1
	#endif
#else
	#define HAVE_ADC 							  0
	#if defined(INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER)
		#undef INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER
	#endif
	#define INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER  0
#endif

#if !HAVE_ADC
  #undef INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER
  #define INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER  0
#else
  #ifndef INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER 
    #define INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER   1
  #endif
#endif

/*=============================================================================
  Allow the "secondary timers" to be optional for low-power applications
=============================================================================*/

#ifndef INITIALIZE_SECONDARY_TIMERS
  #define INITIALIZE_SECONDARY_TIMERS               1
#endif


#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus
#include "WCharacter.h"
#include "WString.h"

#if USE_SERIAL_TYPE    == SERIAL_TYPE_HARDWARE
  #include "HardwareSerial.h"
#elif USE_SERIAL_TYPE == SERIAL_TYPE_SOFTWARE
  #include "TinySoftwareSerial.h"
#elif USE_SERIAL_TYPE == SERIAL_TYPE_HALF_DUPLEX
  #include "HalfDuplexSerial.h"
#endif

uint16_t makeWord(uint16_t w);
uint16_t makeWord(byte h, byte l);

#define word(...) makeWord(__VA_ARGS__)

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L);

#if !defined(tone) && !defined(NO_TONE)
void initToneTimer(void);
void tone(uint8_t _pin, unsigned int frequency, unsigned long duration = 0);
void noTone(uint8_t _pin = 255);
#endif

// WMath prototypes
long random(long);
long random(long, long);
void randomSeed(unsigned int);
// map()
// {{{
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Pay attention, Multiplication and Division have the same operator 
// precedence, execution left to right!  Multiplication will happen before 
// division in the map() calculation, this means overflow is highly likely 
// for anything less than a uint32_t, using a smaller integer size will have
// bad results unfortunately.

uint32_t _map(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
static inline uint32_t map(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) __attribute__((always_inline, unused));
static inline uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max)
{
  if(__builtin_constant_p(in_min) && __builtin_constant_p(in_max) && 
     __builtin_constant_p(out_min) && __builtin_constant_p(out_max))
  {    
    // For a couple of very common cases we can cheat.
    // Note that these do not strictly adhere to the results "map()" would
    // otherwise give but the difference is minimal and in my opinion the
    // cheaper result is the more expected one anyway.
    if(in_min == 0 && out_min == 0 && in_max == 1023 && out_max == 255) 
    {
      // Just lose 2 bits
      return x >> 2;
    }
    else if(in_min == 0 && out_min == 0 && in_max == 255 && out_max == 1023) 
    {
      // Just add 2 bits
      return x << 2;
    }
    else if ( x <= in_min ) 
    {
      return out_min;
    }
    else if (x >= in_max )
    {
      return out_max;
    }

    // For other constant ranges, we inline these calculations so at least
    // some of it can happen at compile time to save us some bytes
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }

  // Otherwise just drop through to _map itself
  return _map(x, in_min, in_max, out_min, out_max);
}

// }}}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void     tiny_srandom(unsigned int);
unsigned int tiny_random();
#endif

/*=============================================================================
  Aliases for the interrupt service routine vector numbers so the code 
  doesn't have to be riddled with #ifdefs.
=============================================================================*/

#if defined( TIM0_COMPA_vect ) && ! defined( TIMER0_COMPA_vect )
#define TIMER0_COMPA_vect TIM0_COMPA_vect
#endif

#if defined( TIM0_COMPB_vect ) && ! defined( TIMER0_COMPB_vect )
#define TIMER0_COMPB_vect TIM0_COMPB_vect
#endif

#if defined( TIM0_OVF_vect ) && ! defined( TIMER0_OVF_vect )
#define TIMER0_OVF_vect TIM0_OVF_vect
#endif

#if defined( TIM1_COMPA_vect ) && ! defined( TIMER1_COMPA_vect )
#define TIMER1_COMPA_vect TIM1_COMPA_vect
#endif

#if defined( TIM1_COMPB_vect ) && ! defined( TIMER1_COMPB_vect )
#define TIMER1_COMPB_vect TIM1_COMPB_vect
#endif

#if defined( TIM1_OVF_vect ) && ! defined( TIMER1_OVF_vect )
#define TIMER1_OVF_vect TIM1_OVF_vect
#endif

#endif
