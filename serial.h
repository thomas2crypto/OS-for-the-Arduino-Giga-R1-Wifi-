class SerialCommander {
public:
  // Commander Version
  float com_ver;
  void begin(unsigned long baud, float com_verValue) {
    com_ver = com_verValue;
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

    } else if (message == "VER") {  // Prints the current version
      Serial.println(com_ver);

    } else if (message.startsWith("ECHO")) {  // Echoes the input string
      String string = message.substring(4);
      Serial.println(string);

    //... Rest of the commands ...

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
        return;
      }
    }
    Serial.println("Key not found.");
  }

  void printHelp() {
    Serial.println(F("Available Commands:"));
    Serial.println(F("  HELP                                      - Displays this help message"));
    Serial.println(F("  VER                                       - Shows the system version"));
    Serial.println(F("  SET <KEY> <VALUE>                         - Stores data"));
    Serial.println(F("  GET <KEY>                                 - Reads data"));
    Serial.println(F("  LIST                                      - Lists all stored data"));
    Serial.println(F("  PIN ON/OFF <NUMBER>                       - Turns a pin on/off"));
    Serial.println(F("  READ DI                                   - Reads the value of a digital pin"));
    Serial.println(F("  ANALOG <PIN> <VALUE>                      - Sets PWM value (0-255) on a pin"));
    Serial.println(F("  TOGGLE PIN <NUMBER>                       - Toggles pin state (HIGH/LOW)"));
    Serial.println(F("  ECHO <String>                             - Returns the input string"));
    Serial.println(F("  SET TIME <HOUR> <MINUTE> <SECOND>        - Sets the system time"));
    Serial.println(F("  REPEAT <NUMBER> <TEXT>                    - Repeats the text a specified number of times"));
    Serial.println(F("  CLEAR                                     - Clears the command line"));
    Serial.println(F("  BANNER                                    - Displays a banner like in MSF"));
    Serial.println(F("  I2C SCAN                                  - Scans the I2C bus for devices"));
    Serial.println(F("  I2C READ <ADDR> <REG> <LEN>               - Reads <LEN> bytes from device @ADDR starting at REG"));
    Serial.println(F("  I2C WRITE <ADDR> <VAL...>                 - Sends one or more values to an I2C address"));
    Serial.println(F("  TONE <PIN> <FREQUENCY> <DURATION(ms)>    - Plays tone on pin"));
    Serial.println(F("  IP                                        - Shows the IP address on the current WiFi"));
    Serial.println(F("  MAC                                       - Shows the MAC address of the current WiFi"));
    Serial.println(F("  RSSI                                      - Shows signal strength of current WiFi"));
    Serial.println(F("  DNS                                       - Shows current DNS server (usually router)"));
    Serial.println(F("  STATUS                                    - Shows WiFi connection status"));
    Serial.println(F("  EXEC <Script-name>                        - Executes a pre-defined script in code"));
    Serial.println(F("  LIST_SCRIPTS                              - Lists all available scripts"));
    Serial.println(F("  EXECUSB <Script-name>                     - Executes script stored on USB (if supported)"));
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
};