#ifndef JDUINO_UC_ARDUINOIO_H_
#define JDUINO_UC_ARDUINOIO_H_

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

#include "message.h"
#include "uc_module.h"

namespace arduinoio {

class ArduinoIO {
 public:
  ArduinoIO();

  ~ArduinoIO();

  void Add(UCModule *module);
  void HandleLoopMessages();

 private:
  UCModule* modules_[10];
  int num_modules_;
  Message* first_message_;
};

}  // namespace arduinoio

#endif  // JDUINO_UC_ARDUINOIO_H_
