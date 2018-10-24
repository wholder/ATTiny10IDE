/*
  pins_arduino.h - Pin definition functions for Arduino
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2007 David A. Mellis

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

  $Id: wiring.h 249 2007-02-03 16:52:51Z mellis $
*/

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

// Initialisation Core Configuration
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  USE_WIRING_LITE will cause wiring_lite.c to be used instead of wiring.c
//    this is a simpler, better system, but means more code is needed 
//    for each variant (as it should be IMHO) so the variant can do the 
//    chip specific stuff.
//
//  USE_NEW_MILLIS  will cause MillisMicrosDelay.c/h to be used, this is only
//    possible with USE_WIRING_LITE, it produces better accuracy with less 
//    code, and includes the extra feature of the REAL_MILLIS() macro.
//
#define USE_WIRING_LITE  1
#define USE_NEW_MILLIS   1

// TODO: Make this automatic on analogRead()?
#ifndef INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER
  #define INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER    1
#endif

// Print Support
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// The print library is pretty heavy, we can elect to shrink a bit.
//
// PRINT_MAX_INT_TYPE determins the maximum integer that can be
//  printed.  PRINT_INT_TYPE_LONG; PRINT_INT_TYPE_INT; PRINT_INT_TYPE_BYTE
// 
// PRINT_USE_BASE_xxx defines if you can Print numbers in that base/
//
// Print.h defines BIN, OCT, DEC and HEX as the defaults
// we will leave all these commented out here, just including
// them in case you want to force-override
//
// You might use for example 
//    build.extra_flags=-DPRINT_USE_BASE_BIN -DPRINT_USE_BASE_DEC
// in your boards.txt

#define PRINT_MAX_INT_TYPE  PRINT_INT_TYPE_LONG
//#define PRINT_USE_BASE_BIN
//#define PRINT_USE_BASE_OCT
//#define PRINT_USE_BASE_DEC
//#define PRINT_USE_BASE_HEX
//#define PRINT_USE_BASE_ARBITRARY

// Tone Support
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// This will cause the core's default tone() [and anything supporting it]
// to go away and we will implement our own _tone() 
#if defined( __cplusplus ) && !defined( NO_TONE )
  // Note because we use default values for length and pin (as does the official
  // Arduino implementation) this only work in __cplusplus
  #define tone(...)   _tone(__VA_ARGS__)
  #define noTone(...) _noTone(__VA_ARGS__)
  void _tone(uint8_t pin, uint32_t frequency, uint32_t length = 0);
  void _noTone(uint8_t pin = 0);
#endif

// Serial Port Configuration
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  we have 3 options for serial, hardware, software and half_duplex 
//  hardware and software are self explanitory, half_duplex is 
//  a very small memory/flash usage routine without any buffering
//  or interrupts really intended for sending or receiving minimal stuff
//  for helping with debugging.  
//
//  For half_duplex we also have the option to disable the *standard*
//  read() and/or write() functions in order to save space, however
//  even if disabled, you can use some alternative functions such
//  as read_byte() and write_byte() which can be optimized out if 
//  not used.
//

// #define USE_SERIAL_TYPE           SERIAL_TYPE_HARDWARE
#define USE_SERIAL_TYPE           SERIAL_TYPE_SOFTWARE
// #define USE_SERIAL_TYPE              SERIAL_TYPE_HALF_DUPLEX
// #define HALF_DUPLEX_SERIAL_DISABLE_WRITE
// #define HALF_DUPLEX_SERIAL_DISABLE_READ


// The below are used for SERIAL_TYPE_SOFTWARE
// #define USE_SERIAL_TYPE SERIAL_TYPE_SOFTWARE
// TX is on AIN0, RX is on AIN1
#define ANALOG_COMP_DDR               DDRA
#define ANALOG_COMP_PORT              PORTA
#define ANALOG_COMP_PIN               PINA
#define ANALOG_COMP_AIN0_BIT            1
#define ANALOG_COMP_AIN1_BIT            2

// Analog reference bit masks.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define DEFAULT     (0)           // VCC
#define EXTERNAL    (1)           // External on AREF (PA0)
#define INTERNAL    (2)           // Internal (1v1)
#define INTERNAL1V1 (2)

// PWM On/Off
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define turnOnPWM(t,v)  ( _turnOnPWM(t,v) )
void _turnOnPWM(uint8_t t, uint8_t v);

#define turnOffPWM(t) ( _turnOffPWM(t) )
void _turnOffPWM(uint8_t t);


// millis()/micros() Timer On/Off
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ( Applicable only with  USE_WIRING_LITE )
// If the macros are not defined, a default implementation may be implemented
// if the macros are defined, then they will be used 
//
// Your macro may call a function which you can implement in pins_arduino.c
// or as I have done here for the t13, just do it directly in the macro to
// save some bytes of flash.  This "function" is only called once normally and
// not by users.

