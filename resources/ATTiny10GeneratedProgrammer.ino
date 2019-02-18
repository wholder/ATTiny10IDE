#include <avr/io.h>

/*
 * ATtiny10 TPI Generated Programmer for IDE and non High Voltage Programmer
 * IMPORTANT: Not Compatible with original Emulator/Programmer board
 */
 
//                 +-\/-+
//  TPIDAT / PB0 1 |    | 6 PB3 / RESET
//           Gnd 2 |    | 5 Vcc
//  TPICLK / PB1 3 |    | 4 PB2
//                 +----+

const unsigned char program[] PROGMEM  = { /*[CODE]*/ };
const char          progName[] = "/*[NAME]*/";
unsigned int        progSize;
unsigned char       flashMem[1024];
unsigned char       fuse = 0x0/*[FUSE]*/;
boolean             hvMode = false;

#define  BIT_TIME  1
// Definitons for 5V Programmer
#define  VCC        2   // Pin 5 on ATtiny10
#define  TPICLK     3   // Pin 3 on ATtiny10
#define  TPIDAT     4   // Pin 1 on ATtiny10
#define  RESET      5   // Pin 6 on ATtiny10
#define  GND        6   // Pin 2 on ATtiny10
// Definitons for 12V Programmer (just different names for pins)
#define  SHDN       5   // 0 to turn on 12V, 1 to disable
#define  RELAY      6   // 1 to program, 0 to emulate
#define  HV_DETECT  A0  // Is LOW if HV Programmer (version 2)

// Define IO Registers
#define  NVMCMD  0x33
#define  NVMCSR  0x32

// Define NVM Commands
#define  NO_OPERATION  0x00
#define  CHIP_ERASE    0x10
#define  SECTION_ERASE 0x14
#define  WORD_WRITE    0x1D

void setup () {
  // Check for HV Progammer (HV_DETECT pin is LOW)
  pinMode(HV_DETECT, INPUT_PULLUP);
  delay(10);
  hvMode = digitalRead(HV_DETECT) == LOW;
  // Set flashMem to unprogrammed value and then copy program code into it
  memset(flashMem, 0xFF, sizeof(flashMem));
  memcpy_P(flashMem, program, sizeof(program));
  progSize = sizeof(program);
  Serial.begin(115200);
  disablePins();
  if (hvMode) {
    pinMode(SHDN, OUTPUT);        // Connect SHDN
    digitalWrite(SHDN, HIGH);     // 12v off
    pinMode(RELAY, OUTPUT);       // Connect RELAY
    digitalWrite(RELAY, LOW);     // Chip in Emulate mode (Relay Off)
    Serial.print("\x06\x06");     // Send ACK ACK sequence tp signal ready
  } else {
    pinMode(GND, OUTPUT);         // Connect GND
    digitalWrite(GND, LOW);       // GND Always Low
    printInstructions();
  }
}

void enablePins () {
  pinMode(VCC, OUTPUT);           // Connect VCC
  digitalWrite(VCC, LOW);         // VCC off
  pinMode(TPICLK, OUTPUT);        // Connect TPICLK
  digitalWrite(TPICLK, HIGH);     // Clk high
  pinMode(TPIDAT, INPUT);         // Connect TPIDAT
  digitalWrite(TPIDAT, HIGH);     // Enable pullup
  delay(10);
  if (hvMode) {
    digitalWrite(RELAY, HIGH);    // Chip in Program mode (Relay On)
  } else {
    pinMode(RESET, OUTPUT);       // Connect RESET
    digitalWrite(RESET, HIGH);    // RESET Off
  }
  delay(200);
}

void disablePins () {
  pinMode(VCC, INPUT);          // Disconnect VCC
  pinMode(TPICLK, INPUT);       // Disconnect TPICLK
  pinMode(TPIDAT, INPUT);       // Disconnect TPIDAT
  digitalWrite(VCC, LOW);       // No pull up
  digitalWrite(TPICLK, LOW);    // No pull up
  digitalWrite(TPIDAT, LOW);    // No pull up
  if (hvMode) {
    digitalWrite(RELAY, LOW);   // Chip in Emulate mode
  } else {
    pinMode(RESET, INPUT);      // Disconnect RESET
    digitalWrite(RESET, LOW);   // No pull up
  }
  delay(200);
}

void pulseClock () {
  digitalWrite(TPICLK, LOW);
  delayMicroseconds(BIT_TIME);
  digitalWrite(TPICLK, HIGH);
  delayMicroseconds(BIT_TIME);
}

