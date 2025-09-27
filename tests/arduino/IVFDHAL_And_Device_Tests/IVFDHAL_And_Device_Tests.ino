// Arduino test runner for IVFDHAL contract + VFD20S401HAL device tests
#include <Arduino.h>
#include "tests/framework/EmbeddedTest.h"
#include "tests/common/IVFDHALContractTests.hpp"
#include "tests/device/VFD20S401HALTests.hpp"
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

  // Run tests once
  EmbeddedTest::runAll();
}

void loop() {
  // no-op
}
