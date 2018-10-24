/*
  wiring_analog.c - analog input and output
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2005-2006 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  $Id: wiring.c 248 2007-02-03 15:36:30Z mellis $

  Modified  28-08-2009 for attiny84 R.Wiersma
  Modified  14-10-2009 for attiny45 Saposoft
  Corrected 17-05-2010 for ATtiny84 B.Cook
*/

#include "wiring_private.h"
#include "pins_arduino.h"

#ifndef DEFAULT
//For those with no ADC, need to define default.
#define DEFAULT (0)
#endif

uint8_t analog_reference = DEFAULT;

void analogReference(uint8_t mode)
{
  // can't actually set the register here because the default setting
  // will connect AVCC and the AREF pin, which would cause a short if
  // there's something connected to AREF.
  // fix? Validate the mode?
  analog_reference = mode;
}

#if defined(REFS1)
#define ADMUX_REFS_MASK (0x03)
#else
#define ADMUX_REFS_MASK (0x01)
#endif

#if defined(MUX5)
#define ADMUX_MUX_MASK (0x3f)
#elif defined(MUX4)
#define ADMUX_MUX_MASK (0x1f)
#elif defined(MUX3)
#define ADMUX_MUX_MASK (0x0f)
#else
#define ADMUX_MUX_MASK (0x07)
#endif

uint16_t _analogRead(uint8_t pin)
{
  // This has been stripped off already by analogRead() in Arduino.h
  // pin &=127; //strip off the high bit of the A# constants
  
#if ( defined(HAVE_ADC) && !HAVE_ADC ) || !defined(ADCSRA)
  // This is handled by analogRead() in Arduino.h
  // return digitalRead(analogInputToDigitalPin(pin)) ? 1023 : 0; //No ADC, so read as a digital pin instead.
  return 0;
#else
  #if defined(REFS0)
  #if defined(ADMUX)
  ADMUX = ((analog_reference & ADMUX_REFS_MASK) << REFS0) | ((pin & ADMUX_MUX_MASK) << MUX0); //select the channel and reference
  #if defined(REFS2)
  ADMUX |= (((analog_reference & 0x04) >> 2) << REFS2); //some have an extra reference bit in a weird position.
  #endif
  #endif
  #else
    // Chips without any other reference than Vcc
    ADMUX = pin;
  #endif
  
  #if defined(HAVE_ADC) && HAVE_ADC
  sbi(ADCSRA, ADSC); //Start conversion

  while(ADCSRA & (1<<ADSC)); //Wait for conversion to complete.

  uint8_t low = ADCL;
#if defined(ADCH)
  uint8_t high = ADCH;
  return (high << 8) | low;
#else
  // Some chips only have 8 bit ADC, because everything written mostly assumes 10 bit
  // we will multiply by 4 before we return it
  return low << 2;
#endif
  #else
  return LOW;
  #endif
#endif
}

// Right now, PWM output only works on the pins with
// hardware support.  These are defined in the appropriate
// pins_*.c file.  For the rest of the pins, we default
// to digital output.

// Arduino core has val as int, but explicitly sets 255 as HIGH,
// so what's the freaking point?!
//
// Good grief, they were even given a Pull Request that fixed it
//   https://github.com/arduino/Arduino/pull/4625
// and rejected because it breaks some obscure code somewhere 
// maybe possibly if somebody is using a function pointer and
// doesn't recompile.
//
// Seriously?!
//
// Well lucky for you I can do what I like, so you get to save
// some bytes in ATTinyCore

