
#if defined(__AVR_ATtiny10__) || defined(__AVR_ATtiny9__) || defined(__AVR_ATtiny4__) || defined(__AVR_ATtiny5__)

#include "Monitor2.h"

uint8_t _pinMask;

void setMonitorPin (uint8_t monitorPin) {
    _pinMask = 1 << monitorPin;
    DDRB |= _pinMask;
}

void printMonitor (char msg[]) {
    writeMonitor(0xAA);                 // SYNC
    writeMonitor(0xAB);                 // START
    uint8_t tmp = 0;
    char* ptr1 = msg;
    while (*ptr1++ != 0) {
        tmp++;
    }
    writeMonitor(tmp);                  // Length of data
    tmp = 0;
    char cc;
    while ((cc = *msg++) != 0) {
        writeMonitor(cc);               // Data bytes
        tmp += cc;
    }
    writeMonitor(tmp);                  // Checksum
    writeMonitor(0x00);                 // Pad
}

void printMonitor (const __FlashStringHelper msg[]) {
    PGM_P ptr = reinterpret_cast<PGM_P>(msg);
    printMonitor(ptr);
}

void writeMonitor (uint8_t data) {
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
