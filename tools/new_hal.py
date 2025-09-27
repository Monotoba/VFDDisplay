#!/usr/bin/env python3
"""
HAL Scaffold Tool

Generates a new device HAL skeleton implementing IVFDHAL, a device test stub,
and a per‑HAL documentation page based on the template.

Usage:
  python3 tools/new_hal.py --name 20X2ABC --class VFD20X2ABCHAL --rows 2 --cols 20 \
    --datasheet docs/datasheets/20X2ABC.pdf --family hd44780 --transport sync3

Arguments:
  --name        Human-readable device/model name (e.g., 20T202)
  --class       C++ class name (e.g., VFD20T202HAL)
  --rows        Text rows (e.g., 2)
  --cols        Text columns (e.g., 20)
  --datasheet   Path to the PDF in docs/datasheets (optional)
  --family      Command family: hd44780 | esc (default: hd44780)
  --transport   Recommended transport: serial | sync3 | parallel (default: serial)

What it creates (non-destructive; fails if files already exist):
  - src/HAL/<Class>.h and .cpp (skeleton implementing IVFDHAL)
  - tests/device/<Class>Tests.hpp (basic tests)
  - docs/api/<Class>.md from docs/api/HAL_Doc_Template.md

Notes:
  - You still need to add a capabilities factory to CapabilitiesRegistry and register it
    in your HAL constructor (see HAL Authoring Guide).
  - Add your HAL to test runners (embedded + Arduino).
"""

import argparse
import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

H_TEMPLATE = """#pragma once
#include "IVFDHAL.h"
#include "Transports/ITransport.h"
#include "../Capabilities/IDisplayCapabilities.h"
#include "../Capabilities/DisplayCapabilities.h"
#include <Arduino.h>

class {CLASS} : public IVFDHAL {{
public:
    {CLASS}();
    ~{CLASS}() override = default;

    void setTransport(ITransport* transport) override {{ _transport = transport; }}

    bool init() override;
    bool reset() override;
    bool clear() override;
    bool cursorHome() override;
    bool setCursorPos(uint8_t row, uint8_t col) override;
    bool setCursorBlinkRate(uint8_t rate_ms) override;

    bool writeCharAt(uint8_t row, uint8_t column, char c) override;
    bool writeAt(uint8_t row, uint8_t column, const char* text) override;
    bool moveTo(uint8_t row, uint8_t column) override;

    bool backSpace() override;
    bool hTab() override;
    bool lineFeed() override;
    bool carriageReturn() override;

    bool writeChar(char c) override;
    bool write(const char* msg) override;
    bool centerText(const char* str, uint8_t row) override;
    bool writeCustomChar(uint8_t index) override;
    bool getCustomCharCode(uint8_t index, uint8_t& codeOut) const override;

    bool setBrightness(uint8_t lumens) override;
    bool saveCustomChar(uint8_t index, const uint8_t* pattern) override;
    bool setCustomChar(uint8_t index, const uint8_t* pattern) override;
    bool setDisplayMode(uint8_t mode) override;
    bool setDimming(uint8_t level) override;
    bool cursorBlinkSpeed(uint8_t rate) override;
    bool changeCharSet(uint8_t setId) override;

    bool sendEscapeSequence(const uint8_t* data) override;

    bool hScroll(const char* str, int dir, uint8_t row) override;
    bool vScroll(const char* str, int dir) override;
    bool vScrollText(const char* text, uint8_t startRow, ScrollDirection direction) override;
    bool starWarsScroll(const char* text, uint8_t startRow) override;

    bool flashText(const char* str, uint8_t row, uint8_t col,
                   uint8_t on_ms, uint8_t off_ms) override;

    int getCapabilities() const override;
    const char* getDeviceName() const override;
    const IDisplayCapabilities* getDisplayCapabilities() const override {{ return _capabilities; }}

    void delayMicroseconds(unsigned int us) const override {{ ::delayMicroseconds(us); }}

    VFDError lastError() const override {{ return _lastError; }}
    void clearError() override {{ _lastError = VFDError::Ok; }}

    // ===== NO_TOUCH: device primitives =====
    bool _cmdInit();
    bool _cmdClear();
    bool _cmdHome();
    bool _posRowCol(uint8_t row, uint8_t col);
    bool _posLinear(uint8_t addr);

private:
    ITransport* _transport = nullptr;
    DisplayCapabilities* _capabilities = nullptr;
    VFDError _lastError = VFDError::Ok;
}};
"""

