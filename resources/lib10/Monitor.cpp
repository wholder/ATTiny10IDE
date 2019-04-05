
#if defined(__AVR_ATtiny10__) || defined(__AVR_ATtiny9__) || defined(__AVR_ATtiny4__) || defined(__AVR_ATtiny5__)

#include "Monitor.h"

Monitor::Monitor (uint8_t monitorPin) {
    _pinMask = 1 << monitorPin;
    DDRB |= _pinMask;
}

void Monitor::print (const char msg[]) {
    write(0xAA);                        // SYNC
    write(0xAB);                        // START
    uint8_t tmp = 0;
    char* ptr1 = msg;
    while (*ptr1++ != 0) {
        tmp++;
    }
    write(tmp);                         // Length of data
    tmp = 0;
    char cc;
    while ((cc = *msg++) != 0) {
        write(cc);                      // Data bytes
        tmp += cc;
    }
    write(tmp);                         // Checksum
    write(0x00);                        // Pad
}

void Monitor::print (const __FlashStringHelper msg[]) {
    PGM_P ptr = reinterpret_cast<PGM_P>(msg);
    print(ptr);
}

void Monitor::write (uint8_t data) {
    for (uint8_t ii = 0; ii < 8; ii++) {
        PORTB ^= _pinMask;
        if ((data & 0x80) != 0) {
            _delay_ms(2);               // One bit is 2 ms
        } else {
            _delay_ms(1);               // Zero bit is 1 ms
        }
        data <<= 1;
    }
}

#else
#error undefined chip
#endif
