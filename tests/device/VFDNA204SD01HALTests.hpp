// Device-specific tests for VFDNA204SD01HAL
#pragma once

#include <Arduino.h>
#include "HAL/VFDNA204SD01HAL.h"
#include "tests/mocks/MockTransport.h"
#include "tests/framework/EmbeddedTest.h"

static void test_na204sd01_init_reset() {
  VFDNA204SD01HAL hal; MockTransport mock; hal.setTransport(&mock);
  ET_ASSERT_TRUE(hal.init());
  ET_ASSERT_GE((int)mock.size(), (int)1);
  ET_ASSERT_EQ((int)mock.at(0), (int)0x1F); // Reset
}

static void test_na204sd01_clear_home_pos() {
  VFDNA204SD01HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  mock.clear(); ET_ASSERT_TRUE(hal.clear()); ET_ASSERT_EQ((int)mock.at(0),(int)0x0D);
  mock.clear(); ET_ASSERT_TRUE(hal.cursorHome()); ET_ASSERT_EQ((int)mock.at(0),(int)0x0C);
  // Row 3 (0-based, 4th line), Col 5 -> base 0x3C + 5 = 0x41
  mock.clear(); ET_ASSERT_TRUE(hal.setCursorPos(3,5));
  ET_ASSERT_EQ((int)mock.size(), (int)2);
  ET_ASSERT_EQ((int)mock.at(0),(int)0x10);
  ET_ASSERT_EQ((int)mock.at(1),(int)0x41);
}

static void test_na204sd01_dimming_levels() {
  VFDNA204SD01HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  uint8_t codes[] = {0x00,0x20,0x40,0x60,0x80,0xFF};
  for (uint8_t lvl=0; lvl<6; ++lvl) {
    mock.clear();
    ET_ASSERT_TRUE(hal.setDimming(lvl));
    ET_ASSERT_EQ((int)mock.at(0),(int)0x04);
    ET_ASSERT_EQ((int)mock.at(1),(int)codes[lvl]);
  }
}

inline void register_VFDNA204SD01HAL_device_tests() {
  ET_ADD_TEST("NA204SD01.init_reset", test_na204sd01_init_reset);
  ET_ADD_TEST("NA204SD01.clear_home_pos", test_na204sd01_clear_home_pos);
  ET_ADD_TEST("NA204SD01.dimming", test_na204sd01_dimming_levels);
}

