#ifndef SCRIPTS_H
#define SCRIPTS_H

#include <Arduino.h>

typedef void (*ScriptFunction)();

struct ScriptEntry {
  const char* name;
  ScriptFunction func;
};

void script_demoMessage() {
  Serial.println("Demo Script ausgeführt!");
}

void script_blinkAll() {
  for (int i = 2; i <= 13; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, HIGH);
    delay(100);
    digitalWrite(i, LOW);
  }
  Serial.println("script_blinkAll() done.");
}

ScriptEntry scriptTable[] = {
  { "DEMO", script_demoMessage },
  { "BLINK_ALL", script_blinkAll }
};

const int scriptCount = sizeof(scriptTable) / sizeof(ScriptEntry);

#endif
