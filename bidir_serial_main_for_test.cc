#include "arduino_simulator.h"
#include "bidir_serial_module.h"
#include "message.h"
#include <cstring>
#include <stdlib.h>

const int kOtherAddress = 1;

int main(int argc, char **argv) {
  // arg1 = incoming serial file.
  // arg2 = outgoing serial file.
  if (argc < 4) {
    printf("Call with args:\nbidir_serial_main <incoming serial filename> <outgoing serial filename> <address>");
    return 1;
  }
  arduinoio::SerialAbstraction module_serial;
  module_serial.UseFiles(argv[1], argv[2]);
  arduinoio::BidirSerialRXModule module(&module_serial, atoi(argv[3]));
  char message[7] = "COUNT\0";
  arduinoio::Message counter_message(kOtherAddress, 6, message);
  bool do_find_replace = false;
  arduinoio::Message find_replace_message(kOtherAddress, 6, message);
  while (true) {
    const arduinoio::Message *incoming = module.Tick();
    if (incoming != nullptr) {
      int length;
      const char *incoming_command = incoming->command(&length);
      if (0 == strncmp("SET_COUNT", incoming_command, 9) && length == 10) {
        message[5] = incoming->command(&length)[9];
      } else if (0 == strncmp("REWRITE", incoming_command, 7) && length >= 10) {
        char rewritten[length];
        char find = incoming_command[7];
        char replace = incoming_command[8];
        for (int i = 0; i < length; ++i) {
          if (i < 9 || incoming_command[i] != find) {
            rewritten[i] = incoming_command[i];
          } else {
            rewritten[i] = replace;
          }
        }
        find_replace_message.Reset(kOtherAddress, length, rewritten);
        do_find_replace = true;
      }
    }
    if (do_find_replace) {
      if (module.AcceptMessage(find_replace_message)) {
        do_find_replace = false;
      }
    } else {
      if (module.AcceptMessage(counter_message)) {
        message[5]++;
        counter_message.Reset(kOtherAddress, 6, message);
      }
    }
  }
  return 0;
}
