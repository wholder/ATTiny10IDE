#ifndef ARDUINO_H
#define ARDUINO_H

#include <avr/io.h>
#include <util/delay.h>

#define INPUT   0
#define OUTPUT  1

#define LOW     0
#define HIGH    1

enum byte : unsigned char {};

enum CLKSP : unsigned char {
  CLK_8000000 = 0,
  CLK_4000000 = 1,
  CLK_2000000 = 2,
  CLK_1000000 = 3,
  CLK_500000  = 4,
  CLK_250000  = 5,
  CLK_125000  = 6,
  CLK_62500   = 7,
  CLK_31250   = 8
};

enum ANALOG_PIN : unsigned char {A0 = 0, A1 = 1, A2 = 2, A3 = 3};

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
unsigned char analogRead (unsigned char pin);
void dummy (unsigned char pin, unsigned char value);
void clockSpeed (CLKSP rate);

#endif  /*ifndef ARDUINO_H */