// Device-specific tests for VFD20T202HAL
#pragma once

#include <Arduino.h>
#include "HAL/VFD20T202HAL.h"
#include "tests/mocks/MockTransport.h"
#include "tests/framework/EmbeddedTest.h"

static void test_vfd20t202_init_sends_0x49() {
  VFD20T202HAL hal; MockTransport mock; hal.setTransport(&mock);
  bool ok = hal.init();
  ET_ASSERT_TRUE(ok);
  ET_ASSERT_EQ((int)mock.size(), (int)1);
  ET_ASSERT_EQ((int)mock.at(0), (int)0x49);
}

static void test_vfd20t202_clear_home_codes() {
  VFD20T202HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  mock.clear(); ET_ASSERT_TRUE(hal.clear()); ET_ASSERT_EQ((int)mock.at(0),(int)0x09);
  mock.clear(); ET_ASSERT_TRUE(hal.cursorHome()); ET_ASSERT_EQ((int)mock.at(0),(int)0x0C);
}

static void test_vfd20t202_setCursorPos_ESC_H_addr() {
  VFD20T202HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  mock.clear();
  bool ok = hal.setCursorPos(1, 3); // addr = 1*20 + 3 = 23 (0x17)
  ET_ASSERT_TRUE(ok);
  ET_ASSERT_EQ((int)mock.size(), (int)3);
  ET_ASSERT_EQ((int)mock.at(0), (int)0x1B);
  ET_ASSERT_EQ((int)mock.at(1), (int)0x48);
  ET_ASSERT_EQ((int)mock.at(2), (int)0x17);
}

inline void register_VFD20T202HAL_device_tests() {
  ET_ADD_TEST("VFD20T202.init_sends_0x49", test_vfd20t202_init_sends_0x49);
  ET_ADD_TEST("VFD20T202.clear_and_home", test_vfd20t202_clear_home_codes);
  ET_ADD_TEST("VFD20T202.setCursorPos_ESC_H_addr", test_vfd20t202_setCursorPos_ESC_H_addr);
}

