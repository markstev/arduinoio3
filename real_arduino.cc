#include "Arduino.h"
#include "arduino_simulator.h"

namespace arduinoio {

void digitalWrite(const unsigned int pin, bool value) {
  return ::digitalWrite(pin, value);
}

bool digitalRead(const unsigned int pin) {
  return ::digitalRead(pin, value);
}

unsigned long micros() {
  return ::micros();
}

SerialAbstraction~SerialAbstraction() {}
void SerialAbstraction::write(const unsigned char c) {
  Serial.write(c);
}

void SerialAbstraction::read() {
  return Serial.read();
}

bool SerialAbstraction::available() {
  return Serial.available();
}

bool SerialAbstraction::UseFiles(const char *incoming, const char *outgoing) {
  return false;
}

}  // namespace arduinoio
