# OS for the Arduino GIGA R1 WiFi

## TL;DR

A lightweight embedded OS and CLI framework for the Arduino GIGA R1 WiFi.  
Features include: startup configuration, login system, USB Mass Storage support, WiFi auto-connect, and a built-in command-line interface for runtime interaction and system control.

---

## Features

- Configurable startup options (WiFi, USB Mass Storage, login requirement)
- Soft reset capability via CLI
- CLI-controlled subsystem toggling (WiFi, USB, etc.)
- USB Mass Storage integration (read/write/delete files)
- WiFi connection using credentials from `arduino_secrets.h`
- Visual status indicator via onboard LED (D13)

---

## Startup Sequence

1. **Initialize Serial Interface** using `SerialCommander::begin`.
2. **Evaluate Startup Configuration** if enabled:
   - Prompts for binary inputs (0/1) to toggle:
     - USB Mass Storage
     - WiFi
     - (More coming soon.)
3. **Authentication** starts if `enable_login` is set to `true`.
4. **USB Setup**  
   If `USBDRIVE == true`, the system connects USB storage and displays  
   `"Directory /usb opened successfully."` on success.
5. **WiFi Setup**
   1. IF `fixed-wifi == true`, starts the func connectToWifi wich let's you dynamikliy connect to a Wifi at startup.
   2. If `wlan == true`, attempts to connect using `arduino_secrets.h`.
7. **Re-setup Handling**  
   Prevents infinite soft-reset loops.
8. Enters the `loop()`.

---

## Main Loop

- If `re_setup == true`: runs soft reset to apply new settings.
- Runs `commander.update()` to listen for commands over Serial.
- Calls `toggleStatusLed()` to blink the onboard LED.

---

## Command Line Interface (CLI)

### Control & Information Commands

These are the primary commands used to control and monitor the system:

| Command            | Description                                     |
|--------------------|-------------------------------------------------|
| `HELP`             | Lists all available commands                    |
| `SYSINFO`          | Prints Information about the CLI                |
| `UPTIME`           | Prints the current uptime since last reboot     |
| `Soft-Reset`       | Re-runs `setup()` to apply changes              |
| `EN-Wifi`          | Enables/disables WiFi subsystem                 |
| `EN-USB`           | Enables/disables USB subsystem                  |
| `EN-Startup_config`| Enables/disables startup configuration prompt   |

Note: Configuration changes only take effect after running `Soft-Reset`.

---

## Closer Look: The `FILE` Command

The `FILE` command allows basic interaction with the USB Mass Storage device.  
It supports the following subcommands:

| Subcommand     | Function                                  |
|----------------|-------------------------------------------|
| `FILE LIST`    | Lists all files on the USB device         |
| `FILE READ`    | Reads contents of a `.txt` file           |
| `FILE WRITE`   | Appends content to a specified file       |
| `FILE DELETE`  | Deletes a specified file                  |

Only basic `.txt` files have been tested. File system support relies on `FATFileSystem.h`.

---

## Used Libraries

- `Arduino.h`
- `Arduino_USBHostMbed5.h`
- `FATFileSystem.h`
- `DigitalOut.h`
- `SDRAM.h`
- `WiFiInterface.h`
- `SPI.h`
- `WiFi.h`

If you're unsure about the exact name in the Library Manager, search using the `.h` filename.

---

## Known Issues

- The `ANALOG PIN` command is currently unstable.  
  Contributions or working implementations are welcome.

---

## Credits

Based on the official [Arduino GIGA Cheat Sheet](https://docs.arduino.cc/tutorials/giga-r1-wifi/cheat-sheet)

---

## Feedback

Found an issue or have a suggestion?  
Open an issue or submit a pull request on GitHub.