#define turnOnMillis(prescale)  ( _turnOnMillis(prescale)  )
#define turnOffMillis()         ( _turnOffMillis() )
void _turnOnMillis(uint8_t prescale);
void _turnOffMillis();

// Arduino Pin Numbering to Chip's PORT.PIN and ADC Numbers
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// See PinMapping.jpg

#define NUM_DIGITAL_PINS            11
#define NUM_ANALOG_INPUTS           8

#define analogInputToDigitalPin(p)  (p)

#define digitalPinHasPWM(p)         ((p) == 5 || (p) == 6 || (p) == 7 || (p) == 8)

// These are some convenient NAME => Arduino Digital Pin Number mappings
//
//  MOSI/MISO : These are when using SPI as a master
//              NOT when programming which are inverted and what the 
//              datasheet shows.  
//

#define SS   ((uint8_t) 3)
#define MOSI ((uint8_t) 5)
#define MISO ((uint8_t) 6)
#define SCK  ((uint8_t) 4)
#define SDA  ((uint8_t) 6)
#define SCL  ((uint8_t) 4)

#define USI_DDR_PORT DDRA
#define USI_SCK_PORT DDRA
#define USCK_DD_PIN DDA4
#define DO_DD_PIN DDA5
#define DI_DD_PIN DDA6
#  define DDR_USI DDRA
#  define PORT_USI PORTA
#  define PIN_USI PINA
#  define PORT_USI_SDA PORTA6
#  define PORT_USI_SCL PORTA4
#  define PIN_USI_SDA PINA6
#  define PIN_USI_SCL PINA4
#  define USI_START_VECTOR USI_START_vect
#  define USI_OVERFLOW_VECTOR USI_OVF_vect
#  define DDR_USI_CL DDR_USI
#  define PORT_USI_CL PORT_USI
#  define PIN_USI_CL PIN_USI
#ifndef USI_START_COND_INT
#  define USI_START_COND_INT USISIF
#endif

// Analog Pins are set to ( 128 | [ADC] )
// so that they can be identified by pinMode, digitalRead/Write and analogRead/Write
static const uint8_t A0 = 0x80 | 0;
static const uint8_t A1 = 0x80 | 1;
static const uint8_t A2 = 0x80 | 2;
static const uint8_t A3 = 0x80 | 3;
static const uint8_t A4 = 0x80 | 4;
static const uint8_t A5 = 0x80 | 5;
static const uint8_t A6 = 0x80 | 6;
static const uint8_t A7 = 0x80 | 7;

// Pin Change Interrupt (PCI) Setup
#define digitalPinToPCICR(p)    (((p) <= 11) ? (&GIMSK) : ((uint8_t *)NULL))
#define digitalPinToPCICRbit(p) (((p) < 8 ) ? (PCIE0) : (PCIE1))
#define digitalPinToPCMSK(p)    (((p) < 8)  ? (&PCMSK0) : (&PCMSK1))
#define digitalPinToPCMSKbit(p) ( \
  ( (p < 8)   ? (p)    :\
  ( (p == 8)  ? (10-8) :\
  ( (p == 10) ? (8-8)  :\
  ( (p-8) ))))          \
)

// The x4 has 2 ports
//  we have to start from 1 because NOT_A_PIN and NOT_A_PORT are defined as 0 in Arduino.h
//  annoyingly.
#define PA 1
#define PB 2

// Just to make sure we do not have these defined since these ports 
// do not exist on the x4
#undef PC
#undef PD

// Instead of the usual "arduino way" of using lookup tables stored in 
// progmem, we will just use some macros.  Notice that little care is 
// taken with regard to input validity, if you request an invalid
// pin number, it's invalid, you'll get garbage at best.
#undef  digitalPinToPort
#define digitalPinToPort(P) ( (P<8) ? PA : PB  )

#undef  digitalPinToBitMask
#define digitalPinToBitMask(P) ( \
  ( (P  < 8 ) ? (_BV(P)) :       \
  ( (P == 8 ) ? (_BV(10-8)) :    \
  ( (P == 10) ? (_BV(8-8)) :     \
  ( _BV(P-8) ))))                \
)

#undef digitalPinToTimer
#define digitalPinToTimer(P) ( \
  ( (P == 5) ? TIMER1B  :      \
  ( (P == 6) ? TIMER1A  :      \
  ( (P == 7) ? TIMER0B  :      \
  ( (P == 8) ? TIMER0A  :      \
  ( ( NOT_ON_TIMER )    )))))  \
)

#undef portOutputRegister
#define portOutputRegister(P) ( (P==PA) ? (volatile uint8_t *)(&PORTA) :  (volatile uint8_t *)(&PORTB) )

#undef portInputRegister
#define portInputRegister(P)  ( (P==PA) ? (volatile uint8_t *)(&PINA)  :  (volatile uint8_t *)(&PINB)  )

#undef portModeRegister
#define portModeRegister(P)   ( (P==PA) ? (volatile uint8_t *)(&DDRA)  :  (volatile uint8_t *)(&DDRB)  )

#endif
