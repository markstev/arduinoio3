#include "message.h"
#include <stdio.h>

namespace arduinoio {

const bool kUseChecksums = false;

Message::Message() {
  Clear();
}

Message::Message(unsigned char address, unsigned char command_length,
      const char* command)
    : address_(address), address_length_(1), command_length_(command_length) {
  for (int i = 0; i < command_length_; ++i) {
    command_[i] = command[i];
  }
}

Message::~Message() {}

void Message::Reset(unsigned char address, unsigned char command_length,
      const char* command) {
  address_ = address;
  address_length_ = 1;
  command_length_ = command_length;
  for (int i = 0; i < command_length_; ++i) {
    command_[i] = command[i];
  }
}

void Message::Clear() {
  has_address_length_ = false;
  has_command_length_ = false;
  has_timeout_ = false;
  byte_index_ = 0;
  address_ = 0;
  command_length_ = 0;
  first_checksum_ = 0;
  second_checksum_ = 0;
  error_ = false;
}

bool Message::Empty() {
  return has_address_length_;
}

bool Message::AddByte(char next_byte) {
  if (!has_address_length_) {
    address_length_ = next_byte;
    has_address_length_ = true;
  } else if (!has_command_length_) {
    command_length_ = next_byte;
    has_command_length_ = true;
  } else if (!has_timeout_) {
    timeout_ = next_byte;
    has_timeout_ = true;
  } else if (byte_index_ < address_length_) {
    int address_tmp = next_byte;
    address_ |= address_tmp << (byte_index_ * 8);
    byte_index_++;
  } else if (byte_index_ - address_length_ < command_length_) {
    command_[byte_index_ - address_length_] = next_byte;
    byte_index_++;
    if (!kUseChecksums && byte_index_ - address_length_ == command_length_) {
      return true;
    }
  } else if (byte_index_ - address_length_ == command_length_) {
    byte_index_++;
    if (next_byte != second_checksum_) {
      error_ = true;
    }
    return false;
  } else if (byte_index_ - address_length_ == command_length_ + 1) {
    byte_index_++;
    if (next_byte != first_checksum_) {
      error_ = true;
    }
    return !error_;
  } else {
    // error
  }
  first_checksum_ = (first_checksum_ + next_byte) & 0xff;
  second_checksum_ = (second_checksum_ + first_checksum_) & 0xff;
  return false;
}

}  // namespace arduinoio
