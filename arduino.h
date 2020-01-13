#ifndef hardware_simulator_arduino_h_
#define hardware_simulator_arduino_h_

// This interface mimics Arduino.h, so it can be used as a drop-in replacement.

#define USE_SIMULATOR true
#ifdef USE_SIMULATOR
  #include "arduino_simulator.h"
  SerialAbstraction Serial;
#else
  #include "Arduino.h"
#endif

#endif