void _analogWrite(uint8_t pin, uint8_t val)
{
  // If we are passed a pin greater than 127 that means we got an analog pin number
  // which is 0b10000000 | [ADC_REF]
  // strip off the top bit to get the Reference and convert to digital pin number
  if( pin & 0b10000000 ) 
  {
    pin = analogInputToDigitalPin( pin & 0b01111111 );    
  }
      
  // We need to make sure the PWM output is enabled for those pins
  // that support it, as we turn it off when digitally reading or
  // writing with them.  Also, make sure the pin is in output mode
  // for consistenty with Wiring, which doesn't require a pinMode
  // call for the analog output pins.
  // This is now done from static inline void analogWrite() 
  // which is defined in Arduino.h, this allows us to use the
  // optimized pinMode() for constant pins.
  // pinMode(pin, OUTPUT);

#if ! (defined(ANALOG_WRITE_FLIPPED) && ANALOG_WRITE_FLIPPED)
  if (val <= 0)
  {
    digitalWrite(pin, LOW);
  }
  else if (val >= 255)
  {
    digitalWrite(pin, HIGH);
  }
  else
#else
  // If we flip (invert) the PWM, so that 255 is Off and 0 is On,
  // then we can do away with the special cases of val == 0
  // and val == 255 which in the normal directionality have to 
  // turn off the PWM and use digitalWrite.
  // 
  // We don't get fully "on" (max is about 99.5%), but that's less 
  // of a problem than not getting fully "off" especially when using
  // an LED which is probably the most common use case here.
  //
  // This saves us a bunch of space for something like the usual
  // led fader since digitalWrite() can be totally optimized out.
  //
  // Of course, we don't want the user to know to do this, so 
  // we take 0 = Off and 255 = On and flip that before passing
  // it to the AVR which is expecting the flipped value.
    
  val = 255-val;
#endif
  {
    
#ifdef turnOnPWM
    // Some variants (see tiny13) define a turnOnPWM macro which reduces code size
    turnOnPWM( digitalPinToTimer(pin), val );
#else   

    // All this defined() logic is silly trying to overly generalise wiring_analog.h
    // to cater for different variants; instead see variants/tiny13/pins_arduino.c for a 
    // better way of doing this for variants in future - in other words, you should ask
    // the variant to turn on pwm for itself, not try and figure out how to turn on pwm
    // for any given variant by looking at what is and isn't defined.  Crazyness.
    // 
    // In short, you should #define turnOnPWM(t, v) ( _turnOnPWM(t,v) )
    // in pins_arduino.h, and create pins_arduino.c to define the _turnOnPWM() function
    // (where t is the timer (eg TIMER0A) and v is the value 0-255)
		//
    // Do similar for turnOffPWM(t) by the way!

    uint8_t timer = digitalPinToTimer(pin);
	#if defined(TCCR0A) && defined(COM0A1)
	if( timer == TIMER0A){
		// connect pwm to pin on timer 0, channel A
		sbi(TCCR0A, COM0A1);
		cbi(TCCR0A, COM0A0);
		OCR0A = val; // set pwm duty
	} else
	#endif

	#if defined(TCCR0A) && defined(COM0B1)
	if( timer == TIMER0B){
		// connect pwm to pin on timer 0, channel B
		sbi(TCCR0A, COM0B1);
		cbi(TCCR0A, COM0B0);
		OCR0B = val; // set pwm duty
	} else
	#endif

	#if defined(TCCR1A) && defined(COM1A1) && !defined(TCCR1E)
	if( timer == TIMER1A){
		// connect pwm to pin on timer 1, channel A
		sbi(TCCR1A, COM1A1);
		cbi(TCCR1A, COM1A0);
	#ifdef OC1AX 
		cbi(TCCR1D, OC1AV);
		cbi(TCCR1D, OC1AU);
		cbi(TCCR1D, OC1AW);
		sbi(TCCR1D, OC1AX);
	#endif
		OCR1A = val; // set pwm duty
	} else
	#endif

	#if defined(TCCR1E)
	if( timer == TIMER1A){
		// connect pwm to pin on timer 1, channel A
		cbi(TCCR1C,COM1A1S);
		sbi(TCCR1C,COM1A0S);
		OCR1A = val; // set pwm duty
	} else if (timer == TIMER1B){
		// connect pwm to pin on timer 1, channel A
		cbi(TCCR1C,COM1B1S);
		sbi(TCCR1C,COM1B0S);
		OCR1B = val; // set pwm duty
	} else if (timer == TIMER1D){
		// connect pwm to pin on timer 1, channel A
		cbi(TCCR1C,COM1D1);
		sbi(TCCR1C,COM1D0);
		OCR1D = val; // set pwm duty
	} else
	#endif

	#if defined(TCCR1) && defined(COM1A1)
	if(timer == TIMER1A){
		// connect pwm to pin on timer 1, channel A
		sbi(TCCR1, COM1A1);
		cbi(TCCR1, COM1A0);
		OCR1A = val; // set pwm duty
	} else
	#endif

	#if defined(TCCR1A) && defined(COM1B1) && !defined(TCCR1E)
	if( timer == TIMER1B){
		// connect pwm to pin on timer 1, channel B
		sbi(TCCR1A, COM1B1);
		cbi(TCCR1A, COM1B0);
	#ifdef OC1BV
		sbi(TCCR1D, OC1BV);
		cbi(TCCR1D, OC1BU);
		cbi(TCCR1D, OC1BW);
		cbi(TCCR1D, OC1BX);
	#endif
		OCR1B = val; // set pwm duty
	} else
	#endif

	#if defined(TCCR1) && defined(COM1B1)
	if( timer == TIMER1B){
		// connect pwm to pin on timer 1, channel B
		sbi(GTCCR, COM1B1);
		cbi(GTCCR, COM1B0);
		OCR1B = val; // set pwm duty
	} else
	#endif
	
    {
      if (val < 128)
      {
        digitalWrite(pin, LOW);
      }
      else
      {
        digitalWrite(pin, HIGH);
      }
    }
#endif
  }
}
