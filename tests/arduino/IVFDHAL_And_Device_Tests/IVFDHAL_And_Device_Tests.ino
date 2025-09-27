// Arduino test runner for IVFDHAL contract + VFD20S401HAL device tests
#include <Arduino.h>
#include "tests/framework/EmbeddedTest.h"
#include "tests/common/IVFDHALContractTests.hpp"
#include "tests/device/VFD20S401HALTests.hpp"
#include "tests/device/VFD20T202HALTests.hpp"
#include "tests/device/VFDCU20025HALTests.hpp"
#include "tests/device/VFDCU40026HALTests.hpp"
#include "tests/device/VFDHT16514HALTests.hpp"
#include "tests/device/VFDM202MD15HALTests.hpp"
#include "tests/device/VFDM202SD01HALTests.hpp"
#include "tests/device/VFDM0216MDHALTests.hpp"
#include "VFDDisplay.h"           // ensure Arduino builder pulls in library sources
#include "HAL/VFD20S401HAL.h"

void setup() {
  Serial.begin(115200);
  delay(2000);
  EmbeddedTest::setOutput(&Serial);
  EmbeddedTest::begin();

  // Register reusable interface contract tests for this HAL
  register_IVFDHAL_contract_tests<VFD20S401HAL>("VFD20S401");

  // Register device-specific tests
  register_VFD20S401HAL_device_tests();

  // VFD20T202 HAL interface and device tests
  register_IVFDHAL_contract_tests<VFD20T202HAL>("VFD20T202");
  register_VFD20T202HAL_device_tests();

  // VFDCU20025 HAL interface and device tests
  register_IVFDHAL_contract_tests<VFDCU20025HAL>("VFDCU20025");
  register_VFDCU20025HAL_device_tests();

  // VFDCU40026 HAL interface and device tests
  register_IVFDHAL_contract_tests<VFDCU40026HAL>("VFDCU40026");
  register_VFDCU40026HAL_device_tests();

  // HT16514
  register_IVFDHAL_contract_tests<VFDHT16514HAL>("HT16514");
  register_VFDHT16514HAL_device_tests();

  // M202MD15
  register_IVFDHAL_contract_tests<VFDM202MD15HAL>("M202MD15");
  register_VFDM202MD15HAL_device_tests();

  // M202SD01
  register_IVFDHAL_contract_tests<VFDM202SD01HAL>("M202SD01");
  register_VFDM202SD01HAL_device_tests();

  // M0216MD
  register_IVFDHAL_contract_tests<VFDM0216MDHAL>("M0216MD");
  register_VFDM0216MDHAL_device_tests();

  // Run tests once
  EmbeddedTest::runAll();
}

void loop() {
  // no-op
}
