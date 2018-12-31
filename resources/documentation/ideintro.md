ATTiny10IDE is a simple, GNU-based IDE I originally wrote to simplify writing code for the ATTiny10 Series Microcontrollers using C, C++ or Assembly language.  This new version is a greatly expanded and updated version I started working on in 2014.  [See this article for some additional details on how this project started](https://sites.google.com/site/wayneholder/attiny10-c-ide-and-improved-device-programmer).  For this release, I've added support for the ATTiny25/45/85 and ATTiny24/44/84 series of microcontrollers, as well support for programming them using an ISCP Programmer.  There is also an experimental feature for automatic generation of function prototypes.

## Credit and Thanks

This project would have been much harder and much less cool without help from the following open source projects, or freely available software.

 + [Java Simple Serial Connector 2.8.0](https://github.com/scream3r/java-simple-serial-connector) - JSSC is used to communicate with the Arduino-based programmer
 + [JSyntaxPane](https://github.com/nordfalk/jsyntaxpane) - Now uses [CppSyntaxPane](https://github.com/wholder/CppSyntaxPane), which is based on JSyntaxPane.
 + [IntelliJ IDEA from JetBrains](https://www.jetbrains.com/idea/) (my favorite development environment for Java coding. Thanks JetBrains!)
 + [ANTLR version 4.7.1](http://www.antlr.org) - ALTLRv4 was used to generate the parser for the new (experimental) automatic prototype type generation feature.
 + [CPP14 Antlr Grammar](https://github.com/antlr/grammars-v4/blob/master/cpp/CPP14.g4) - CPP14 was used as the base, C++ grammer for the automatic prototype type generation feature.
 
## MIT License

Copyright 2014-2018 Wayne Holder

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.