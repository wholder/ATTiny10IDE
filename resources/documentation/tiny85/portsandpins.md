## The ATTiny25/45/85's I/O Ports and Pins

The following image shows the functions for the various pins on the ATTiny85.  When coding an Arduino-like sketch (more below) for the ATTiny85, functions line `pinMode()` and` digitalWrite()` are used to access the physical I/O pins and you refer to a specific I/O pin using a number from 0-5.  For example, to set physical pin 5 as an output pin, you write `pinMode(0, OUTPUT)`  and to set physical pin 5 to the HIGH state you write `digitalWrite(0, HIGH)`.

<p align="center"><img src="images/tiny85pins.jpg" width="760" height="267"></p>

When coding in C, you'll need to refer to I/O pins by **Port** name, such as `DDRB` (Data Direction Register B) and a **bit number**, such as `PB0`, to select one of the physical pins on that port.  For example, to set physical pin 5 as an output pin, you write `DDRB |= (1 << PB0)`.  Likewise, to set pin 5 to the `HIGH` state, you write `PORTB |= (1 << PB0)`.  To understand this fully, you should consult the ATTiny85's datasheet.

**Note:** Please notice that the pins for the signals `MOSI` and `MISO` are labelled differently when used to connect to an external, in-circuit programmer (yellow) and when they are used to connect to an SPI device (pink) and be careful not to confuse these connections.