## Understanding Fuses

Fuses are a special set of programmable, nonvolatile registers that configure, or enable certain special features in the ATTiny.  Where the ATTiny4/5/9/10 series has only a single fuse "byte," the ATTiny24/44/84 and ATTiny25/45/85 series each have three fuse bytes called the "Low" fuse, the "high" fuse and the "extended" fuse.  Normally, these fuse registers are intended to be programmed once rather than each time you program new code into flash memory, so an in-circuit programmer uses a separate set of programming commands.  In general, fuses used to enable, disable, or configure certain special or optional features on this chip, such as:

  + Enable and Configure Brownout Detection
  + Enable and Configure the Watchdog Timer
  + The Source of the System Clock
  + A special Divide CLock by 8 setting
  + Special pin functions, as as using RESET as an I/O pin
  + The debugWire Debugger
  + Whether Self Programming is enabled (for a bootloader)
  + Whether "In Circuit" Programming is enabled
  + Enabling a pin as an output for the System Clock
  + Start Up Delay to allow the clock to settle
  
ATTinyIDE is able to read an set these fuse register and also provides a pop up dialog (see below) that shows either the current settings of the fuses, of the fuses that will be programmed when you choose to write new settings.

<p align="center"><img src="images/Fuse Dialog.png" width="538" height="387"></p>

**Caution:** unfortunately, unless you are careful, some fuse settings can configure the device such that it can no longer be programmed by an in-circuit programmer.  In the dialog shown above, the most dangerous fuses are shown in RED, but other fuses, such as those that configure the System Clock can also "brick" an ATTiny is programmed improperly.  It is possible to recover a bricked device by using a programmer that supports a special, [High Voltage Programming Protocol](https://sites.google.com/site/wayneholder/attiny-fuse-reset) that can reset all the fuses back to their factory default settings.


