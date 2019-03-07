#ifndef TINY10_ARDUINO_H
#define TINY10_ARDUINO_H

#if defined(__AVR_ATtiny10__) || defined(__AVR_ATtiny9__) || defined(__AVR_ATtiny4__) || defined(__AVR_ATtiny5__)

#include <avr/io.h>
#include <util/delay.h>

#define INPUT   0
#define OUTPUT  1

#define LOW     0
#define HIGH    1

enum byte : unsigned char {};

#if defined(__AVR_ATtiny10__) || defined(__AVR_ATtiny5__)
enum ANALOG_PIN : unsigned char {A0 = 0, A1 = 1, A2 = 2, A3 = 3};
unsigned char analogRead (unsigned char pin);
#endif

extern void setup();
extern void loop();

#define pinMode(PIN, MODE) (MODE ? (DDRB |= (1 << PIN)) : (DDRB &= ~(1 << PIN)))
#define digitalWrite(PIN, DIRECTION) (DIRECTION != 0 ? (PORTB |= (1 << PIN)) : (PORTB &= ~(1 << PIN)))
#define digitalRead(PIN) ((PINB & (1 << PIN)) != 0)

#define analogWrite(PIN, VALUE) (PIN == 0 ? pwm0(VALUE) : (PIN == 1 ? pwm1(VALUE) : dummy(PIN, VALUE)))

#define delay(MILLSECONDS) (_delay_ms(MILLSECONDS))
#define delayMicroseconds(MICROSECONDS) (_delay_us(MICROSECONDS))

void pwm0 (unsigned char value);
void pwm1 (unsigned char value);
void dummy (unsigned char pin, unsigned char value);

#else
#error undefined chip
#endif

#endif  /*ifndef TINY10_ARDUINO_H */