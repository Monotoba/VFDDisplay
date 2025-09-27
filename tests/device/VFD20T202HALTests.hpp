// Device-specific tests for VFD20T202HAL
#pragma once

#include <Arduino.h>
#include "HAL/VFD20T202HAL.h"
#include "tests/mocks/MockTransport.h"
#include "tests/framework/EmbeddedTest.h"

static void test_vfd20t202_init_sequence() {
  VFD20T202HAL hal; MockTransport mock; hal.setTransport(&mock);
  bool ok = hal.init();
  ET_ASSERT_TRUE(ok);
  // Expect at least: 0x38 (function set), 0x0C (display on), 0x01 (clear), 0x06 (entry mode)
  ET_ASSERT_TRUE(mock.size() >= 4);
  ET_ASSERT_EQ((int)mock.at(0), (int)0x38);
  ET_ASSERT_EQ((int)mock.at(1), (int)0x0C);
  ET_ASSERT_EQ((int)mock.at(2), (int)0x01);
  ET_ASSERT_EQ((int)mock.at(3), (int)0x06);
}

static void test_vfd20t202_clear_home_codes() {
  VFD20T202HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  mock.clear(); ET_ASSERT_TRUE(hal.clear()); ET_ASSERT_EQ((int)mock.at(0),(int)0x01);
  mock.clear(); ET_ASSERT_TRUE(hal.cursorHome()); ET_ASSERT_EQ((int)mock.at(0),(int)0x02);
}

static void test_vfd20t202_setCursorPos_set_ddram() {
  VFD20T202HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  mock.clear();
  bool ok = hal.setCursorPos(1, 3); // DDRAM base row1=0x40 => 0x40+3=0x43; command is 0x80|0x43=0xC3
  ET_ASSERT_TRUE(ok);
  ET_ASSERT_EQ((int)mock.size(), (int)1);
  ET_ASSERT_EQ((int)mock.at(0), (int)0xC3);
}

inline void register_VFD20T202HAL_device_tests() {
  ET_ADD_TEST("VFD20T202.init_sequence", test_vfd20t202_init_sequence);
  ET_ADD_TEST("VFD20T202.clear_and_home", test_vfd20t202_clear_home_codes);
  ET_ADD_TEST("VFD20T202.setCursorPos_set_ddram", test_vfd20t202_setCursorPos_set_ddram);
}
