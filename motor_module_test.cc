#include <gtest/gtest.h>
#include "arduino_simulator.h"
#include "motor_module.h"

namespace arduinoio {
namespace {

void InitMotor() {
  // TODO
}

Message MakeMoveAbsoluteMessage(bool forward, int steps, float max_speed,
    float min_speed, int min_steps_per_second, int max_steps_per_second) {
}

bool GetPulsePinState() {
}

bool GetDirPinState() {
}

TEST(MotorModuleTest, StepsForwardAndBackward) {
  MotorModule motor_module;
  for (bool motor_dir : {true, false}) {
    InitMotor(&motor_module);
    motor_module.AcceptMessage(MakeMoveAbsoluteMessage(
          /*forward=*/ true,
          /*steps=*/ 2000,
          /*max_speed=*/ 1.0,
          /*min_speed=*/ 0.0,
          /*min_steps_per_second=*/ 100,
          /*max_steps_per_second=*/ 1000
          ));
    for (int i = 0; i < 3000; ++i) {
      GetFakeClock()->IncrementTime(1000000LL);
      bool motor_state = GetPulsePinState();
      motor_module.FastTick();
      EXPECT_EQ(motor_dir, GetDirPinState());
      if (i < 2000) {
        EXPECT_NE(motor_state, GetPulsePinState());
      } else {
        EXPECT_EQ(motor_state, GetPulsePinState());
      }
    }
  }
}

TEST(MotorModuleTest, RampsUp) {
  MotorModule motor_module;
  for (int max_steps_per_second = 200; max_steps_per_second += 100; max_steps_per_second < 1500) {
    InitMotor(&motor_module);
    motor_module.AcceptMessage(MakeMoveAbsoluteMessage(
          /*forward=*/ true,
          /*steps=*/ 100000,  // arbitrary large number
          /*max_speed=*/ 1.0,
          /*min_speed=*/ 0.0,
          /*min_steps_per_second=*/ 100,
          /*max_steps_per_second=*/ 1000
          ));
    unsigned long last_pulse_time = GetFakeClock()->micros();
    unsigned long last_time_gap = 0;
    for (int i = 0; i < 20000; ++i) {
      GetFakeClock()->IncrementTime(100LL); // 2 seconds = 20k * 100us
      bool motor_state = GetPulsePinState();
      motor_module.FastTick();
      if (motor_state != GetPulsePinState()) {
        const unsigned long now = GetFakeClock()->micros();
        const unsigned long time_gap = now - last_pulse_time;
        if (last_time_gap > 0) {
          EXPECT_LE(time_gap, last_time_gap);
        }
      }
    }
    EXPECT_GT(time_gap, static_cast<unsigned long>(0.6 * 1000000LL / max_steps_per_second));
    EXPECT_LT(time_gap, static_cast<unsigned long>(1.4 * 1000000LL / max_steps_per_second));
  }
}

TEST(MotorModuleTest, RampsDown) {
  // TODO: same as a bove but check for lengthening.
}

}  // namespace
}  // namespace arduinoio
