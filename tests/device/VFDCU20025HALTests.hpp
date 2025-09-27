// Device-specific tests for VFDCU20025HAL
#pragma once

#include <Arduino.h>
#include "HAL/VFDCU20025HAL.h"
#include "tests/mocks/MockTransport.h"
#include "tests/framework/EmbeddedTest.h"

static void test_vfdcu20025_init_sequence() {
  VFDCU20025HAL hal; MockTransport mock; hal.setTransport(&mock);
  bool ok = hal.init();
  ET_ASSERT_TRUE(ok);
  ET_ASSERT_TRUE(mock.size() >= 4);
  ET_ASSERT_EQ((int)mock.at(0), (int)0x38);
  ET_ASSERT_EQ((int)mock.at(1), (int)0x0C);
  ET_ASSERT_EQ((int)mock.at(2), (int)0x01);
  ET_ASSERT_EQ((int)mock.at(3), (int)0x06);
}

static void test_vfdcu20025_clear_home_and_pos() {
  VFDCU20025HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  mock.clear(); ET_ASSERT_TRUE(hal.clear()); ET_ASSERT_EQ((int)mock.at(0),(int)0x01);
  mock.clear(); ET_ASSERT_TRUE(hal.cursorHome()); ET_ASSERT_EQ((int)mock.at(0),(int)0x02);
  mock.clear(); ET_ASSERT_TRUE(hal.setCursorPos(1, 3)); ET_ASSERT_EQ((int)mock.at(0),(int)0xC3);
}

static void test_vfdcu20025_dimming_brightness_set() {
  VFDCU20025HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  struct { uint8_t in; uint8_t expected; } cases[] = {{0,0},{1,1},{2,2},{3,3}};
  for (auto &c : cases) { mock.clear(); ET_ASSERT_TRUE(hal.setDimming(c.in)); ET_ASSERT_EQ((int)mock.at(0),(int)c.expected); }
}

inline void register_VFDCU20025HAL_device_tests() {
  ET_ADD_TEST("VFDCU20025.init_sequence", test_vfdcu20025_init_sequence);
  ET_ADD_TEST("VFDCU20025.clear_home_pos", test_vfdcu20025_clear_home_and_pos);
  ET_ADD_TEST("VFDCU20025.dimming", test_vfdcu20025_dimming_brightness_set);
}

