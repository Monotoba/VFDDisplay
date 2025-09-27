// Device-specific tests for VFDM0216MDHAL
#pragma once

#include <Arduino.h>
#include "HAL/VFDM0216MDHAL.h"
#include "tests/mocks/MockTransport.h"
#include "tests/framework/EmbeddedTest.h"

static void test_m0216_init_sequence() {
  VFDM0216MDHAL hal; MockTransport mock; hal.setTransport(&mock);
  ET_ASSERT_TRUE(hal.init());
  ET_ASSERT_TRUE(mock.size() >= 4);
  ET_ASSERT_EQ((int)mock.at(0), (int)0x38);
  ET_ASSERT_EQ((int)mock.at(1), (int)0x0C);
  ET_ASSERT_EQ((int)mock.at(2), (int)0x01);
  ET_ASSERT_EQ((int)mock.at(3), (int)0x06);
}

static void test_m0216_clear_home_pos() {
  VFDM0216MDHAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  mock.clear(); ET_ASSERT_TRUE(hal.clear()); ET_ASSERT_EQ((int)mock.at(0),(int)0x01);
  mock.clear(); ET_ASSERT_TRUE(hal.cursorHome()); ET_ASSERT_EQ((int)mock.at(0),(int)0x02);
  mock.clear(); ET_ASSERT_TRUE(hal.setCursorPos(1, 3)); ET_ASSERT_EQ((int)mock.at(0),(int)0xC3);
}

static void test_m0216_dimming_function_set() {
  VFDM0216MDHAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  struct { uint8_t lvl; uint8_t cmd; } cases[] = {{0,0x38},{1,0x39},{2,0x3A},{3,0x3B}};
  for (auto &c: cases) { mock.clear(); ET_ASSERT_TRUE(hal.setDimming(c.lvl)); ET_ASSERT_EQ((int)mock.at(0),(int)c.cmd); }
}

inline void register_VFDM0216MDHAL_device_tests() {
  ET_ADD_TEST("M0216MD.init_sequence", test_m0216_init_sequence);
  ET_ADD_TEST("M0216MD.clear_home_pos", test_m0216_clear_home_pos);
  ET_ADD_TEST("M0216MD.dimming", test_m0216_dimming_function_set);
}

