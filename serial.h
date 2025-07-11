#include <SDRAM.h>
#include <WiFiInterface.h>
#include <SPI.h>
#include <WiFi.h>
#include <Arduino.h>
#include <Arduino_USBHostMbed5.h>
#include <FATFileSystem.h>
#include <DigitalOut.h>
#include "config.h"
#include <mbed.h>
#include <mbed_stats.h>
#include "Programms.h"

// Get the VERSION
#define FW_VERSION "0.3.1"
#define GIT_HASH "unknown"

class SerialCommander {
public:

  void begin(unsigned long baud) {
    Serial.begin(baud);
  }

  void update() {
    readSerialData();

    if (serialIndex > 0 && serialIn[serialIndex - 1] == '\n') {
      serialIn[serialIndex - 1] = '\0';
      receivedMessage = String(serialIn);
      serialIndex = 0;
      processMessage(receivedMessage);
    }
  }

private:

  static const int BUFFER_SIZE = 200;
  static const int MAX_ITEMS = 200;
  char serialIn[BUFFER_SIZE];
  int serialIndex = 0;
  String receivedMessage;
  String keys[MAX_ITEMS];
  String values[MAX_ITEMS];
  bool echoEnabled = false;

  void readSerialData() {
    while (Serial.available() > 0) {
      char c = Serial.read();
      if (serialIndex < BUFFER_SIZE - 1) {
        serialIn[serialIndex++] = c;
      } else {
        serialIndex = 0;
        Serial.println("Buffer overflow!");
      }
    }
  }