CPP_TEMPLATE_HD44780 = """#include "{CLASS}.h"
#include "../Capabilities/CapabilitiesRegistry.h"
#include <string.h>

{CLASS}::{CLASS}() {{
    _capabilities = nullptr; // TODO: create and register capabilities
}}

bool {CLASS}::init() {{
    if (!_transport) {{ _lastError = VFDError::TransportFail; return false; }}
    return _cmdInit();
}}

bool {CLASS}::reset() {{ return _cmdInit(); }}

bool {CLASS}::clear() {{ return _cmdClear(); }}
bool {CLASS}::cursorHome() {{ return _cmdHome(); }}

bool {CLASS}::setCursorPos(uint8_t row, uint8_t col) {{ return _posRowCol(row, col); }}
bool {CLASS}::setCursorBlinkRate(uint8_t) {{ _lastError = VFDError::NotSupported; return false; }}

bool {CLASS}::writeCharAt(uint8_t row, uint8_t column, char c) {{ return _posRowCol(row,column) && writeChar(c); }}
bool {CLASS}::writeAt(uint8_t row, uint8_t column, const char* text) {{ return _posRowCol(row,column) && write(text); }}
bool {CLASS}::moveTo(uint8_t row, uint8_t column) {{ return _posRowCol(row,column); }}

bool {CLASS}::backSpace() {{ return writeChar(0x08); }}
bool {CLASS}::hTab()      {{ return writeChar(0x09); }}
bool {CLASS}::lineFeed()  {{ return writeChar(0x0A); }}
bool {CLASS}::carriageReturn() {{ return writeChar(0x0D); }}

bool {CLASS}::writeChar(char c) {{ return _transport && _transport->write(reinterpret_cast<const uint8_t*>(&c), 1); }}
bool {CLASS}::write(const char* msg) {{ if(!_transport||!msg) return false; return _transport->write(reinterpret_cast<const uint8_t*>(msg), strlen(msg)); }}

bool {CLASS}::centerText(const char* str, uint8_t row) {{
    if (!str) return false; size_t len=strlen(str); (void)len; // TODO: use capabilities rows/cols
    if (!setCursorPos(row,0)) return false; return write(str);
}}

bool {CLASS}::writeCustomChar(uint8_t index) {{ uint8_t code; return getCustomCharCode(index, code) && writeChar((char)code); }}
bool {CLASS}::getCustomCharCode(uint8_t index, uint8_t& codeOut) const {{ if (index<8) {{ codeOut=index; return true; }} return false; }}

bool {CLASS}::setBrightness(uint8_t) {{ _lastError = VFDError::NotSupported; return false; }}
bool {CLASS}::saveCustomChar(uint8_t i, const uint8_t* p) {{ return setCustomChar(i,p); }}
bool {CLASS}::setCustomChar(uint8_t index, const uint8_t* pattern) {{ (void)index; (void)pattern; _lastError = VFDError::NotSupported; return false; }}
bool {CLASS}::setDisplayMode(uint8_t) {{ _lastError = VFDError::NotSupported; return false; }}
bool {CLASS}::setDimming(uint8_t) {{ _lastError = VFDError::NotSupported; return false; }}
bool {CLASS}::cursorBlinkSpeed(uint8_t) {{ _lastError = VFDError::NotSupported; return false; }}
bool {CLASS}::changeCharSet(uint8_t) {{ _lastError = VFDError::NotSupported; return false; }}
bool {CLASS}::sendEscapeSequence(const uint8_t*) {{ _lastError = VFDError::NotSupported; return false; }}

bool {CLASS}::hScroll(const char*, int, uint8_t) {{ _lastError = VFDError::NotSupported; return false; }}
bool {CLASS}::vScroll(const char*, int) {{ _lastError = VFDError::NotSupported; return false; }}
bool {CLASS}::vScrollText(const char*, uint8_t, ScrollDirection) {{ _lastError = VFDError::NotSupported; return false; }}
bool {CLASS}::starWarsScroll(const char*, uint8_t) {{ _lastError = VFDError::NotSupported; return false; }}

bool {CLASS}::flashText(const char*, uint8_t, uint8_t, uint8_t, uint8_t) {{ _lastError = VFDError::NotSupported; return false; }}

int {CLASS}::getCapabilities() const {{ return _capabilities ? _capabilities->getAllCapabilities() : 0; }}
const char* {CLASS}::getDeviceName() const {{ return "{NAME}"; }}

// ===== NO_TOUCH primitives (HD44780-style placeholders) =====
bool {CLASS}::_cmdInit() {{
    uint8_t seq[]={{0x38,0x0C,0x01,0x06}}; // Function Set, Display On, Clear, Entry Mode
    for (uint8_t b:seq) {{ if(!_transport->write(&b,1)) return false; }}
    return true;
}}
bool {CLASS}::_cmdClear() {{ uint8_t b=0x01; return _transport->write(&b,1); }}
bool {CLASS}::_cmdHome() {{ uint8_t b=0x02; return _transport->write(&b,1); }}
bool {CLASS}::_posLinear(uint8_t addr) {{ uint8_t b=(uint8_t)(0x80 | (addr & 0x7F)); return _transport->write(&b,1); }}
bool {CLASS}::_posRowCol(uint8_t row, uint8_t col) {{ uint8_t base[]={{0x00,0x40}}; if (row>=2) return false; return _posLinear((uint8_t)(base[row]+col)); }}
"""

