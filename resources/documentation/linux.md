### Linux Serial Ports
Serial port access is limited to certain users and groups in Linux, which may prevent you from seeing programmers that use a serial port interface. To enable user access, you must open a terminal and enter the following commands before **`ATTiny10IDE`** will be able to access the ports on your system. Don't worry if some of the commands fail. All of these groups may not exist on every Linux distro. (Note, this process must only be done once for each user):

  + `sudo usermod -a -G uucp username`
  + `sudo usermod -a -G dialout username`
  + `sudo usermod -a -G lock username`
  + `sudo usermod -a -G tty username`

Replace the username parameter with your current username. (If you are not sure what your username is, type **`whoami`** and it will tell you.) If you are using SUSE 11.3 or higher, replace the '**`-a -G`**' flags with a single '**`-A`**' flag. Log out and restart **`ATTiny10IDE`** you should have access to the serial port.