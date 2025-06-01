//Here can you write your on programms to run with the "run <name>" command
#ifndef PROGRAMMS_H
#define PROGRAMMS_H

#include <Arduino.h>
#include <string.h>

namespace Programms {

  // === Configuration constants ===
  const int MAX_PROGRAMS = 20;            // Maximum number of programs that can be registered
  const int MAX_NAME_LEN = 20;            // Maximum length of a program name

  // === Type definition ===
  typedef void (*ProgramFunc)();          // Function pointer type for programs

  // === Internal storage for registered programs ===
  static char programNames[MAX_PROGRAMS][MAX_NAME_LEN];   // Array to hold program names
  static ProgramFunc programFuncs[MAX_PROGRAMS];          // Array to hold function pointers
  static int programCount = 0;                            // Counter for how many programs are registered

  // === Example programs ===


  void helloProgram() {
    Serial.println(F("Hello from helloProgram!"));
  }


  void testFloatDoublePrecision() {
  // Größe ausgeben
  Serial.print("sizeof(float): ");
  Serial.println(sizeof(float));

  Serial.print("sizeof(double): ");
  Serial.println(sizeof(double));

  // Werte definieren
  float fVal = 1.1234567f;          
  double dVal = 1.123456789012345;  

  Serial.print("float Wert:  ");
  Serial.println(fVal, 7);  

  Serial.print("double Wert: ");
  Serial.println(dVal, 15); 

  // Differenz berechnen
  double diff = dVal - (double)fVal;
  Serial.print("Differenz double - float: ");
  Serial.println(diff, 15);
  }

  // === Function to register a new program ===
  // name: the string name to register under
  // func: a pointer to the function representing the program
  // returns: true if registration is successful, false if the registry is full
  bool registerProgram(const char* name, ProgramFunc func) {
    if (programCount >= MAX_PROGRAMS) return false;

    strncpy(programNames[programCount], name, MAX_NAME_LEN);
    programFuncs[programCount] = func;
    programCount++;
    return true;
  }

  // === Initialization function to be called in setup() ===
  // Registers all available programs
  void init() {
    //         (name to call the programm, the name of the funktion the programm has in this class)
    registerProgram("hello", helloProgram);
    registerProgram("fpu-check", testFloatDoublePrecision);
  }

  // === Utility function to list all registered programs ===
  // out: output stream to print to (default: Serial)
  void listAll(Stream& out = Serial) {
    out.println(F("== Registered Programs =="));
    for (int i = 0; i < programCount; i++) {
      out.println(programNames[i]);
    }
  }

  // === Executes a program by name ===
  // name: the program name to search and execute
  // out: output stream to print logs to (default: Serial)
  // returns: true if program was found and executed, false otherwise
  bool run(const char* name, Stream& out = Serial) {
    for (int i = 0; i < programCount; i++) {
      if (strcmp(programNames[i], name) == 0) {
        out.print(F("[RUNNING] "));
        out.println(name);
        programFuncs[i]();  // Call the registered function
        return true;
      }
    }
    out.print(F("[ERROR] Program not found: "));
    out.println(name);
    return false;
  }
};

#endif
