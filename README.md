# OS-for-the-Arduino-Giga-R1-Wifi-

Functional overview:

1. Startup Configuration:
   you can onfigure what of the feature you wanna use, for example:
   Wifi,USB-MassStorage,and the Login in functionality at the Startup.
   Also the baudrate, for the Serial interface, can be changed.
2. Startup sequence:
   1. initialize the Serial-interface(using the begin method within the class "SerialCommander".
   2. checking if the Startup-conifg is enable, if it is the CLI will ask for a combination of 0 and 1, after the folowing path (a 0 being false, a 1 being true):
      1. USB-MassStorage Enable/Disable.
      2. Wifi Enable/Disable.
      3. othe one are coming soon!
   3. If the bool enable_login(at the top positioned) is true, is the authentication prozess starting now.
   4. If the bool USBDRIVE(at the top positioned) is true, is the USB-MassStorage devices will be connected and only if that's finished, the Setup prozess will go on!
        -It will run a quick test and show "Directory /usb opened successfully." in Serial-Interface.
   5. If the bool wlan (at the top positioned) is true, is the will try to connect, with the access data from the "arduino_secrets.h" file and only if that's finished, the Setup prozess will go on!
   6. At the end is it checking if re_setup(for the Soft-Reset used) at set's it to false(otherwise it will Soft-Reset over and over again)
   7. Then Starts the loop.
3. The Loop:
   1. Checks if re_setup is true, it will then Soft-Reset(rerunning the setup-function, to enable things like wifi,blutooth(wich have to be enable using the command in the CLI(type "HELP" in the Command line))).
   2. runs commander.update , wich checks for new commands from the Serial-Interface.
   3. Runs toggleStatusLed, wich just blinks the on board LED(D13) green.
4. The Command-Line-Interface:
   1. Idee: Having a CLI to Test things on the Arduino platform, be able to control the Ardunio.
   2. Command-Set:
      1.   HELP: Showes the HELP about all the commands.
      2.   VER: Prints out the Version of the CLI.
      (the Next commands are explained with the "HELP" command.
      3.   FILE: (FILE <COMMAND>
           Commands:
           1. LIST: Lists everything in the USB-MassStorage Device.
           2. READ: Reads a File contens(only .txt were tested)(FILE READ <FILENAME>).
           3. WRITE: Writes contes behind the Filename to the File(FILE WRITE <FILENAME> <STRING>).
           4. DELETE: Deletes a File. (FILE DELETE <FILENAME>).
      4.Soft-Reset: Re-runs the Setup to change things like: Wifi on/off,USB on/off,Startup-Config on/off.
      5.EN-(Wifi/USB_THUMB/Startup_config) turns a bool(see Startup_secuence: 3./4./5.) to turn these things on or off, to "write" the changes run "Soft-Reset".
5.Used Libarys:
    Arduino.h
    Arduino_USBHostMbed5.h
    FATFileSystem.h  
    DigitalOut.h
    SDRAM.h
    WiFiInterface.h
    SPI.h
    WiFi.h
    (I'm sorry, i can't remember the names these Libarys are in the Libary manager, just search for the name of the ".h" file then you should find the libary needed).

PS:
the "ANALOG PIN" command is making me real truble, if any one is so kind to give me an fixed version of the else if fot the command, i would really appriciate it.
CREDS:
 Ardunio Cheat Sheet: https://docs.arduino.cc/tutorials/giga-r1-wifi/cheat-sheet/?_gl=1*1xmc0ej*_up*MQ..*_ga*MTY3MTkzNDM4Ni4xNzQ4NjQxMDk4*_ga_NEXN8H46L5*czE3NDg2NDEwOTckbzEkZzAkdDE3NDg2NDEwOTckajYwJGwwJGgzMDY1MDcwNDM.

Pleas let me know any issues.
