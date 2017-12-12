<p align="center"><img src="https://github.com/wholder/ATTiny10IDE/blob/master/images/ATTiny10IDE Screenshot.png"></p>

# ATTiny10IDE
ATTiny10IDE is a simple, GNU-based IDE I wrote to simplify writing code for the ATTiny10 Series Microcontrollers using CPP and Assembler.

If you just want to try out the program, you don't need to download and compile the source code, as I try to maintain a pre-built, executable JAR file in the [out/artifacts/ATTiny10IDE_jar](https://github.com/wholder/ATTiny10IDE/tree/master/out/artifacts/ATTiny10IDE_jar) folder from which you can download and run ATTiny10IDE as long as you have Java installed on your system.  On a Mac, simply double click the ATTiny10IDE.jar file to run it once you've downloaded it, although you'll probably have to right click and select "Open" the  first time you run ATTiny10IDE due to the Mac OS X security check and the fact that I'm currently unable to digitally sign the JAR file.  You should also be able to double click and run using Windows, but some versions of Windows may require you to [enable this ability](https://www.addictivetips.com/windows-tips/run-a-jar-file-on-windows/) first.  The current version does not run on Linux.
## Requirements
Note: Requires Java 8, or better.
I wrote ATTiny10IDE on a Mac Pro using the _Community_ version of [IntelliJ IDEA from JetBrains](https://www.jetbrains.com/idea/) and OS X is the only environment where I have extensively tested and used ATTiny10IDE .  However, as the code is nearly 100% Java, it should also run on MS Windows and Linux systems.  One exception is JSSC (Java Simple Serial Connector), which contains low-level, native drivers, but JSSC is only needed to talk to GRBL-based laser cutters.  Feel free to report any issues you discover.  I'll do my best, when time permits, to investigate them, but I cannot guarantee fixes, timely or otherwise.
## Credits
- [Java Simple Serial Connector 2.8.0](https://github.com/scream3r/java-simple-serial-connector) "**JSSC**" 
- [IntelliJ IDEA from JetBrains](https://www.jetbrains.com/idea/) (my favorite development environment for Java coding. Thanks JetBrains!)
