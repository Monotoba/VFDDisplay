// File: test_futaba_blinkspeed_cmd.ino
// Purpose: probe Futaba 20S401DA1 blink speed control (0x1B 0x54 <speed>)

#include <Arduino.h>

#define DISPLAY_SERIAL Serial1  // adjust for your MCU

const uint8_t CMD_CURSOR_ON = 0x15; // Verified: enables cursor blink
const uint8_t CMD_CURSOR_OFF = 0x16;
const uint8_t ESC = 0x1B;
const uint8_t CLEAR = 0x0E;
const uint8_t RESET = 0x49;
const uint8_t HOME = 0x0C;
const uint8_t CMD_BLINK_SPEED = 0x53;

const uint8_t blinkSpeeds[] = {0x00, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xc0, 0xE0, 0xF0};

void resetDisplay(HardwareSerial &s) {
  // ESC @ is commonly documented as reset
  s.write(ESC);
  s.write(RESET);
  delay(500);
}

void clearDisplay(HardwareSerial &s) {
  // HD44780-style clear screen
  s.write(CLEAR);
  delay(200);
  s.write(HOME);
  delay(300);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Futaba blink speed probe (ESC 0x54)...");

  DISPLAY_SERIAL.begin(19200);  // try default first
  delay(200);

  resetDisplay(DISPLAY_SERIAL);
  clearDisplay(DISPLAY_SERIAL);
  delay(1000);

}

void blinkRate(uint8_t cmd) {
    
    // RUn through speed 
    for (uint8_t i = 0; i < sizeof(blinkSpeeds); i++) {
    uint8_t speed = blinkSpeeds[i];
    str = sprintf("Setting blink cmd = 0x%d", cmd);
    Serial.print("Setting blink speed = 0x");
    Serial.println(speed, HEX);

    DISPLAY_SERIAL.write(ESC);
    DISPLAY_SERIAL.write(cmd);
    DISPLAY_SERIAL.write(speed);

    // let it run for a while so you can see if blink changes
    delay(3000);
  }
    
}

void loop() {
  resetDisplay(DISPLAY_SERIAL);
  clearDisplay(DISPLAY_SERIAL);
  delay(1000);

    
  // Enable cursor blink first
  DISPLAY_SERIAL.write(CMD_CURSOR_ON);
  delay(3000);

  for (uint8_t i = 0; i <=255; i++) {

    Serial.print("Setting blink CMD = 0x");
    Serial.println(i, HEX);

    blinkRate(i);
    // let it run for a while so you can see if blink changes
    delay(3000);
  }
  
  DISPLAY_SERIAL.write(CMD_CURSOR_OFF);
  Serial.println("Done testing blink speeds.");
  delay(3000);
}

