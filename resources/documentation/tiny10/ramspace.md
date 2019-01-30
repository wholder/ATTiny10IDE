## Dealing with the ATTiny4/5/9/10's Limited RAM Space

The ATTiny10 provides only 32 bytes of RAM space for variables and the program call stack.  So, it's important to use `char` and `unsigned char` variables instead of `int` and `unsigned int` wherever possible and limit function call depth (recursion is not recommended), use stack variables carefully (variables declared in functions) and globally-declared arrays.  Fortunately, the AVR architecture has 32 registers available and the GNU compiler makes very efficient use of them, but there are still a few tricks that can help keep RAM usage under control.

One essential trick is [using the `PROGMEM` directive](http://www.atmel.com/webdoc/avrlibcreferencemanual/pgmspace_1pgmspace_data.html) to create `const` arrays that are stored in Flash memory rather than RAM.  This only works for read only arrays, but using it for table lookup, or strings of text makes it possible to do things would be impossible without it.  The example program `Morse.c` shows how to use `PROGMEM` to store both a lookup table for ASCII to Morse code conversion as well as for a `char[]` string for the message text.

