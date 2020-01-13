#ifndef ARDUINOIO2_UC_MESSAGE_H_
#define ARDUINOIO2_UC_MESSAGE_H_

namespace arduinoio {

const int MAX_BUFFER_SIZE = 50;

class Message {
 public:
  Message();

  Message(unsigned char address, unsigned char command_length,
      const char* command);

  ~Message();

  void Reset(unsigned char address, unsigned char command_length,
      const char* command);
  
  void Clear();

  bool Empty();
  
  // Returns true when the message is complete.
  bool AddByte(char next_byte);
  
  int address() const {
    return address_;
  }

  int address_length() const {
    return address_length_;
  }
  
  const char* command(int *length) const {
    *length = command_length_;
    return command_;
  }

  bool error() const {
    return error_;
  }

  char first_checksum() const {
    return first_checksum_;
  }

  char second_checksum() const {
    return second_checksum_;
  }
  
 private:
  int address_;
  int address_length_;
  int command_length_;
  int timeout_;
  
  char command_[MAX_BUFFER_SIZE];
  
  // Used for constructing messages.
  bool has_address_length_;
  bool has_command_length_;
  bool has_timeout_;
  int byte_index_;
  int first_checksum_;
  int second_checksum_;
  bool error_;
};

}  // namespace arduinoio

#endif  // ARDUINOIO2_UC_MESSAGE_H_
