// PlatformIO test runner entry that builds within this library repo
#include <Arduino.h>
#include "tests/framework/EmbeddedTest.h"
#include "tests/common/IVFDHALContractTests.hpp"
#include "tests/device/VFD20S401HALTests.hpp"
#include "tests/device/VFD20T202HALTests.hpp"
#include "tests/device/VFDCU20025HALTests.hpp"
#include "HAL/VFD20S401HAL.h"

void setup() {
  Serial.begin(115200);
  delay(2000);
  EmbeddedTest::setOutput(&Serial);
  EmbeddedTest::begin();

  register_IVFDHAL_contract_tests<VFD20S401HAL>("VFD20S401");
  register_VFD20S401HAL_device_tests();

  // VFD20T202
  register_IVFDHAL_contract_tests<VFD20T202HAL>("VFD20T202");
  register_VFD20T202HAL_device_tests();

  // VFDCU20025
  register_IVFDHAL_contract_tests<VFDCU20025HAL>("VFDCU20025");
  register_VFDCU20025HAL_device_tests();

  EmbeddedTest::runAll();
}

void loop() {
}
