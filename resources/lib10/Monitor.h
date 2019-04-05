#ifndef TINY10_MONITOR_H
#define TINY10_MONITOR_H

#include <inttypes.h>
#include <avr/pgmspace.h>
#include <avr/delay.h>

class __FlashStringHelper;
#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(PSTR(string_literal)))

class Monitor {
public:
    Monitor (uint8_t _monitorPin);
    void    print (const char msg[]);
    void    print (const __FlashStringHelper msg[]);

private:
    void    write (uint8_t data);
    uint8_t _pinMask;
};

#endif  /*ifndef TINY10_MONITOR_H */