void sendFrame (unsigned char data) {
  unsigned char tmp = data;
  // Write start bit
  pinMode(TPIDAT, OUTPUT);
  digitalWrite(TPIDAT, LOW);
  pulseClock();
  unsigned char check = 0;
  // Write data bits
  for (unsigned char ii = 0; ii < 8; ii++) {
    digitalWrite(TPIDAT, tmp & 1);
    check ^= tmp & 1;
    tmp >>= 1;
    pulseClock();
  }
  // Write check bit
  digitalWrite(TPIDAT, check);
  pulseClock();
  // Write stop bits
  digitalWrite(TPIDAT, HIGH);
  pulseClock();
  pinMode(TPIDAT, INPUT);
  pulseClock();
}

unsigned char readBit () {
  digitalWrite(TPICLK, LOW);
  delayMicroseconds(BIT_TIME);
  unsigned char data = digitalRead(TPIDAT) ? 1 : 0;
  digitalWrite(TPICLK, HIGH);
  delayMicroseconds(BIT_TIME);
  return data;
}

unsigned char readFrame () {
  pinMode(TPIDAT, INPUT);
  for (int ii = 0; ii < 256; ii++) {
    if (readBit() == 0) {
      // Start bit found
      unsigned char data = 0;
      unsigned char check = 0;
      for (unsigned char jj = 0; jj < 8; jj++) {
        unsigned char tmp = readBit();
        data >>= 1;
        data |= tmp != 0 ? 0x80 : 0x00;
        check ^= tmp & 1;
      }
      unsigned char tmp = readBit();
      readBit();
      readBit();
      if (tmp == check) {
        return data;
      }
      return 0xFF;
    }
  }
  Serial.println("Read timeout");
  // Timeout, or error
  return 0xFF;
}

unsigned char enterProgMode () {
  sendFrame(0xE0);              // SKEY Opcode
  sendFrame(0xFF);              // Send 8 Byte Key
  sendFrame(0x88);
  sendFrame(0xD8);
  sendFrame(0xCD);
  sendFrame(0x45);
  sendFrame(0xAB);
  sendFrame(0x89);
  sendFrame(0x12);
  for (unsigned char ii = 0; ii < 100; ii++) {
    if ((readCtlSpace(0x00) & 0x02) != 0) {
      return 1;  // enabled
    }
  }
  Serial.println("Unable to select program mode");
  return 0;      // timeout
}

void setPointerReg (unsigned int add) {
  sendFrame(0x68);          // SSTPR 0
  sendFrame(add & 0xFF);
  sendFrame(0x69);          // SSTPR 1
  sendFrame(add >> 8);
}

unsigned char readAndInc () {
  sendFrame(0x24);          // SLD++
  return readFrame();
}

void writeAndInc (unsigned char data) {
  sendFrame(0x64);          // SST++
  sendFrame(data);
}

unsigned char readIoSpace (unsigned char add) {
  sendFrame(0x10 | ((add & 0x30) << 1) | (add & 0x0F));  // SIN
  return readFrame();
}

void writeIoSpace(unsigned char add, unsigned char data) {
  sendFrame(0x90 | ((add & 0x30) << 1) | (add & 0x0F));  // SOUT
  sendFrame(data);
}

unsigned char readCtlSpace (unsigned char add) {
  sendFrame(0x80 | (add & 0x0F));   // SLDCS
  return readFrame();
}

void nvmWait () {
  while ((readIoSpace(0x32) & 0x80) != 0) {
    delay(BIT_TIME);    // Arbitrary delay
  }
}

boolean powerOn () {
  enablePins();
  digitalWrite(VCC, HIGH);        // VCC on
  delay(128);                     // Wait 128 ms
  if (hvMode) {
    digitalWrite(SHDN, LOW);      // 12v On
  } else {
    digitalWrite(RESET, LOW);     // RESET On
  }
  delay(10);                      // Wait 10 ms
  digitalWrite(TPIDAT, HIGH);     // Data high
  pinMode(TPIDAT, OUTPUT);
  for (unsigned char ii = 0; ii < 32; ii++) {
    pulseClock();
  }
  return true;
}

void powerOff () {
  if (hvMode) {
    digitalWrite(SHDN, HIGH);     // 12v Off
  } else {
    digitalWrite(RESET, HIGH);    // RESET Off
  }
  delay(128);                     // Wait 128 ms
  digitalWrite(VCC, LOW);         // VCC off
  disablePins();
}

/*
 *  0x1E, 0x8F, 0x0A  ATtiny4
 *  0x1E, 0x8F, 0x09  ATtiny5
 *  0x1E, 0x90, 0x08  ATtiny9
 *  0x1E, 0x90, 0x03  ATtiny10
 */
