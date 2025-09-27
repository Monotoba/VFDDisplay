// Device-specific tests for VFD20S401HAL
#pragma once

#include <Arduino.h>
#include "HAL/VFD20S401HAL.h"
#include "tests/mocks/MockTransport.h"
#include "tests/framework/EmbeddedTest.h"

static void test_vfd20s401_init_sends_0x49() {
  VFD20S401HAL hal;
  MockTransport mock; hal.setTransport(&mock);
  bool ok = hal.init();
  ET_ASSERT_TRUE(ok);
  ET_ASSERT_EQ((int)mock.size(), (int)1);
  ET_ASSERT_EQ((int)mock.at(0), (int)0x49);
}

static void test_vfd20s401_clear_sends_0x09() {
  VFD20S401HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  mock.clear();
  bool ok = hal.clear();
  ET_ASSERT_TRUE(ok);
  ET_ASSERT_EQ((int)mock.size(), (int)1);
  ET_ASSERT_EQ((int)mock.at(0), (int)0x09);
}

static void test_vfd20s401_home_sends_0x0C() {
  VFD20S401HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  mock.clear();
  bool ok = hal.cursorHome();
  ET_ASSERT_TRUE(ok);
  ET_ASSERT_EQ((int)mock.size(), (int)1);
  ET_ASSERT_EQ((int)mock.at(0), (int)0x0C);
}

static void test_vfd20s401_setCursorPos_sends_ESC_H_addr() {
  VFD20S401HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  mock.clear();
  bool ok = hal.setCursorPos(2, 3); // addr = 2*20 + 3 = 43 (0x2B)
  ET_ASSERT_TRUE(ok);
  ET_ASSERT_EQ((int)mock.size(), (int)3);
  ET_ASSERT_EQ((int)mock.at(0), (int)0x1B);
  ET_ASSERT_EQ((int)mock.at(1), (int)0x48);
  ET_ASSERT_EQ((int)mock.at(2), (int)0x2B);
}

static void test_vfd20s401_sendEscapeSequence_null_terminated() {
  VFD20S401HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  mock.clear();
  const uint8_t data[] = { 0x41, 0x42, 0x00 };
  bool ok = hal.sendEscapeSequence(data);
  ET_ASSERT_TRUE(ok);
  ET_ASSERT_EQ((int)mock.size(), (int)3);
  ET_ASSERT_EQ((int)mock.at(0), (int)0x1B);
  ET_ASSERT_EQ((int)mock.at(1), (int)0x41);
  ET_ASSERT_EQ((int)mock.at(2), (int)0x42);
}

static void test_vfd20s401_getCustomCharCode_mapping() {
  VFD20S401HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  uint8_t code = 0xFF;
  bool ok0 = hal.getCustomCharCode(0, code);
  ET_ASSERT_TRUE(ok0);
  ET_ASSERT_EQ((int)code, (int)0x00);
  bool ok8 = hal.getCustomCharCode(8, code);
  ET_ASSERT_TRUE(ok8);
  ET_ASSERT_EQ((int)code, (int)0x80);
}

static void test_vfd20s401_setCustomChar_all_zeros_packs_to_zeros() {
  VFD20S401HAL hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  const uint8_t pattern[8] = {0,0,0,0,0,0,0,0};
  mock.clear();
  bool ok = hal.setCustomChar(0, pattern);
  ET_ASSERT_TRUE(ok);
  // Expect: ESC, 'C', CHR, PT1..PT5 (zeros)
  ET_ASSERT_EQ((int)mock.size(), (int)8);
  ET_ASSERT_EQ((int)mock.at(0), (int)0x1B);
  ET_ASSERT_EQ((int)mock.at(1), (int)0x43);
  ET_ASSERT_EQ((int)mock.at(2), (int)0x00); // chr for index 0
  ET_ASSERT_EQ((int)mock.at(3), (int)0x00);
  ET_ASSERT_EQ((int)mock.at(4), (int)0x00);
  ET_ASSERT_EQ((int)mock.at(5), (int)0x00);
  ET_ASSERT_EQ((int)mock.at(6), (int)0x00);
  ET_ASSERT_EQ((int)mock.at(7), (int)0x00);
}

inline void register_VFD20S401HAL_device_tests() {
  ET_ADD_TEST("VFD20S401.init_sends_0x49", test_vfd20s401_init_sends_0x49);
  ET_ADD_TEST("VFD20S401.clear_sends_0x09", test_vfd20s401_clear_sends_0x09);
  ET_ADD_TEST("VFD20S401.home_sends_0x0C", test_vfd20s401_home_sends_0x0C);
  ET_ADD_TEST("VFD20S401.setCursorPos_ESC_H_addr", test_vfd20s401_setCursorPos_sends_ESC_H_addr);
  ET_ADD_TEST("VFD20S401.sendEscapeSequence_nullterm", test_vfd20s401_sendEscapeSequence_null_terminated);
  ET_ADD_TEST("VFD20S401.getCustomCharCode_mapping", test_vfd20s401_getCustomCharCode_mapping);
  ET_ADD_TEST("VFD20S401.setCustomChar_zeros_packed_zeros", test_vfd20s401_setCustomChar_all_zeros_packs_to_zeros);
}