TEST_TEMPLATE = """// Device tests for {CLASS}
#pragma once
#include <Arduino.h>
#include "HAL/{CLASS}.h"
#include "tests/mocks/MockTransport.h"
#include "tests/framework/EmbeddedTest.h"

static void test_{LOW}_init_sequence() {{
  {CLASS} hal; MockTransport mock; hal.setTransport(&mock);
  ET_ASSERT_TRUE(hal.init());
  ET_ASSERT_TRUE(mock.size() >= 4);
}}

static void test_{LOW}_clear_and_home() {{
  {CLASS} hal; MockTransport mock; hal.setTransport(&mock); (void)hal.init();
  mock.clear(); ET_ASSERT_TRUE(hal.clear());
  mock.clear(); ET_ASSERT_TRUE(hal.cursorHome());
}}

inline void register_{CLASS}_device_tests() {{
  ET_ADD_TEST("{CLASS}.init", test_{LOW}_init_sequence);
  ET_ADD_TEST("{CLASS}.clear_home", test_{LOW}_clear_and_home);
}}
"""

DOC_TEMPLATE_PATH = ROOT / "docs" / "api" / "HAL_Doc_Template.md"

def write_file(path: Path, content: str):
    if path.exists():
        print(f"[SKIP] {path} already exists")
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content)
    print(f"[ADD ] {path}")
    return True

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--name", required=True)
    ap.add_argument("--class", dest="klass", required=True)
    ap.add_argument("--rows", type=int, default=2)
    ap.add_argument("--cols", type=int, default=20)
    ap.add_argument("--datasheet", default="")
    ap.add_argument("--family", choices=["hd44780","esc"], default="hd44780")
    ap.add_argument("--transport", choices=["serial","sync3","parallel"], default="serial")
    args = ap.parse_args()

    klass = args.klass
    name = args.name
    low = klass.lower()

    # HAL code
    h_path = ROOT / "src" / "HAL" / f"{klass}.h"
    cpp_path = ROOT / "src" / "HAL" / f"{klass}.cpp"
    h_content = H_TEMPLATE.replace("{CLASS}", klass)
    cpp_content = CPP_TEMPLATE_HD44780.replace("{CLASS}", klass).replace("{NAME}", name)
    ok1 = write_file(h_path, h_content)
    ok2 = write_file(cpp_path, cpp_content)

    # Tests
    test_path = ROOT / "tests" / "device" / f"{klass}Tests.hpp"
    test_content = TEST_TEMPLATE.replace("{CLASS}", klass).replace("{LOW}", low)
    ok3 = write_file(test_path, test_content)

    # Docs
    doc_path = ROOT / "docs" / "api" / f"{klass}.md"
    if DOC_TEMPLATE_PATH.exists():
        tpl = DOC_TEMPLATE_PATH.read_text()
    else:
        tpl = "# <HAL Name>\n\nSee HAL_Doc_Template.md"
    tpl = tpl.replace("<HAL Name>", klass)
    if args.datasheet:
        ds_line = f"- Datasheet: {args.datasheet}\n"
        tpl = tpl.replace("Datasheet Links\n- ", f"Datasheet Links\n{ds_line}- ")
    ok4 = write_file(doc_path, tpl)

    print("\nNext steps:")
    print("- Add a capabilities factory in CapabilitiesRegistry (create<HAL>Capabilities).")
    print("- In your HAL constructor, set _capabilities = create<HAL>Capabilities() and register.")
    print("- Add your HAL tests to test runners (embedded_runner and Arduino runner).")
    print("- Update CHANGELOG.md.")

    return 0 if (ok1 or ok2 or ok3 or ok4) else 1

if __name__ == "__main__":
    sys.exit(main())

