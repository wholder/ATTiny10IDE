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
  //  we will set it up so that the "mid point" of the 
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
  //  if the clock cycles is greater than 65535 (16 bit timer)
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
  // get one that does, or run out of options.
    
  uint16_t  prescaleDivider = 1; // The divider increases by powers of 2
  uint8_t   prescaleBitMask = 1; // The bitmask only increments unitarily

  while(((F_CPU / prescaleDivider) / frequency) > 65535)
  {    
    switch(prescaleDivider)
    {
      case 1:
        prescaleDivider  = 8;
        break;
        
      case 8:
        prescaleDivider  = 64;
        break;
        
      case 64:
        prescaleDivider  = 256;
        break;
        
      case 256:
        prescaleDivider  = 1024;
        break;
        
      default:
        // Unachievable
        return;          
    }    
    prescaleBitMask++;
  }      
  
  // The official Arduino tone() sets it as output for you
  //  so we will also.
  if(pin != CurrentTonePin)
  {
    CurrentTonePin      = pin;
    pinMode(pin, OUTPUT);    
  }
    
  // Shut down interrupts while we fiddle about with the timer.
  cli();
  
  TCCR1B &= ~0b111; // Turn off the timer before changing anytning
  TCNT1   = 0;      // Timer counter back to zero
  
  // Set the comparison, we will flip the bit every time this is hit      
  //  (actually, this will set TOP of the timer and we flip on the overflow) 
  OCR1A = ((F_CPU / prescaleDivider) / frequency);
  
  // Enable the overflow interrupt
  TIMSK1 |= _BV(TOIE1);
  
  // Start playing and record time
  digitalWrite(pin, HIGH);   
  CurrentToneStarted = millis();  
  
  // Start the timer    
  TCCR1A = 0b00000011;  
  TCCR1B = 0b00011000 | prescaleBitMask;
  
  sei(); // We **have** to enable interrupts here even if they were disabled coming in,
         //  otherwise it's not going to do anything.  Hence not saving/restoring SREG.
}

void _noTone(uint8_t pin) 
{
  // Disable the interrupt
  //  Note we can leave the rest of the timer setup as is, turnOnPWM() will
  //  fix it for itself next time you analogWrite() if you need to.
  TIMSK1   &= ~_BV(TOIE1);
  
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