#include <Arduino.h>
#include <Wire.h>
#include "serial.h"
#include <SDRAM.h>
#include <WiFiInterface.h>
#include <SPI.h>
#include <WiFi.h>
#include "arduino_secrets.h"

#include <Arduino_USBHostMbed5.h>
#include <FATFileSystem.h>
#include <DigitalOut.h>

// USB Host Mass Storage Device
USBHostMSD msd;
// Filesystem object with "usb" mount point
mbed::FATFileSystem usb("usb");

// USB-A Port Power Enable Pin (board-specific)
const int USB_POWER_PIN = PA_15;

// Configuration flags
bool USBDRIVE = true;
bool wlan = false;
bool enable_login = false;
bool startup_config_enabled = true;

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int status = WL_IDLE_STATUS;

bool isAuthenticated = false;
String loginBuffer = "";

const int StatusLedPin = LED_BUILTIN;
int ledState = LOW;
unsigned long previousMillis = 0;
const long interval = 500;

// Flag to trigger re-setup
bool re_setup = false;

// CLI instance
SerialCommander commander;

void setup() {
  commander.begin(115200, 0.2);
  while (!Serial) {}

  if (startup_config_enabled) {
    Serial.println("Please enter startup configuration (e.g., 101):");

    while (Serial.available() == 0) {
      // Waiting for input
    }

    String config = Serial.readStringUntil('\n');
    config.trim();

    // Example: 3 flags e.g. USB, WLAN, Login
    if (config.length() >= 1 && config.charAt(0) == '1') {
      USBDRIVE = true;
      Serial.println("USB enabled");
    } else {
      USBDRIVE = false;
      Serial.println("USB disabled");
    }

    if (config.length() >= 2 && config.charAt(1) == '1') {
      wlan = true;
      Serial.println("WiFi enabled");
    } else {
      wlan = false;
      Serial.println("WiFi disabled");
    }

    if (config.length() >= 3 && config.charAt(2) == '1') {
      enable_login = true;
      Serial.println("Login enabled");
    } else {
      enable_login = false;
      Serial.println("Login disabled");
    }
  }

  // Clear screen by printing blank lines
  for (int i = 0; i < 100; i++) Serial.println("");

  if (enable_login) {
    // Authentication procedure
    while (!isAuthenticated) {
      Serial.println("Please enter password:");
      loginBuffer = "";
      while (true) {
        if (Serial.available()) {
          char c = Serial.read();
          if (c == '\n' || c == '\r') {
            loginBuffer.trim();
            if (loginBuffer == CLI_PASSWORD) {
              isAuthenticated = true;
              Serial.println("Access granted. Welcome, " CLI_USERNAME ".");
              break;
            } else {
              Serial.println("Incorrect password. Please try again:");
              loginBuffer = "";
            }
          } else {
            loginBuffer += c;
          }
        }
      }
    }
  }

  Serial.println("Commander started...");
  delay(500);

  pinMode(StatusLedPin, OUTPUT);

  if (USBDRIVE) {
    // Enable USB-A port power
    pinMode(USB_POWER_PIN, OUTPUT);
    digitalWrite(USB_POWER_PIN, HIGH);
    Serial.println("USB-A port powered on.");

    // Connect USB Mass Storage Device
    Serial.println("Attempting to connect to USB mass storage...");
    while (!msd.connect()) {
      Serial.println("USB drive not found, retrying...");
      delay(1000);
    }

    // Mount FAT filesystem
    Serial.print("Mounting USB drive... ");
    int err = usb.mount(&msd);
    if (err) {
      Serial.print("Mount error: ");
      Serial.println(err);
      while (1)
        ;  // Fatal error, halt execution
    }
    Serial.println("successful.");

    DIR* test = opendir("/usb");
    if (test) {
      Serial.println("Directory /usb opened successfully.");
      closedir(test);
    } else {
      Serial.println("ERROR: Unable to open /usb directory!");
    }
  }

  Serial.println("Initializing I2C interface...");
  delay(250);
  Wire.begin();
  Serial.println("I2C ready.");

  if (wlan) {
    if (WiFi.status() == WL_NO_MODULE) {
      Serial.println("Communication with WiFi module failed!");
      while (true) {}  // Halt execution
    }

    WiFi.setHostname("GIGA_R1");
    while (status != WL_CONNECTED) {
      Serial.print("Connecting to SSID: ");
      Serial.println(ssid);
      status = WiFi.begin(ssid, pass);
      delay(5000);
    }

    Serial.println("Network connected.");
    printCurrentNetwork();
    printWifiData();
  } else {
    Serial.println("WiFi disabled.");
  }

  Serial.println("Setup complete.");
  if (re_setup) {
    re_setup = !re_setup;
  }
}

void loop() {
  if (re_setup) {
    setup();
  }
  commander.update();
  toggleStatusLed();
}

void toggleStatusLed() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    ledState = (ledState == LOW) ? HIGH : LOW;
    digitalWrite(StatusLedPin, ledState);
  }
}

// WiFi related helpers
void printWifiData() {
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC Address: ");
  printMacAddress(mac);
}

void printCurrentNetwork() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.println(rssi);

  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type: ");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}
