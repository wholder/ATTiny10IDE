
#if (USE_SERIAL_TYPE == SERIAL_TYPE_SOFTWARE)
#ifndef TinySoftwareSerial_h
#define TinySoftwareSerial_h
#include <inttypes.h>
#include "Stream.h"

#if !defined(ACSR) && defined(ACSRA)
#define ACSR ACSRA
#endif

#if (RAMEND < 250)
  #define SERIAL_BUFFER_SIZE 8
#elif (RAMEND < 500)
  #define SERIAL_BUFFER_SIZE 16
#elif (RAMEND < 1000)
  #define SERIAL_BUFFER_SIZE 32
#else
  #define SERIAL_BUFFER_SIZE 128
#endif
struct soft_ring_buffer
{
  unsigned char buffer[SERIAL_BUFFER_SIZE];
  int head;
  int tail;
};

// When LTO is enabled, the linker drops uartDelay out because
// it doesn't look used, then it complains that it actually did
// need it after all (if it indeed did), adding used solves this
// and doesn't *appear* to cause any problems... probably.
extern "C"{
  void uartDelay() __attribute__ ((naked,used));
  uint8_t getch();
  void store_char(unsigned char c, soft_ring_buffer *buffer);
}

class TinySoftwareSerial : public Stream
{
  public: //should be private but needed by extern "C" {} functions.
	uint8_t _rxmask;
	uint8_t _txmask;
	uint8_t _txunmask;
	soft_ring_buffer *_rx_buffer;
	uint8_t _delayCount;
  public:
    TinySoftwareSerial(soft_ring_buffer *rx_buffer, uint8_t txBit, uint8_t rxBit);
    void begin(long);
    void end();
    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    virtual void flush(void);
    virtual size_t write(uint8_t);
    using Print::write; // pull in write(str) and write(buf, size) from Print
    operator bool();
};

extern TinySoftwareSerial Serial;

//extern void putch(uint8_t);
#endif
#endif
