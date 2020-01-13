#ifndef hardware_simulator_arduino_simulator_h_
#define hardware_simulator_arduino_simulator_h_

#include <stdio.h>

// This interface mimics Arduino.h, so it can be used as a drop-in replacement.
namespace arduinoio {

void digitalWrite(const unsigned int pin, bool value);
bool digitalRead(const unsigned int pin);
unsigned long micros();

// Interface for a variety of ways to check the current time, including fake clocks.
class Clock {
 public:
  ~Clock() {}
  virtual unsigned long micros() const = 0;
};

// By default, a real clock is used. To use a fake clock instead, call
// UseFakeClock() or GetFakeClock().
class FakeClock : public Clock {
 public:
  FakeClock();
  unsigned long micros() const override;
  void IncrementTime(const unsigned long micros);

 private:
  unsigned long current_time_micros_;
};

FakeClock* GetFakeClock();

class SerialAbstraction {
 public:
   ~SerialAbstraction();
  void write(const unsigned char c);
  int read();
  bool available();

  bool UseFiles(const char *incoming, const char *outgoing);

 private:
  FILE *incoming_file_, *outgoing_file_;
  int next_byte_ = EOF;
};

}  // namespace arduinoio

#endif
