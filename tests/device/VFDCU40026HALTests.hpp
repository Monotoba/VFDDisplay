// Device-specific tests for VFDCU40026HAL
#pragma once

#include <Arduino.h>
#include "HAL/VFDCU40026HAL.h"
#include "tests/mocks/MockTransport.h"
#include "tests/framework/EmbeddedTest.h"

static void test_vfdcu40026_init_sends_ESC_I() {
  VFDCU40026HAL hal; MockTransport mock; hal.setTransport(&mock);
  ET_ASSERT_TRUE(hal.init());
  ET_ASSERT_EQ((int)mock.size(), (int)2);
  ET_ASSERT_EQ((int)mock.at(0), (int)0x1B);
  ET_ASSERT_EQ((int)mock.at(1), (int)0x49);
}

static void test_vfdcu40026_clear_home() {
  VFDCU40026HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  mock.clear(); ET_ASSERT_TRUE(hal.clear()); ET_ASSERT_EQ((int)mock.at(0),(int)0x0E);
  mock.clear(); ET_ASSERT_TRUE(hal.cursorHome()); ET_ASSERT_EQ((int)mock.at(0),(int)0x0C);
}

static void test_vfdcu40026_setCursorPos_ESC_H_addr() {
  VFDCU40026HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  mock.clear();
  ET_ASSERT_TRUE(hal.setCursorPos(1,3));
  ET_ASSERT_EQ((int)mock.size(), (int)3);
  ET_ASSERT_EQ((int)mock.at(0), (int)0x1B);
  ET_ASSERT_EQ((int)mock.at(1), (int)0x48);
  ET_ASSERT_EQ((int)mock.at(2), (int)(1*40+3));
}

static void test_vfdcu40026_dimming_ESC_L() {
  VFDCU40026HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  struct { uint8_t lvl; uint8_t code; } cases[] = {{0,0x00},{1,0x40},{2,0x80},{3,0xC0}};
  for (auto &c: cases) {
    mock.clear();
    ET_ASSERT_TRUE(hal.setDimming(c.lvl));
    ET_ASSERT_EQ((int)mock.at(0),(int)0x1B);
    ET_ASSERT_EQ((int)mock.at(1),(int)'L');
    ET_ASSERT_EQ((int)mock.at(2),(int)c.code);
  }
}

inline void register_VFDCU40026HAL_device_tests() {
  ET_ADD_TEST("VFDCU40026.init_ESC_I", test_vfdcu40026_init_sends_ESC_I);
  ET_ADD_TEST("VFDCU40026.clear_home", test_vfdcu40026_clear_home);
  ET_ADD_TEST("VFDCU40026.pos_ESC_H_addr", test_vfdcu40026_setCursorPos_ESC_H_addr);
  ET_ADD_TEST("VFDCU40026.dimming_ESC_L", test_vfdcu40026_dimming_ESC_L);
}

