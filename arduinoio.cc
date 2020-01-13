#include "arduinoio.h"

namespace arduinoio {

ArduinoIO::ArduinoIO() : num_modules_(0),
      first_message_(NULL) {}

ArduinoIO::~ArduinoIO() {
  for (int i = 0; i < num_modules_; ++i) {
    delete modules_[i];
  }
}

void ArduinoIO::Add(UCModule *module) {
  modules_[num_modules_] = module;
  num_modules_++;
}

void ArduinoIO::HandleLoopMessages() {
  for (int i = 0; i < num_modules_; ++i) {
    const Message* message;
    if (first_message_ != NULL) {
      message = first_message_;
    } else {
      message = modules_[i]->Tick();
    }
    if (message != NULL) {
      for (int j = 0; j < num_modules_; ++j) {
        modules_[j]->AcceptMessage(*message);
      }
    }
    if (first_message_ != NULL) {
      delete first_message_;
      first_message_ = NULL;
    }
  }
}

}  // namespace arduinoio
