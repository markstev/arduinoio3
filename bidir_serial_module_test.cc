#include <gtest/gtest.h>
#include "arduino_simulator.h"
#include "bidir_serial_module.h"

namespace arduinoio {
namespace {

TEST(BidirSerialModuleTest, SendsErrorEventually) {
  FakeArduino module_serial;
  FakeArduino test_serial;
  ASSERT_TRUE(module_serial.UseFiles("/tmp/bsmt_1_a", "/tmp/bsmt_1_b"));
  ASSERT_TRUE(test_serial.UseFiles("/tmp/bsmt_1_b", "/tmp/bsmt_1_a"));
  BidirSerialRXModule module(&module_serial, 0);
  while (module_serial.micros() < 150000LL) {
    EXPECT_EQ(nullptr, module.Tick());
    EXPECT_EQ(false, test_serial.available());
  }
  while (module_serial.micros() < 210000LL);
  EXPECT_EQ(nullptr, module.Tick());
  EXPECT_EQ(true, test_serial.available());
  EXPECT_EQ(ERROR, test_serial.read());
}

TEST(BidirSerialModuleTest, SendsReadyInResponse) {
  FakeArduino module_serial;
  FakeArduino test_serial;
  ASSERT_TRUE(module_serial.UseFiles("/tmp/bsmt_2_a", "/tmp/bsmt_2_b"));
  ASSERT_TRUE(test_serial.UseFiles("/tmp/bsmt_2_b", "/tmp/bsmt_2_a"));
  BidirSerialRXModule module(&module_serial, 0);
  EXPECT_EQ(nullptr, module.Tick());
  EXPECT_EQ(false, test_serial.available());
  test_serial.write(READY_AND_DONE);
  EXPECT_EQ(nullptr, module.Tick());
  EXPECT_EQ(READY_AND_DONE, test_serial.read());
  EXPECT_EQ(false, test_serial.available());
}

TEST(BidirSerialModuleTest, TransmitOneMessage) {
  FakeArduino module_serial;
  FakeArduino test_serial;
  ASSERT_TRUE(module_serial.UseFiles("/tmp/bsmt_3_a", "/tmp/bsmt_3_b"));
  ASSERT_TRUE(test_serial.UseFiles("/tmp/bsmt_3_b", "/tmp/bsmt_3_a"));
  BidirSerialRXModule module(&module_serial, 0);
  EXPECT_EQ(nullptr, module.Tick());
  const char* command = "Hello!";
  Message send_message(1, 6, command);
  EXPECT_EQ(true, module.AcceptMessage(send_message));
  test_serial.write(READY_AND_DONE);
  EXPECT_EQ(nullptr, module.Tick());
  EXPECT_EQ(true, test_serial.available());
  EXPECT_EQ(READY_AND_SENDING, test_serial.read() & 0xf0);
  Message received_message;
  bool done = false;
  for (int i = 0; i < 100; ++i) {
    if (!test_serial.available()) {
      break;
    }
    const char c = test_serial.read();
    if (c == -1) break;
    printf("char = %c\n", c);
    EXPECT_EQ(false, received_message.error());
    EXPECT_EQ(false, done);
    done = received_message.AddByte(c);
  }
  ASSERT_EQ(true, done);
  EXPECT_EQ(1, received_message.address_length());
  EXPECT_EQ(1, received_message.address());
  int command_length;
  EXPECT_EQ(0, strncmp(command, received_message.command(&command_length), command_length));
  EXPECT_EQ(6, command_length);

  test_serial.write(READY_AND_DONE);
  EXPECT_EQ(nullptr, module.Tick());
  EXPECT_EQ(READY_AND_DONE, test_serial.read());
}

TEST(BidirSerialModuleTest, ReceiveOneMessage) {
  FakeArduino module_serial;
  FakeArduino test_serial;
  ASSERT_TRUE(module_serial.UseFiles("/tmp/bsmt_4_a", "/tmp/bsmt_4_b"));
  ASSERT_TRUE(test_serial.UseFiles("/tmp/bsmt_4_b", "/tmp/bsmt_4_a"));
  BidirSerialRXModule module(&module_serial, 0);
  EXPECT_EQ(nullptr, module.Tick());
  const char* command = "Hello!";
  Message send_message(1, 6, command);
  test_serial.write(READY_AND_SENDING | (4 + 6));
  test_serial.write(1);  // address length
  test_serial.write(6);  // length
  test_serial.write(0);  // ???
  test_serial.write(1);  // address
  for (int i = 0; i < 6; ++i) {
    test_serial.write(command[i]);
  }
  const Message *received = module.Tick();
  EXPECT_EQ(READY_AND_DONE, test_serial.read());
  EXPECT_NE(received, nullptr);
  EXPECT_EQ(1, received->address());
  EXPECT_EQ(1, received->address_length());
  int command_length;
  EXPECT_EQ(0, strncmp("Hello!", received->command(&command_length), command_length));
  EXPECT_EQ(6, command_length);
}

TEST(BidirSerialModuleTest, TransmitReceiveSameTime) {
}

TEST(BidirSerialModuleTest, TransmitReceiveSameTimeTwoMessages) {
}

TEST(BidirSerialModuleTest, DualModulePair) {
}

}  // namespace
}  // namespace arduinoio
