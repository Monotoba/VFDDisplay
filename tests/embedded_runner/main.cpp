// PlatformIO test runner entry that builds within this library repo
#include <Arduino.h>
#include "tests/framework/EmbeddedTest.h"
#include "tests/common/IVFDHALContractTests.hpp"
#include "tests/device/VFD20S401HALTests.hpp"
#include "tests/device/VFD20T202HALTests.hpp"
#include "tests/device/VFD20T204HALTests.hpp"
#include "tests/device/VFDCU20025HALTests.hpp"
#include "tests/device/VFDCU40026HALTests.hpp"
#include "tests/device/VFDHT16514HALTests.hpp"
#include "tests/device/VFDM202MD15HALTests.hpp"
#include "tests/device/VFDM202SD01HALTests.hpp"
#include "tests/device/VFDNA204SD01HALTests.hpp"
#include "tests/device/VFDM0216MDHALTests.hpp"
#include "tests/device/VFDVK20225HALTests.hpp"
#include "tests/device/VFDPT6302HALTests.hpp"
#include "tests/device/VFDPT6314HALTests.hpp"
#include "tests/device/VFDUPD16314HALTests.hpp"
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

  // VFD20T204
  register_IVFDHAL_contract_tests<VFD20T204HAL>("VFD20T204");
  register_VFD20T204HAL_device_tests();

  // VFDCU20025
  register_IVFDHAL_contract_tests<VFDCU20025HAL>("VFDCU20025");
  register_VFDCU20025HAL_device_tests();

  // VFDCU40026
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

  // NA204SD01
  register_IVFDHAL_contract_tests<VFDNA204SD01HAL>("NA204SD01");
  register_VFDNA204SD01HAL_device_tests();

  // M0216MD
  register_IVFDHAL_contract_tests<VFDM0216MDHAL>("M0216MD");
  register_VFDM0216MDHAL_device_tests();

  // VK202-25
  register_IVFDHAL_contract_tests<VFDVK20225HAL>("VK202-25");
  register_VFDVK20225HAL_device_tests();

  // PT6302
  register_IVFDHAL_contract_tests<VFDPT6302HAL>("PT6302");
  register_VFDPT6302HAL_device_tests();

  // PT6314
  register_IVFDHAL_contract_tests<VFDPT6314HAL>("PT6314");
  register_VFDPT6314HAL_device_tests();

  // uPD16314
  register_IVFDHAL_contract_tests<VFDUPD16314HAL>("uPD16314");
  register_VFDUPD16314HAL_device_tests();

  EmbeddedTest::runAll();
}

void loop() {
}
