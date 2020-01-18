
#include "bidir_serial_module.h"
//#include <cmath>
#include <stdio.h>

namespace arduinoio {
namespace {

const unsigned long kCommunicationTimeout = 200000;  // 0.2s in usec

}  // namespace

BidirSerialRXModule::BidirSerialRXModule(
    ArduinoInterface *arduino,
    int address)
  : arduino_(arduino), address_(address), transmit_state_(READY_AND_DONE),
  receive_state_(READY_AND_DONE),
  last_communication_time_(arduino->micros()) {
}

const Message* BidirSerialRXModule::Tick() {
  // Receiving
  unsigned char next_rx_command = 0x00;
  if (!ReadFlushingErrors(&next_rx_command)) {
    if (arduino_->micros() - last_communication_time_ > kCommunicationTimeout) {
      SendError();
      last_communication_time_ = arduino_->micros();
    }
    return nullptr;
  }
  const unsigned char new_rx_state = next_rx_command & 0xf0;
  const unsigned char bytes_receiving = next_rx_command & 0x0f;
  bool finished = false;
  if (new_rx_state == ERROR) {
    ResetRX();
  } else {
    switch (new_rx_state) {
      case READY_AND_SENDING: {
        if (receive_state_ == SENT_LAST) {
          SendError();
          return nullptr;
        }
        unsigned char next_byte = 0x00;
        for (int i = 0; i < bytes_receiving; ++i) {
          if (finished) {
            printf("Message finished early.\n");
            SendError();
            return nullptr;
          }
          if (!ReadWithTimeout(&next_byte)) {
            printf("Failed to read expected byte %d of %d.\n", i, bytes_receiving);
            SendError();
            return nullptr;
          }
          finished = message_.AddByte(static_cast<unsigned char>(next_byte & 0xff));
        }
        receive_state_ = finished ? SENT_LAST : READY_AND_SENDING;
        break;
      }
      case READY_AND_DONE: {
        if (receive_state_ == READY_AND_SENDING) {
          // Terminated message early.
          SendError();
          return nullptr;
        }
        break;
      }
      case ERROR: {
        ResetRX();
        break;
      }
      default: {
        SendError();
        return nullptr;
      }
    }
  }

  // Transmitting
  switch (transmit_state_) {
    case SENT_LAST:
      transmit_state_ = READY_AND_DONE;
      // FALLTHROUGH_INTENDED
    case READY_AND_DONE:
      arduino_->write(transmit_state_);
      break;
    case READY_AND_SENDING: {
      const int kMaxBytesToSend = 0x0f;
      const int total_bytes_left = length_sending_ - next_send_index_;
      const int num_bytes_to_send = total_bytes_left < kMaxBytesToSend ?
          total_bytes_left : kMaxBytesToSend;
      arduino_->write(transmit_state_ | num_bytes_to_send);
      for (int i = 0; i < num_bytes_to_send; ++i, ++next_send_index_) {
        arduino_->write(bytes_sending_[next_send_index_]);
      }
      if (next_send_index_ == length_sending_) {
        transmit_state_ = SENT_LAST;
      }
      break;
    }
    default:
      SendError();
      break;
  }
  last_communication_time_ = arduino_->micros();
  return finished ? &message_ : nullptr;
}

bool BidirSerialRXModule::ReadFlushingErrors(unsigned char *output) {
  bool read_something = false;
  while (arduino_->available()) {
    read_something = true;
    *output = (unsigned char)arduino_->read() & 0xff;
    if (*output != ERROR) return true;
  }
  return read_something;
}

bool BidirSerialRXModule::ReadWithTimeout(unsigned char *output) {
  const unsigned long timeout = arduino_->micros() + 200000LL;
  while (!arduino_->available()) {
    if (arduino_->micros() > timeout) return false;
  }
  *output = (unsigned char)arduino_->read() & 0xff;
  return true;
}

void BidirSerialRXModule::SendError() {
  arduino_->write(ERROR);
  ResetRX();
}

void BidirSerialRXModule::ResetRX() {
  next_send_index_ = 0;
  message_.Clear();
  receive_state_ = READY_AND_DONE;
  if (transmit_state_ != READY_AND_DONE) {
    transmit_state_ = READY_AND_SENDING;
  }
}

bool BidirSerialRXModule::AcceptMessage(const Message &message) {
  if (transmit_state_ != READY_AND_DONE) {
    // Already sending something else.
    return false;
  }
  transmit_state_ = READY_AND_SENDING;
  next_send_index_ = 0;
  int length;
  const char* command = message.command(&length);
  if (message.address() == address_) {
    // Local message doesn't need to be sent.
    return false;
  }
  bytes_sending_[0] = message.address_length();
  bytes_sending_[1] = length;
  bytes_sending_[2] = 0;
  bytes_sending_[3] = message.address();
  for (int i = 0; i < length; ++i) {
    bytes_sending_[4 + i] = command[i];
  }
  length_sending_ = length + 4;
  const bool kSendChecksums = false;
  if (kSendChecksums) {
    length_sending_ += 2;
    bytes_sending_[length + 4] = message.second_checksum();
    bytes_sending_[length + 4 + 1] = message.first_checksum();
  }
  return true;
}

}  // namespace arduinoio