  void processMessage(String message) {
    message.trim();
    message.toUpperCase();

    if (message.startsWith("SET ")) {  // Writes the value to a key
      int keyStart = 4;
      int keyEnd = message.indexOf(' ', keyStart);
      int valueStart = keyEnd + 1;

      if (keyEnd > 0 && valueStart < message.length()) {
        String key = message.substring(keyStart, keyEnd);
        String value = message.substring(valueStart);
        storeKeyValue(key, value);
      }

    } else if (message.startsWith("GET ")) {  // Reads a value from a key
      String key = message.substring(4);
      getKeyValue(key);

    } else if (message.startsWith("PIN ON ")) {  // Turns the pin on
      int pin = message.substring(7).toInt();
      digitalWrite(pin, HIGH);
      Serial.print("Pin ");
      Serial.print(pin);
      Serial.println(" set to HIGH");

    } else if (message.startsWith("PIN OFF ")) {  // Turns the pin off
      int pin = message.substring(8).toInt();
      digitalWrite(pin, LOW);
      Serial.print("Pin ");
      Serial.print(pin);
      Serial.println(" set to LOW");

    } else if (message.startsWith("READ DI")) {  // Reads digital input pin state
      int pin = message.substring(8).toInt();
      bool is_on = digitalRead(pin);
      Serial.println(is_on ? "HIGH" : "LOW");

    } else if (message.startsWith("ANALOG ")) {  // Sets PWM value on pin
      int pinStart = 7;
      int pinEnd = message.indexOf(' ', pinStart);
      int valueStart = pinEnd + 1;

      if (pinEnd > 0 && valueStart < message.length()) {
        int pin = message.substring(pinStart, pinEnd).toInt();
        int value = message.substring(valueStart).toInt();

        if (value >= 0 && value <= 255) {
          analogWrite(pin, value);
          Serial.print("Analog value ");
          Serial.print(value);
          Serial.print(" written to pin ");
          Serial.println(pin);
        } else {
          Serial.println("Invalid value. Allowed range: 0 - 255.");
        }
      } else {
        Serial.println("Syntax error. Usage: ANALOG <PIN> <VALUE>");
      }

    } else if (message == "HELP") {  // Shows the help menu
      printHelp();

    } else if (message.startsWith("REPEAT ")) {  // Repeats text N times
      int spacePos = message.indexOf(' ', 7);
      int repeatCount = message.substring(7, spacePos).toInt();
      String text = message.substring(spacePos + 1);

      for (int i = 0; i < repeatCount; i++) {
        Serial.println(text);
      }

    } else if (message.startsWith("BLINK ")) {  // Blink a pin for a duration
      int pin = message.substring(7, 9).toInt();
      int duration = message.substring(10).toInt();
      digitalWrite(pin, HIGH);
      delay(duration);
      digitalWrite(pin, LOW);
      delay(duration);
      Serial.print("Pin ");
      Serial.print(pin);
      Serial.print(" blinked for ");
      Serial.print(duration);
      Serial.println(" ms.");

    } else if (message.startsWith("TONE ")) {  // Play a tone on a pin
      int firstSpace = message.indexOf(' ', 5);
      int secondSpace = message.indexOf(' ', firstSpace + 1);

      if (firstSpace > 0 && secondSpace > 0) {
        int pin = message.substring(5, firstSpace).toInt();
        int freq = message.substring(firstSpace + 1, secondSpace).toInt();
        int dur = message.substring(secondSpace + 1).toInt();
        playTone(pin, freq, dur);
      } else {
        Serial.println("Syntax: TONE <PIN> <FREQUENCY> <DURATION(ms)>");
      }

    } else if (message == "CLEAR") {  // Clears the command line (prints empty lines)
      for (int i = 0; i < 100; i++) {
        Serial.println("          ");
      }

    } else if (message.startsWith("ECHO")) {  // Echoes the input string
      String string = message.substring(4);
      Serial.println(string);



    } else if (message == "PING") {
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("PING OK - IP: " + WiFi.localIP().toString());
      } else {
        Serial.println("PING FAIL - WiFi disconnected");
      }
    } else if (message == "IP") {
      Serial.println("IP Address: " + WiFi.localIP().toString());
    } else if (message == "MAC") {
      Serial.println("MAC Address: " + WiFi.macAddress());
    } else if (message == "RSSI") {
      Serial.println("Signal Strength: " + String(WiFi.RSSI()) + " dBm");
    } else if (message == "GATEWAY") {
      Serial.println("Gateway: " + WiFi.gatewayIP().toString());
    } else if (message == "DNS") {
      Serial.println("DNS: " + WiFi.dnsIP().toString());
    } else if (message == "STATUS") {
      String status = (WiFi.status() == WL_CONNECTED) ? "Connected" : "Not connected";
      Serial.println("Status: " + status + ", IP: " + WiFi.localIP().toString());
    } else if (message.startsWith("FILE ")) {
      String args = message.substring(5);
      args.trim();

      if (args == "LIST") {
        handleFileCommand("LIST");
      } else if (args.startsWith("READ ")) {
        String filename = args.substring(5);
        filename.trim();
        handleFileCommand("READ", filename);
      } else if (args.startsWith("WRITE ")) {
        int spaceIndex = args.indexOf(' ', 6);
        if (spaceIndex == -1) {
          Serial.println("Syntax error: FILE WRITE <filename> <text>");
          return;
        }
        String filename = args.substring(6, spaceIndex);
        String text = args.substring(spaceIndex + 1);
        handleFileCommand("WRITE", filename, text);
      } else if (args.startsWith("DELETE ")) {
        String filename = args.substring(7);
        filename.trim();
        handleFileCommand("DELETE", filename);
      } else {
        Serial.println("Invalid FILE command.");
      }
    } else if (message == "RESTART") {
      re_setup = true;
      Serial.println("System is restarting...");
    } else if (message == "EN-WIFI") {
      wlan = true;
      if (wlan) {
        Serial.println("Wifi is now enable,please RESTART");
      } else {
        Serial.println("Wifi is now disable");
      }
    } else if (message == "EN-FIXED-WIFI") {
      fixed_wifi = true;
      if (fixed_wifi) {
        Serial.println("fixed-Wifi is now enable,please RESTART");
      } else {
        Serial.println("fixed-Wifi is now disable");
      }
    } else if (message == "EN-USB-MASS") {
      USBDRIVE = true;
      if (USBDRIVE) {
        Serial.println("USB-Storage is now enable,please RESTART");
      } else {
        Serial.println("USB-Storage is now disable");
      }
    } else if (message == "EN-STARTUP-CONFIG") {
      startup_config_enabled = true;
      if (startup_config_enabled) {
        Serial.println("Startup-Config is now enable,please RESTART");
      } else {
        Serial.println("Startup-Config is now disable");
      }
    } else if (message == "SHOW-CONFIG") {
      Serial.println("Current-Configuration:");
      Serial.println("Wifi: " + String(wlan ? "enabled" : "disabled"));
      Serial.println("USB: " + String(USBDRIVE ? "enabled" : "disabled"));
      Serial.println("Startup-Config: " + String(startup_config_enabled ? "true" : "false"));
    } else if (message == "SYSINFO") {
      showSysInfo();
    } else if (message == "UPTIME") {
      showUptime();
    }else if (message == "LIST-PROGS") {
      Programms::listAll(Serial);
    }
    // RUN command: e.g. "RUN BLINK" runs the program "blink"
    else if (message.startsWith("RUN ")) {
      // Extract everything after "RUN " as the program name
      String programName = message.substring(4);
      programName.trim();

      if (programName.length() > 0) {
        // Convert program name to lowercase if your program registry is case-sensitive
        programName.toLowerCase();
        Programms::run(programName.c_str(), Serial);
      } else {
        Serial.println(F("[ERROR] Please provide a program name after RUN."));
      }
    } else {
      Serial.println("Unknown command: " + message);
      Serial.println(" Type 'HELP' for assistance.");
    }
  }
  void storeKeyValue(String key, String value) {
    for (int i = 0; i < MAX_ITEMS; i++) {
      if (keys[i] == key || keys[i] == "") {
        keys[i] = key;
        values[i] = value;
        Serial.print("Stored: ");
        Serial.print(key);
        Serial.print(" = ");
        Serial.println(value);
        return;
      }
    }
    Serial.println("Storage full! No more space available.");
  }

  void getKeyValue(String key) {
    for (int i = 0; i < MAX_ITEMS; i++) {
      if (keys[i] == key) {
        Serial.print(key);
        Serial.print(" = ");
        Serial.println(values[i]);
https:  //github.com/thomas2crypto/OS-for-the-Arduino-Giga-R1-Wifi-/blob/main/serial.h
        return;
      }
    }
    Serial.println("Key not found.");
  }

  void printHelp() {
    Serial.println(F("Available Commands:"));
    Serial.println(F("  HELP                                      - Displays this help message"));
    Serial.println(F("  SYSINFO                                   - Shows Informatoin about the system"));
    Serial.println(F("  UPTIME                                    - Shows the current Uptime"));
    Serial.println(F("  SET <KEY> <VALUE>                         - Stores data"));
    Serial.println(F("  GET <KEY>                                 - Reads data"));
    Serial.println(F("  LIST                                      - Lists all stored data"));
    Serial.println(F("  LIST-PROGS                                - Lists all avalible Programms, definde in Programms.h"));
    Serial.println(F("  RUN <name>                                      - runs a Programm, definde in Programms.h"));
    Serial.println(F("  PIN ON/OFF <NUMBER>                       - Turns a pin on/off"));
    Serial.println(F("  READ DI <PIN>                             - Reads the value of a digital pin"));
    Serial.println(F("  ANALOG <PIN> <VALUE>                      - Sets PWM value (0-255) on a pin"));
    Serial.println(F("  TOGGLE PIN <NUMBER>                       - Toggles pin state (HIGH/LOW)"));
    Serial.println(F("  ECHO <String>                             - Returns the input string"));
    Serial.println(F("  SET TIME <HOUR> <MINUTE> <SECOND>         - Sets the system time"));
    Serial.println(F("  REPEAT <NUMBER> <TEXT>                    - Repeats the text a specified number of times"));
    Serial.println(F("  CLEAR                                     - Clears the command line"));
    Serial.println(F("  TONE <PIN> <FREQUENCY> <DURATION(ms)>     - Plays tone on pin"));
    Serial.println(F("  IP                                        - Shows the IP address on the current WiFi"));
    Serial.println(F("  MAC                                       - Shows the MAC address of the current WiFi"));
    Serial.println(F("  RSSI                                      - Shows signal strength of current WiFi"));
    Serial.println(F("  DNS                                       - Shows current DNS server (usually router)"));
    Serial.println(F("  STATUS                                    - Shows WiFi connection status"));
    Serial.println(F("  FILE:                                     - Used to access the USB-Massstorage Device(see Readme on:)"));
    Serial.println(F("  Link to Github: https://github.com/thomas2crypto/OS-for-the-Arduino-Giga-R1-Wifi-"));
    Serial.println(F("  RESTART                                   - Reinitializes the system without full reboot"));
    Serial.println(F("  EN-WIFI                                   - Enables WiFi functionality"));
    Serial.println(F("  EN-FIXED-WIFI                             - Enables to connect always to a specifed  network"));
    Serial.println(F("  EN-USB-MASS                               - Enables support for USB thumb drives"));
    Serial.println(F("  EN-STARTUP-CONFIG                         - Enables loading startup configuration from USB"));
    Serial.println(F("  SHOW-CONFIG                               - Shows the current Configuration"));
  }

  void playTone(int pin, int frequency, int duration) {
    tone(pin, frequency, duration);
    delay(duration);
    noTone(pin);
    Serial.print("Tone played on pin ");
    Serial.print(pin);
    Serial.print(" at frequency ");
    Serial.print(frequency);
    Serial.print("Hz for ");
    Serial.print(duration);
    Serial.println(" ms.");
  }
  void handleFileCommand(String action, String filename = "", String text = "") {
    String basePath = "/usb";  // Mount point of your filesystem

    if (action == "LIST") {
      DIR* dir = opendir(basePath.c_str());
      if (!dir) {
        Serial.println("Error: Unable to open directory.");
        return;
      }

      struct dirent* entry;
      while ((entry = readdir(dir)) != NULL) {
        // Only list files, skip directories
        if (!(entry->d_type & DT_DIR)) {
          Serial.println(entry->d_name);
        }
      }
      closedir(dir);
    }

    else if (action == "READ") {
      String fullPath = basePath + filename;
      FILE* file = fopen(fullPath.c_str(), "r");
      if (!file) {
        Serial.println("Error: File not found - " + filename);
        return;
      }

      char buffer[128];
      size_t bytesRead;
      while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        Serial.write((const uint8_t*)buffer, bytesRead);
      }
      fclose(file);
      Serial.println();
    }

    else if (action == "WRITE") {
      String fullPath = basePath + filename;
      FILE* file = fopen(fullPath.c_str(), "w");
      if (!file) {
        Serial.println("Error: Unable to open file for writing - " + filename);
        return;
      }

      fwrite(text.c_str(), 1, text.length(), file);
      fclose(file);
      Serial.println("File written successfully: " + filename);
    }

    else if (action == "DELETE") {
      String fullPath = basePath + filename;
      if (remove(fullPath.c_str()) == 0) {
        Serial.println("File deleted successfully: " + filename);
      } else {
        Serial.println("Error: Failed to delete file - " + filename);
      }
    }

    else {
      Serial.println("Error: Unknown file command.");
      Serial.println("Available actions: LIST, READ, WRITE, DELETE");
    }
  }
  int getFreeMemory() {
    mbed_stats_heap_t heap_stats;
    mbed_stats_heap_get(&heap_stats);

    // Freier Heap = reservierte Gesamtgröße - aktuell genutzter Speicher
    return (heap_stats.reserved_size - heap_stats.current_size);
  }

  void showSysInfo() {
    Serial.println("System Information:");
    Serial.println("-------------------");
    Serial.println("Board         : Arduino GIGA R1 WiFi");
    Serial.println("Firmware Ver. : 0.2.1");
    Serial.println("Build Date    : " __DATE__ " " __TIME__);
    Serial.println("Git Commit  : " GIT_HASH);
    Serial.print("CPU Clock     : ");
    Serial.print(SystemCoreClock / 1000000);
    Serial.println(" MHz");
    Serial.print("Free RAM     : ");
    Serial.print(getFreeMemory() / 1024);
    Serial.println(" KB");
  }

  // UPTIME
  void showUptime() {
    unsigned long ms = millis();
    unsigned long s = ms / 1000;
    unsigned int seconds = s % 60;
    unsigned int minutes = (s / 60) % 60;
    unsigned int hours = (s / 3600) % 24;
    unsigned int days = s / 86400;

    Serial.print("System Uptime: ");
    Serial.print(days);
    Serial.print("d ");
    Serial.print(hours);
    Serial.print("h ");
    Serial.print(minutes);
    Serial.print("m ");
    Serial.print(seconds);
    Serial.println("s");
  }
};
