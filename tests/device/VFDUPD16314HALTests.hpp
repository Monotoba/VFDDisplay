// Device-specific tests for VFDUPD16314HAL
#pragma once

#include <Arduino.h>
#include "HAL/VFDUPD16314HAL.h"
#include "tests/mocks/MockTransport.h"
#include "tests/framework/EmbeddedTest.h"

static void test_upd16314_init_sequence() {
  VFDUPD16314HAL hal; MockTransport mock; hal.setTransport(&mock);
  bool ok = hal.init();
  ET_ASSERT_TRUE(ok);
  ET_ASSERT_TRUE(mock.size() >= 4);
  ET_ASSERT_EQ((int)mock.at(0), (int)0x38);
  ET_ASSERT_EQ((int)mock.at(1), (int)0x0C);
  ET_ASSERT_EQ((int)mock.at(2), (int)0x01);
  ET_ASSERT_EQ((int)mock.at(3), (int)0x06);
}

static void test_upd16314_clear_home_pos() {
  VFDUPD16314HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  mock.clear(); ET_ASSERT_TRUE(hal.clear()); ET_ASSERT_EQ((int)mock.at(0),(int)0x01);
  mock.clear(); ET_ASSERT_TRUE(hal.cursorHome()); ET_ASSERT_EQ((int)mock.at(0),(int)0x02);
  mock.clear(); ET_ASSERT_TRUE(hal.setCursorPos(1,3)); ET_ASSERT_EQ((int)mock.at(0),(int)0xC3);
}

static void test_upd16314_dimming_function_set() {
  VFDUPD16314HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  struct { uint8_t lvl; uint8_t cmd; } cases[] = {{0,0x38},{1,0x39},{2,0x3A},{3,0x3B}};
  for (auto &c: cases) { mock.clear(); ET_ASSERT_TRUE(hal.setDimming(c.lvl)); ET_ASSERT_EQ((int)mock.at(0),(int)c.cmd); }
}

inline void register_VFDUPD16314HAL_device_tests() {
  ET_ADD_TEST("uPD16314.init_sequence", test_upd16314_init_sequence);
  ET_ADD_TEST("uPD16314.clear_home_pos", test_upd16314_clear_home_pos);
  ET_ADD_TEST("uPD16314.dimming", test_upd16314_dimming_function_set);
}

