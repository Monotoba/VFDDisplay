// Device-specific tests for VFDPT6302HAL
#pragma once

#include <Arduino.h>
#include "HAL/VFDPT6302HAL.h"
#include "tests/mocks/MockTransport.h"
#include "tests/framework/EmbeddedTest.h"

static void test_pt6302_init_sequence() {
  VFDPT6302HAL hal; MockTransport mock; hal.setTransport(&mock);
  ET_ASSERT_TRUE(hal.init());
  // Expect at least three bytes: No. of digits (0x60|7), Duty (0x50|7), Lights normal (0x70)
  ET_ASSERT_GE((int)mock.size(), (int)3);
  ET_ASSERT_EQ((int)mock.at(0), (int)0x67);
  ET_ASSERT_EQ((int)mock.at(1), (int)0x57);
  ET_ASSERT_EQ((int)mock.at(2), (int)0x70);
}

static void test_pt6302_writeAt_uses_dcram_header_then_chars() {
  VFDPT6302HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  mock.clear();
  ET_ASSERT_TRUE(hal.writeAt(0, 5, "HI"));
  // DCRAM header: 0x10 | addr (5) => 0x15, then 'H','I'
  ET_ASSERT_EQ((int)mock.size(), (int)3);
  ET_ASSERT_EQ((int)mock.at(0), (int)0x15);
  ET_ASSERT_EQ((int)mock.at(1), (int)'H');
  ET_ASSERT_EQ((int)mock.at(2), (int)'I');
}

static void test_pt6302_set_dimming_levels() {
  VFDPT6302HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  for (uint8_t lvl=0; lvl<8; ++lvl) {
    mock.clear(); ET_ASSERT_TRUE(hal.setDimming(lvl));
    ET_ASSERT_EQ((int)mock.at(0), (int)(0x50 | (lvl & 0x07)));
  }
}

inline void register_VFDPT6302HAL_device_tests() {
  ET_ADD_TEST("PT6302.init_sequence", test_pt6302_init_sequence);
  ET_ADD_TEST("PT6302.writeAt_dcram_then_chars", test_pt6302_writeAt_uses_dcram_header_then_chars);
  ET_ADD_TEST("PT6302.dimming_levels", test_pt6302_set_dimming_levels);
}

