// Minimal embedded-friendly test framework for Arduino/PlatformIO/AVR
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef ARDUINO
#include <Arduino.h>
#endif

namespace EmbeddedTest {

  typedef void (*TestFunc)();

  struct TestCase {
    const char* name;
    TestFunc func;
  };

  // Registry (simple static array to avoid dynamic allocation)
  static const uint16_t MAX_TESTS = 256;
  static TestCase g_tests[MAX_TESTS];
  static uint16_t g_count = 0;

  // Output
#ifdef ARDUINO
  static Stream* g_out = nullptr;
#else
  // Fallback to stdio
  #include <stdio.h>
#endif

  inline void setOutput(
#ifdef ARDUINO
      Stream* out
#else
      void* /*unused*/
#endif
  ) {
#ifdef ARDUINO
    g_out = out;
#endif
  }

  inline void print(const char* s) {
#ifdef ARDUINO
    if (g_out) g_out->print(s);
#else
    fputs(s, stdout);
#endif
  }

  inline void println(const char* s) {
#ifdef ARDUINO
    if (g_out) { g_out->print(s); g_out->print("\r\n"); }
#else
    fputs(s, stdout); fputs("\n", stdout);
#endif
  }

  inline void printHex(uint8_t b) {
#ifdef ARDUINO
    if (g_out) g_out->print(b, HEX);
#else
    char buf[5];
    snprintf(buf, sizeof(buf), "%02X", (unsigned)b);
    fputs(buf, stdout);
#endif
  }

  // Counters
  static uint32_t g_total = 0;
  static uint32_t g_failed = 0;
  static uint32_t g_asserts = 0;
  static uint32_t g_failedAsserts = 0;
  static bool g_currentFailed = false;

  inline void resetSession() {
    g_total = g_failed = g_asserts = g_failedAsserts = 0;
    g_currentFailed = false;
  }

  inline void addTest(const char* name, TestFunc func) {
    if (g_count < MAX_TESTS) {
      g_tests[g_count++] = { name, func };
    }
  }

  inline void begin() {
    resetSession();
  }

  inline void end() {
    // no-op currently
  }

  // Assertion helpers
  inline void fail(const char* msg) {
    g_failedAsserts++;
    g_currentFailed = true;
    print("  FAIL: "); println(msg);
  }

  inline void pass(const char* /*msg*/) {
    // no per-assert output by default
  }

  // Basic equality check for integers
  template <typename A, typename B>
  inline void assertEqual(const A& a, const B& b, const char* msg) {
    g_asserts++;
    if (!(a == b)) fail(msg); else pass(msg);
  }

  inline void assertTrue(bool cond, const char* msg) {
    g_asserts++;
    if (!cond) fail(msg); else pass(msg);
  }

  inline void runAll() {
    println("[TEST] Starting tests");
    for (uint16_t i = 0; i < g_count; ++i) {
      g_currentFailed = false;
      g_total++;
      print("[RUN ] "); println(g_tests[i].name);
      g_tests[i].func();
      if (g_currentFailed) { g_failed++; print("[FAIL] "); }
      else { print("[ OK ] "); }
      println(g_tests[i].name);
    }
    // Summary
    print("[DONE] tests=");
#ifdef ARDUINO
    if (g_out) g_out->print(g_total);
    if (g_out) g_out->print(" failed=");
    if (g_out) g_out->print(g_failed);
    if (g_out) g_out->print(" asserts=");
    if (g_out) g_out->print(g_asserts);
    if (g_out) g_out->print(" assert_fails=");
    if (g_out) g_out->println(g_failedAsserts);
#else
    {
      char buf[128];
      snprintf(buf, sizeof(buf), "%lu failed=%lu asserts=%lu assert_fails=%lu\n",
        (unsigned long)g_total, (unsigned long)g_failed,
        (unsigned long)g_asserts, (unsigned long)g_failedAsserts);
      fputs(buf, stdout);
    }
#endif
  }

} // namespace EmbeddedTest

// Convenience macros
// Use variadic macro so lambda bodies with commas/braces are accepted as a single argument
#define ET_ADD_TEST(name, ...) ::EmbeddedTest::addTest(name, __VA_ARGS__)
#define ET_ASSERT_TRUE(cond) ::EmbeddedTest::assertTrue((cond), #cond)
#define ET_ASSERT_EQ(a,b) ::EmbeddedTest::assertEqual((a), (b), #a " == " #b)
// Additional comparisons used by tests
#define ET_ASSERT_GE(a,b) ::EmbeddedTest::assertTrue(((a) >= (b)), #a " >= " #b)
