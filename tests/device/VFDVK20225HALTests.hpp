// Device-specific tests for VFDVK20225HAL
#pragma once

#include <Arduino.h>
#include "HAL/VFDVK20225HAL.h"
#include "tests/mocks/MockTransport.h"
#include "tests/framework/EmbeddedTest.h"

static void test_vk20225_clear_and_pos() {
  VFDVK20225HAL hal; MockTransport mock; hal.setTransport(&mock);
  // clear
  mock.clear(); ET_ASSERT_TRUE(hal.clear()); ET_ASSERT_EQ((int)mock.size(), (int)2); ET_ASSERT_EQ((int)mock.at(0),(int)254); ET_ASSERT_EQ((int)mock.at(1),(int)88);
  // set cursor pos row=1,col=3 (1-based sent)
  mock.clear(); ET_ASSERT_TRUE(hal.setCursorPos(1,3)); ET_ASSERT_EQ((int)mock.size(), (int)4); ET_ASSERT_EQ((int)mock.at(0),(int)254); ET_ASSERT_EQ((int)mock.at(1),(int)71); ET_ASSERT_EQ((int)mock.at(2),(int)4); ET_ASSERT_EQ((int)mock.at(3),(int)2);
}

static void test_vk20225_brightness() {
  VFDVK20225HAL hal; MockTransport mock; hal.setTransport(&mock);
  mock.clear(); ET_ASSERT_TRUE(hal.setBrightness(128)); ET_ASSERT_EQ((int)mock.size(), (int)3); ET_ASSERT_EQ((int)mock.at(0),(int)254); ET_ASSERT_EQ((int)mock.at(1),(int)89); ET_ASSERT_EQ((int)mock.at(2),(int)128);
}

static void test_vk20225_wrap_and_save_brightness() {
  VFDVK20225HAL hal; MockTransport mock; hal.setTransport(&mock);
  mock.clear(); ET_ASSERT_TRUE(hal.autoLineWrapOn());  ET_ASSERT_EQ((int)mock.size(), (int)2); ET_ASSERT_EQ((int)mock.at(0),(int)254); ET_ASSERT_EQ((int)mock.at(1),(int)67);
  mock.clear(); ET_ASSERT_TRUE(hal.autoLineWrapOff()); ET_ASSERT_EQ((int)mock.size(), (int)2); ET_ASSERT_EQ((int)mock.at(0),(int)254); ET_ASSERT_EQ((int)mock.at(1),(int)68);
  mock.clear(); ET_ASSERT_TRUE(hal.saveBrightness(200)); ET_ASSERT_EQ((int)mock.size(), (int)3); ET_ASSERT_EQ((int)mock.at(0),(int)254); ET_ASSERT_EQ((int)mock.at(1),(int)145); ET_ASSERT_EQ((int)mock.at(2),(int)200);
}

inline void register_VFDVK20225HAL_device_tests() {
  ET_ADD_TEST("VK20225.clear_and_pos", test_vk20225_clear_and_pos);
  ET_ADD_TEST("VK20225.brightness", test_vk20225_brightness);
  ET_ADD_TEST("VK20225.wrap_and_save_brightness", test_vk20225_wrap_and_save_brightness);
}
