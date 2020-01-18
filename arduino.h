#ifndef hardware_simulator_arduino_h_
#define hardware_simulator_arduino_h_

// This interface mimics Arduino.h, so it can be used as a drop-in replacement.
namespace arduinoio {

class ArduinoInterface {
 public:
   virtual ~ArduinoInterface() {}

  virtual void digitalWrite(const unsigned int pin, bool value) = 0;
  virtual bool digitalRead(const unsigned int pin) = 0;
  virtual unsigned long micros() = 0;

  virtual void write(const unsigned char c) = 0;
  virtual int read() = 0;
  virtual bool available() = 0;
};

}  // namespace arduinoio

#endif // hardware_simulator_arduino_h_
