/** Simplified Tone "Library"
 * 
 * The existing Tone.cpp in the core is a complete and utter mess, once again the
 * developers over the years have just used a load of arcane #ifdef's 
 * to try and work out what they should do for a specific chip.
 * 
 * Which pretty much makes it totally completely unreasonably unreadable.
 * 
 * So we simplify by replacing the existing Tone with our own, we are not 
 * composing a grapnd symphony here, let's face it, blurting out Mario Bros
 * is about the most people use tone() for!
 *  
 * @author James Sleeman <james@gogo.co.nz>
 * @license The MIT License (MIT) [See At End Of File]
 * 
 */

#include "Arduino.h"

#if !defined(NO_TONE)

static  volatile uint32_t  CurrentToneFrequency= 0;
static  volatile uint32_t  CurrentToneDuration = 0;
static  volatile uint32_t  CurrentToneStarted  = 0;
static  volatile uint8_t   CurrentTonePin      = 255;

void _tone(uint8_t pin, uint32_t frequency, uint32_t length) 
{
  // We will setup Timer1 (Timer0 is used for millis) 
  //  on the 2/4/85 Timer1 is 8 bit and does not have "Fast PWM"
  //  anyway, we will set it up so that the "mid point" of the 
  //  wave form (that is the transition from high-to-low)
  //  is the overflow point (top of counter), and we will 
  //  flip the pin every overflow.  So we will have the pin 
  //  high for X counts, the overflow happens and pin flips
  //  and then it is low for X counts.
  //
  //  Thus we need to toggle the pin the desired frequency
  //  times per second, we need that many overflows (or maybe
  //  I'm off by one, who cares, it's close enough).
  
  // So, we need to toggle the pin [frequency times per second]
  // ( F_CPU / Prescaler ) / Frequency --> number of clock cycles per toggle
  //  if the clock cycles is greater than 255 (only an 8 bit timer)
  //  which is the maximum clocks we can count  then we must increase 
  //  the prescaler to achieve it
  
  // First, check sanity, if freq comes in as zero that would be a div0
  // make it noTone() instead as sometimes it would be useful to 
  // do this (or at least it's something people might expect).
  if(frequency == 0) { noTone(pin); return; }
    
  CurrentToneDuration = length ? length : ~(0UL);
  
  // Are we already playing that tone, if so, just keep doing that
  // otherwise we would make a clicking sound.
  if(pin == CurrentTonePin && frequency == CurrentToneFrequency) return;
  
  CurrentToneFrequency = frequency;
  
  // Start with prescaling of 1, and if it doesn't fit, step up until we 
  // get one that does.
    
  uint16_t  prescaleDivider = 1; // The divider increases by powers of 2
  uint8_t   prescaleBitMask = 1; // The bitmask only increments unitarily

  while(((F_CPU / prescaleDivider) / frequency) > 255)
  {
    prescaleDivider = prescaleDivider << 1;
    prescaleBitMask++;
  }      
  
  // The official Arduino tone() sets it as output for you
  //  so we will also.
  if(pin != CurrentTonePin)
  {
    CurrentTonePin      = pin;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);    
  }
  
  // Set the comparison, we will flip the bit ever time this is hit
  OCR1C = ((F_CPU / prescaleDivider) / frequency);
  TCCR1 = 0b10000000 | prescaleBitMask;
  
  // Record when we started playing this tone
  CurrentToneStarted = millis();

  // And start playing it (enable the overflow interrupt)
  TIMSK |= _BV(TOIE1);
  
}

void _noTone(uint8_t pin) 
{
  // Disable the interrupt
  //  Note we can leave the rest of the timer setup as is, turnOnPWM() will
  //  fix it for itself next time you analogWrite() if you need to.
  TIMSK   &= ~_BV(TOIE1);
  
  // Pin goes back to input state  
  pinMode(pin == 255 ? CurrentTonePin : pin, INPUT);
  
  // And make sure we will reset it to output state next time you call
  // tone() by treating it as a new pin
  CurrentTonePin      = 255;
}

ISR(TIMER1_OVF_vect) 
{  
  // Toggle the pin, most AVR can toggle an output pin by writing a 1 to the input 
  // register bit for that pin.
  *(portInputRegister(digitalPinToPort(CurrentTonePin))) = digitalPinToBitMask(CurrentTonePin);
  
  // If we have played this tone for the requested duration, stop playing it.
  if (millis() - CurrentToneStarted >= CurrentToneDuration)
  {
    _noTone(); 
  }
}

#endif

/*
  The MIT License (MIT)

  Copyright (c) 2016 James Sleeman

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/