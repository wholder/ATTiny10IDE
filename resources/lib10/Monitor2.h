#ifndef TINY10_MONITOR_H
#define TINY10_MONITOR_H

#include <inttypes.h>
#include <avr/pgmspace.h>
#include <avr/delay.h>

class __FlashStringHelper;
#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(PSTR(string_literal)))

extern "C" void setMonitorPin (uint8_t monitorPin);
extern "C" void writeMonitor (uint8_t data);
extern "C" void printMonitor (char msg[]);
extern     void printMonitor (const __FlashStringHelper msg[]);

#endif  /*ifndef TINY10_MONITOR_H */