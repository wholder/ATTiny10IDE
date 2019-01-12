#### I/O Ports and Pins

The following images show the functions for the various pins of the ATTiny10 series.

<p align="center"><img src="images/tiny4-9pins.jpg" width="587" height="207"></p>

<p align="center"><img src="images/tiny5-10pins.jpg" width="655" height="238"></p>

#### Accessing I/O Ports and Pins

* When coding an Arduino-like sketch (more below) for the ATTiny10, functions like `pinMode()` and `digitalWrite()` are used to access the physical I/O pins and you refer to a specific I/O pin using a number from 0-3.  For example, to set physical pin 4 as an output pin, you write `pinMode(2, OUTPUT)`. To set physical pin 4 to the HIGH state you write `digitalWrite(2, HIGH)`.

* When coding in C, you'll need to refer to I/O pins by **Port** name, such as `DDRB` (Data Direction Register B) and a **bit number**, such as `PB2`, to select one of the physical pins on that port.  For example, to set physical pin 4 as an output pin, you write `DDRB |= (1 << PB2)`.  Likewise, to set pin 4 to the `HIGH` state, you write `PORTB |= (1 << PB2)`.  To understand this fully, you should consult the ATTiny10's datasheet.