boolean printSignature () {
  if (powerOn()) {
    if (enterProgMode()) {
      // Read device signature
      Serial.println();
      Serial.print("Device: ");
      setPointerReg(0x3FC0);        // Device Id Bytes
      unsigned char sig[3];
      for (unsigned char ii = 0; ii < 3; ii++) {
        if (ii > 0) {
          Serial.print(", ");
        }
        printHex(sig[ii] = readAndInc());
      }
      if (sig[0] == 0x1E) {
        if (sig[1] == 0x8f) {
          if (sig[2] == 0x0A) {
            Serial.println(" - ATTiny4");
          } else if (sig[2] == 0x09) {
            Serial.println(" - ATTiny5");
          }
        } else if (sig[1] == 0x90) {
          if (sig[2] == 0x08) {
            Serial.println(" - ATTiny9");
          } else if (sig[2] == 0x03) {
            Serial.println(" - ATTiny10");
          }
        }
      }
      Serial.println();
    }
    powerOff();
    return true;
  } else {
    powerOff();
    return false;
  }
}

// Read 1 second half cycle signal on TPIDAT Input
void measureClock () {
  enablePins();
  digitalWrite(VCC, HIGH);      // VCC on
  delay(200);
  boolean state, lState = digitalRead(TPIDAT);
  unsigned char hi = 0xFF, lo = 0x00, mid = 0x80, sel;
  long lowest = 1000000;
  // Verify TPIDAT signal is changing before measuring time interval
  unsigned long timeout = millis();
  Serial.print("Measuring Clock.");
  do {
    // Must see TPIDAT change within 2 seconds, or timeout
    if ((millis() - timeout) > 2000) {
      Serial.println("Timeout");
      powerOff();
      return;
    }
    state = digitalRead(TPIDAT);
  } while (state == lState);
  do {
    setClockTweak(mid);
    delay(1000);
    do {
      state = digitalRead(TPIDAT);
    } while (state == lState);
    long sTime = micros();
    lState = state;
    do {
      state = digitalRead(TPIDAT);
    } while (state == lState);
    long eTime = micros();
    lState = state;
    long diff = eTime - sTime;
    Serial.print(".");
    if (diff < 1000000) {
      hi = mid;
    } else {
      lo = mid;
    }
    if (diff < lowest) {
      lowest = diff;
      sel = mid;
    }
    mid = (hi + lo) / 2;
  } while (abs(hi - lo) > 1);
  Serial.print("\nRecommended Clock Offset: 0x");
  Serial.println(mid, HEX);
  digitalWrite(VCC, LOW);       // VCC off
  disablePins();
}

void setClockTweak (unsigned char val) {
  digitalWrite(TPICLK, LOW);        // Short Pulse
  delayMicroseconds(1000);
  digitalWrite(TPICLK, HIGH);
  delay(1);
  for (int ii = 0; ii < 8; ii++) {
    int width = (val & 0x80) != 0 ? 80 : 20;
    val = val << 1;
    digitalWrite(TPICLK, LOW);      // Short Pulse
    delayMicroseconds(width);
    digitalWrite(TPICLK, HIGH);
    delay(1);
  }
}

void printFuses () {
  if (powerOn()) {
    if (enterProgMode()) {
      // Read device signature
      Serial.print("Fuses:  ");
      setPointerReg(0x3F40);        // Configuration Byte
      printHex(readAndInc());
      Serial.println();
    }
  }
  powerOff();
}

void printHex (unsigned char val) {
  Serial.print("0x");
  if (val < 16) {
    Serial.print("0");
  }
  Serial.print(val, HEX);
}

void writeFlash (unsigned char fuseByte) {
  if (powerOn()) {
    if (enterProgMode()) {
      Serial.print("\nProgramming");
      // Erase all flash bytes
      writeIoSpace(NVMCMD, CHIP_ERASE);
      setPointerReg(0x4001);
      writeAndInc(0x00);    // Write dummy high byte
      nvmWait();
      writeIoSpace(NVMCMD, NO_OPERATION);
      nvmWait();
      // Erase Config Section
      writeIoSpace(NVMCMD, SECTION_ERASE);
      setPointerReg(0x3F41);
      writeAndInc(0x00);    // Write dummy high byte
      nvmWait();
      writeIoSpace(NVMCMD, NO_OPERATION);
      nvmWait();
      // Write config byte (Note: bits are inverted, so '0' value is ON)
      writeIoSpace(NVMCMD,WORD_WRITE);
      setPointerReg(0x3F40);
      writeAndInc(fuseByte | 0xF8);    // Config Byte
      writeAndInc(0xFF);
      writeIoSpace(NVMCMD, NO_OPERATION);
      nvmWait();
      // Write program to flash memory
      writeIoSpace(NVMCMD,WORD_WRITE);
      setPointerReg(0x4000);
      for (int ii = 0; ii < progSize; ii += 2) {
        writeAndInc(flashMem[ii]);      // Low byte
        writeAndInc(flashMem[ii + 1]);  // High byte
        nvmWait();
        if ((ii & 0x0F) == 0) {
          Serial.print(".");
        }
      }
      writeIoSpace(NVMCMD, NO_OPERATION);
      nvmWait();
      Serial.print("Done - ");
      Serial.print(progSize, DEC);
      Serial.println(" bytes written");
    }
  }
  powerOff();
}

