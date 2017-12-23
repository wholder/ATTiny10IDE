
//           +====+
//  PWMA/PB0 |*   | PB3 (RESET)
//       GND |    | Vcc
//  PWMB/PB1 |    | PB2 (CLKO)
//           +====+

#include "Arduino.h"

#define LED_PIN PB2

void setup () {
  // set PORTB for output
  pinMode(LED_PIN, OUTPUT);
}

void loop () {
  // Blink at 1 Hz Rate
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}
