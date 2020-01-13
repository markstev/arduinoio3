#include "uc_module.h"
#include "arduinoio.h"
#include "bidir_serial_module.h"

arduinoio::ArduinoIO arduino_io;
void setup() {
  const uint16_t kNumLeds = 300;
  digitalWrite(8, HIGH);
  //arduino_io.Add(new nebree8::LedModule(kNumLeds, LED_SIGNAL_PIN));
  Serial.begin(9600);
  arduino_io.Add(new arduinoio::SerialRXModule(0, false));
  arduino_io.Add(new armnyak::MotorBankModule());
}

void loop() {
  arduino_io.HandleLoopMessages();
}
