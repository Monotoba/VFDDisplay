// Device-specific tests for VFDM204SD01AHAL (Futaba 20x4 SD01A)
#pragma once

#include <Arduino.h>
#include "HAL/VFDM204SD01AHAL.h"
#include "tests/mocks/MockTransport.h"
#include "tests/framework/EmbeddedTest.h"

static void test_m204sd01a_clear_home_pos() {
  VFDM204SD01AHAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  mock.clear(); ET_ASSERT_TRUE(hal.clear()); ET_ASSERT_EQ((int)mock.at(0),(int)0x0D);
  mock.clear(); ET_ASSERT_TRUE(hal.cursorHome()); ET_ASSERT_EQ((int)mock.at(0),(int)0x0C);
  mock.clear(); ET_ASSERT_TRUE(hal.setCursorPos(2, 5)); ET_ASSERT_EQ((int)mock.size(), (int)2); ET_ASSERT_EQ((int)mock.at(0),(int)0x10); ET_ASSERT_EQ((int)mock.at(1),(int)(0x14+5));
}

static void test_m204sd01a_dimming_levels() {
  VFDM204SD01AHAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  uint8_t codes[] = {0x00,0x40,0x80,0xFF};
  for (uint8_t lvl=0; lvl<4; ++lvl) { mock.clear(); ET_ASSERT_TRUE(hal.setDimming(lvl)); ET_ASSERT_EQ((int)mock.at(0),(int)0x04); ET_ASSERT_EQ((int)mock.at(1),(int)codes[lvl]); }
}

inline void register_VFDM204SD01AHAL_device_tests() {
  ET_ADD_TEST("M204SD01A.clear_home_pos", test_m204sd01a_clear_home_pos);
  ET_ADD_TEST("M204SD01A.dimming", test_m204sd01a_dimming_levels);
}

