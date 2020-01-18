#ifndef ARDUINOIO_BIDIR_SERIAL_MODULE_H
#define ARDUINOIO_BIDIR_SERIAL_MODULE_H

#include "message.h"
#include "uc_module.h"
#include "arduino.h"

namespace arduinoio {

const unsigned char READY_AND_DONE = 0x50;
const unsigned char READY_AND_SENDING = 0x60;
const unsigned char SENT_LAST = 0x70;
const unsigned char ERROR = 0x80;

// This communication protocol supports bidirectional communication with
// multiple master devices.
//
// TODO: we can support faster communication by passing more bytes per ack.
//
// Protocol:
// 1) Device A sends a ready char indicating whether it will write part of a message.
// 2) Device A sends the indicated char of the message, as indicated.
// 3) Device B responds with a ready char indicating whether it will write part of a message.
// 4) Device B sends the indicated char of the message, as indicated.
// Repeat...
//
// Error Handshakes:
// 1) Device A sends an error code.
// 2) Device B resets any active sends and replies with a ready code and any message.
//
// Initialization:
// 1) All devices are waiting
// 2) If a device waits for 200ms with no signal received, it will send an error byte,
//   triggering the error handshake.
//
// At any point, one device may respond to the other with an error char. This
// indicates the incoming message was not processed and should resend.
//
// At least one READY_AND_DONE should come after each message sent.
//
// Ready, send nothing char: 0x52
// Ready, send more char: 0x53
// Error, resend: 0x45
class BidirSerialRXModule : public UCModule {
 public:
  BidirSerialRXModule(ArduinoInterface *arduino, int address);

  // Handles any incoming/outgoing transmissions. Returns a non-null message on the tick
  // when an incoming message is completed.
  const Message* Tick() override;

  // Sends an error to the other end of the serial link.
  void SendError();

  // Resets the receive and transmit state, restarting nthe process of being sent or received.
  void ResetRX();

  // Accepts any messages that need to be transmitted, if not already sending.
  bool AcceptMessage(const Message &message) override;

 private:
  bool ReadFlushingErrors(unsigned char *output);
  bool ReadWithTimeout(unsigned char *output);

  ArduinoInterface *arduino_;
  int address_;
  unsigned char transmit_state_;
  unsigned char receive_state_;
  Message incoming_message_;
  unsigned char bytes_sending_[MAX_BUFFER_SIZE];
  int next_send_index_;
  int length_sending_;
  unsigned long last_communication_time_;
  Message message_;
};

}  // namespace arduinoio

#endif