unsigned int newDigit (unsigned int cVal, unsigned char digit) {
  cVal <<= 4;
  if (digit >= 'A'  &&  digit <= 'F') {
    cVal += digit - 'A' + 10;
  } else {
    cVal += digit - '0';
  }
  return cVal;
}

  // Implements a state machine that handles a HEX download from the IDE
void download () {
  unsigned char state = 0;
  unsigned char len, type, data, check, fuse;
  unsigned int  add;
  unsigned long timeout = millis();;
  progSize = 0;
  while (true) {
    if (Serial.available() > 0) {
      unsigned char cc = Serial.read();
      timeout = millis();
      if (cc == ':') {                    // Start of HexAscii line
        state = 1;
        len = 0;
        check = 0;
        add = 0;
      } else if (cc == '*') {             // Start of Fuse line
        fuse = 0xFF;
        state = 20;                       // goto Read Fuse state
      } else {
        switch (state) {
        case 1:                           // Length MSD
        case 2:                           // Length LSD
          len = newDigit(len, cc);
          state++;
          break;
        case 3:                           // Address MSD
        case 4:
        case 5:
        case 6:                           // Address LSD
          add = newDigit(add, cc);
          state++;
          break;
        case 7:                           // Type MSD
          type = newDigit(0, cc);
          state++;
          break;
        case 8:                           // Type LSD
          type = newDigit(type, cc);
          if (len > 0) {
            state++;
          } else {
            state = 11;
          }
          break;
        case 9:                           // Data byte MSD
          data = newDigit(0, cc);
          state++;
          break;
        case 10:                          // Data byte LSD
          data = newDigit(data, cc);
          if (type == 0) {
            flashMem[add++] = data;
            progSize++;
          }
          if (--len > 0) {
            state = 9;                    // Get another data byte
          } else {
            state = 11;                   // Get checksum
          }
          break;
        case 11:                          // Checksum MSD
        case 12:                          // Checksum LSD
          check = newDigit(check, cc);
          state++;
          break;
        case 13:                          // Wait for CR/LF
          if (cc == 0x0A  ||  cc == 0x0D) {
            switch (type) {
            case 0:                       // Data record
              Serial.print(".");
              break;
            case 1:                       // EOF Record
              writeFlash(fuse);
             return;
            case 2:                       // Start of Record (actually extended segment address record)
              // Set buffer space and fuse byte to unprogrammed flash value (0xFF)
              memset(flashMem, 0xFF, sizeof(flashMem));
              fuse = 0xFF;
              break;
            }
            state = 0;
          }
          break;
        case 20:                          // Read Fuse LS Byte
          fuse = newDigit(fuse, cc);
          state = 0;
          break;
        }
      }
    } else {
      // Check for 2 second timeout;
      if ((millis() - timeout) > 2000) {
        Serial.println(".Timeout");
        return;
      }
    }
  }
}

void printInstructions () {
  int ardPins[] = {2, 3, 4, 5, 6};
  int tnyPins[] = {5, 3, 1, 6, 2};
  if (sizeof(program) > 0) {
    Serial.print("Programmer for: ");
    Serial.println(progName);
  }
  Serial.println("Connect:");
  for (int ii = 0; ii < 5; ii++) {
    Serial.print("  Arduino pin D");
    Serial.print(ardPins[ii], DEC);
    Serial.print(" to ATtiny10 pin ");
    Serial.println(tnyPins[ii]);
  }
  Serial.println("Commands:\n  P - Program ATtiny10\n  I - Identify ATtiny10");
}

void loop () {
  while (true) {
    if (Serial.available() > 0) {
      unsigned char cc = toupper(Serial.read());
      switch (cc) {
        case 'I':
          // Identify connected ATtiny device
          printSignature();
          break;
        case 'P':
          // Program ATtiny
          writeFlash(fuse);
          break;
        case'D':
          download();
          break;
        case '?':
          printInstructions();
          break;
        case 'M':
          measureClock();
          break;
        case 'S':
          printSignature();
          printFuses();
          break;
        case 'V':
          pinMode(VCC, OUTPUT);
          digitalWrite(VCC, HIGH);      // VCC on
          Serial.println("VCC on");
          break;
        case 'X':
          digitalWrite(VCC, LOW);       // VCC off
          pinMode(VCC, INPUT);
          Serial.println("VCC off");
          break;
        case '*':
          Serial.write(0x1B);           // Respond with ESC code
          break;
      }
    }
  }
}