// Reusable contract tests for any IVFDHAL implementation
#pragma once

#include <Arduino.h>
#include "HAL/IVFDHAL.h"
#include "Capabilities/IDisplayCapabilities.h"
#include "tests/mocks/MockTransport.h"
#include "tests/framework/EmbeddedTest.h"

// Individual templated test functions

template <typename HAL>
void test_hal_init_without_transport_fails() {
  HAL hal;
  bool ok = hal.init();
  ET_ASSERT_TRUE(ok == false);
  ET_ASSERT_EQ((int)hal.lastError(), (int)VFDError::TransportFail);
}

template <typename HAL>
void test_hal_capabilities_present() {
  HAL hal;
  const IDisplayCapabilities* caps = hal.getDisplayCapabilities();
  ET_ASSERT_TRUE(caps != nullptr);
  if (caps) {
    ET_ASSERT_TRUE(caps->getTextRows() > 0);
    ET_ASSERT_TRUE(caps->getTextColumns() > 0);
  }
}

template <typename HAL>
void test_hal_writeAt_positions_then_writes_text() {
  HAL hal;
  MockTransport mock;
  hal.setTransport(&mock);
  ET_ASSERT_TRUE(hal.init());
  mock.clear();
  const char* text = "HI";
  bool ok = hal.writeAt(1, 5, text);
  ET_ASSERT_TRUE(ok);
  // Expect ESC, 'H', addr, then 'H','I'
  uint8_t addr = (uint8_t)(1 * 20 + 5);
  uint8_t expected[5] = { 0x1B, 0x48, addr, 'H', 'I' };
  ET_ASSERT_EQ((int)mock.size(), (int)5);
  // Compare each byte to aid debugging
  for (int i=0;i<5;++i) {
    ET_ASSERT_EQ((int)mock.at(i), (int)expected[i]);
  }
}

template <typename HAL>
void test_hal_setCursorPos_invalid_args_sets_error() {
  HAL hal;
  MockTransport mock;
  hal.setTransport(&mock);
  (void)hal.init();
  bool ok = hal.setCursorPos(99, 0);
  ET_ASSERT_TRUE(ok == false);
  ET_ASSERT_EQ((int)hal.lastError(), (int)VFDError::InvalidArgs);
}

template <typename HAL>
void test_hal_write_nullptr_is_invalid_args() {
  HAL hal;
  MockTransport mock;
  hal.setTransport(&mock);
  (void)hal.init();
  bool ok = hal.write(nullptr);
  ET_ASSERT_TRUE(ok == false);
  ET_ASSERT_EQ((int)hal.lastError(), (int)VFDError::InvalidArgs);
}

// Registration helper to add all contract tests for a specific HAL
template <typename HAL>
void register_IVFDHAL_contract_tests(const char* halName) {
  using namespace EmbeddedTest;
  // Since ET_ADD_TEST expects function pointer, pass explicit template instantiations
  ET_ADD_TEST("IVFDHAL.init_without_transport_fails", test_hal_init_without_transport_fails<HAL>);
  ET_ADD_TEST("IVFDHAL.capabilities_present", test_hal_capabilities_present<HAL>);
  ET_ADD_TEST("IVFDHAL.writeAt_positions_then_writes_text", test_hal_writeAt_positions_then_writes_text<HAL>);
  ET_ADD_TEST("IVFDHAL.setCursorPos_invalid_args_sets_error", test_hal_setCursorPos_invalid_args_sets_error<HAL>);
  ET_ADD_TEST("IVFDHAL.write_nullptr_is_invalid_args", test_hal_write_nullptr_is_invalid_args<HAL>);
  (void)halName; // reserved for future per-HAL labeling
}
