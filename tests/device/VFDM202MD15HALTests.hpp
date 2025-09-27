// Device-specific tests for VFDM202MD15HAL
#pragma once

#include <Arduino.h>
#include "HAL/VFDM202MD15HAL.h"
#include "tests/mocks/MockTransport.h"
#include "tests/framework/EmbeddedTest.h"

static void test_m202_init_sequence() {
  VFDM202MD15HAL hal; MockTransport mock; hal.setTransport(&mock);
  ET_ASSERT_TRUE(hal.init());
  ET_ASSERT_TRUE(mock.size() >= 4);
  ET_ASSERT_EQ((int)mock.at(0), (int)0x38);
  ET_ASSERT_EQ((int)mock.at(1), (int)0x0C);
  ET_ASSERT_EQ((int)mock.at(2), (int)0x01);
  ET_ASSERT_EQ((int)mock.at(3), (int)0x06);
}

static void test_m202_clear_home_pos() {
  VFDM202MD15HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  mock.clear(); ET_ASSERT_TRUE(hal.clear()); ET_ASSERT_EQ((int)mock.at(0),(int)0x01);
  mock.clear(); ET_ASSERT_TRUE(hal.cursorHome()); ET_ASSERT_EQ((int)mock.at(0),(int)0x02);
  mock.clear(); ET_ASSERT_TRUE(hal.setCursorPos(1, 3)); ET_ASSERT_EQ((int)mock.at(0),(int)0xC3);
}

static void test_m202_dimming_function_set() {
  VFDM202MD15HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  struct { uint8_t lvl; uint8_t cmd; } cases[] = {{0,0x38},{1,0x39},{2,0x3A},{3,0x3B}};
  for (auto &c: cases) { mock.clear(); ET_ASSERT_TRUE(hal.setDimming(c.lvl)); ET_ASSERT_EQ((int)mock.at(0),(int)c.cmd); }
}

inline void register_VFDM202MD15HAL_device_tests() {
  ET_ADD_TEST("M202MD15.init_sequence", test_m202_init_sequence);
  ET_ADD_TEST("M202MD15.clear_home_pos", test_m202_clear_home_pos);
  ET_ADD_TEST("M202MD15.dimming", test_m202_dimming_function_set);
}

