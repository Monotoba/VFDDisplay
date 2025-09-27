// Device-specific tests for VFDPT6314HAL
#pragma once

#include <Arduino.h>
#include "HAL/VFDPT6314HAL.h"
#include "tests/mocks/MockTransport.h"
#include "tests/framework/EmbeddedTest.h"

static void test_pt6314_init_sequence() {
  VFDPT6314HAL hal; MockTransport mock; hal.setTransport(&mock);
  bool ok = hal.init();
  ET_ASSERT_TRUE(ok);
  ET_ASSERT_TRUE(mock.size() >= 4);
  ET_ASSERT_EQ((int)mock.at(0), (int)0x38);
  ET_ASSERT_EQ((int)mock.at(1), (int)0x0C);
  ET_ASSERT_EQ((int)mock.at(2), (int)0x01);
  ET_ASSERT_EQ((int)mock.at(3), (int)0x06);
}

static void test_pt6314_clear_home_pos() {
  VFDPT6314HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  mock.clear(); ET_ASSERT_TRUE(hal.clear()); ET_ASSERT_EQ((int)mock.at(0),(int)0x01);
  mock.clear(); ET_ASSERT_TRUE(hal.cursorHome()); ET_ASSERT_EQ((int)mock.at(0),(int)0x02);
  mock.clear(); ET_ASSERT_TRUE(hal.setCursorPos(1,3)); ET_ASSERT_EQ((int)mock.at(0),(int)0xC3);
}

inline void register_VFDPT6314HAL_device_tests() {
  ET_ADD_TEST("PT6314.init_sequence", test_pt6314_init_sequence);
  ET_ADD_TEST("PT6314.clear_home_pos", test_pt6314_clear_home_pos);
}

