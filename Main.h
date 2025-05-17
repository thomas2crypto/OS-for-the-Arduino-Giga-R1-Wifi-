//This is the Main Scribt of the OS

#include <Arduino.h>
#include <Wire.h>
#include "serial.h"
#include <SDRAM.h>

// Status LED Timing
const int StatusledPin = LED_BUILTIN;            // Pin Nummer für die StatusLED
int ledState = LOW;                    // ledState zum Setzen der LED
unsigned long previousMillis = 0;      // speichert den letzten Aktualisierungszeitpunkt der LED
const long interval = 500;             // Zeitintervall (Millisekunden) zum Blinken
// Booted LED
const int BootedLedPin = 7;
bool firstloop = false;
//Memory Array im SDRAM
//uint8_t* Data = (uint8_t*)SDRAM.malloc(4 * 1024 * 1024);
//unsigned long Data_counter=0;
// Klassen-Kopien
SerialCommander commander;

void setup() {
  // Check-LED Pin konfigurieren
  pinMode(StatusledPin, OUTPUT);
  // Booted-Led Pin konfigurieren
  pinMode(BootedLedPin, OUTPUT);
  // Commander initialisieren
  commander.begin(115200,1);
  
   
  //Wire initaliesieren
  Wire.begin();
}

void loop() {
  commander.update();
  if (!firstloop) {
    digitalWrite(BootedLedPin, HIGH);
    firstloop = true;
  }

  LED();


}  
// LED-Blinker-Funktion
void LED(){
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    ledState = (ledState == LOW) ? HIGH : LOW;
    digitalWrite(StatusledPin, ledState);
  }
}

