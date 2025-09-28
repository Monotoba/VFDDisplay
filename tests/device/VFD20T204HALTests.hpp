// Device-specific tests for VFD20T204HAL (20x4 HD44780-like)
#pragma once

#include <Arduino.h>
#include "HAL/VFD20T204HAL.h"
#include "tests/mocks/MockTransport.h"
#include "tests/framework/EmbeddedTest.h"

static void test_20t204_init_sequence() {
  VFD20T204HAL hal; MockTransport mock; hal.setTransport(&mock);
  ET_ASSERT_TRUE(hal.init());
  ET_ASSERT_TRUE(mock.size() >= 4);
  ET_ASSERT_EQ((int)mock.at(0), (int)0x38);
  ET_ASSERT_EQ((int)mock.at(1), (int)0x0C);
  ET_ASSERT_EQ((int)mock.at(2), (int)0x01);
  ET_ASSERT_EQ((int)mock.at(3), (int)0x06);
}

static void test_20t204_pos_ddram_bases() {
  VFD20T204HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  struct { uint8_t row, col, expect; } cases[] = {
    {0,0,0x80},{1,0,0xC0},{2,0,0x94},{3,0,0xD4},
    {2,5,(uint8_t)(0x80 | (0x14+5))}
  };
  for (auto &c : cases) {
    mock.clear(); ET_ASSERT_TRUE(hal.setCursorPos(c.row, c.col));
    ET_ASSERT_EQ((int)mock.at(0), (int)c.expect);
  }
}

inline void register_VFD20T204HAL_device_tests() {
  ET_ADD_TEST("20T204.init_sequence", test_20t204_init_sequence);
  ET_ADD_TEST("20T204.pos_ddram_bases", test_20t204_pos_ddram_bases);
}

