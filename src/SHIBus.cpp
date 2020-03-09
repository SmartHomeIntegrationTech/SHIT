/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "SHIBus.h"

int SHI::Print::getWriteError() { return writeError; }
void SHI::Print::clearWriteError() { writeError = 0; }

size_t SHI::Print::write(const char* str) {
  if (str != nullptr) {
    int len = strlen(str);
    return write(str, len);
  }
  return 0;
}
size_t SHI::Print::write(const char* buffer, size_t size) {
  return write(reinterpret_cast<uint8_t*>(const_cast<char*>(buffer)), size);
}

size_t SHI::Print::write(char c) { return write(static_cast<uint8_t>(c)); }
size_t SHI::Print::write(int8_t c) { return write(static_cast<uint8_t>(c)); }
size_t SHI::Print::write(int16_t t) { return write(static_cast<uint8_t>(t)); }
size_t SHI::Print::write(uint16_t t) { return write(static_cast<uint8_t>(t)); }
size_t SHI::Print::write(int32_t t) { return write(static_cast<uint8_t>(t)); }
size_t SHI::Print::write(uint32_t t) { return write(static_cast<uint8_t>(t)); }
size_t SHI::Print::write(int64_t t) { return write(static_cast<uint8_t>(t)); }
size_t SHI::Print::write(uint64_t t) { return write(static_cast<uint8_t>(t)); }

size_t SHI::Print::printf(const char* format, ...) {
  va_list args;
  va_start(args, format);
  char buf[32];
  size_t written = vsnprintf(buf, sizeof(buf), format, args) + 1;
  va_end(args);
  // The written includes the terminating null (because of +1)
  if (written <= sizeof(buf)) {
    write(buf, written);
  } else {
    std::unique_ptr<char[]> newBuf(new char[written]);
    if (newBuf) {
      va_list newArgs;
      va_start(newArgs, format);
      vsnprintf(newBuf.get(), written, format, newArgs);
      va_end(newArgs);
      write(newBuf.get(), written);
    } else {
      return -1;
    }
  }
  //Becaue we added one in the beginning
  return written-1;
}
size_t SHI::Print::printSigned(int64_t value, int base) {
  if (value < 0) {
    return print('-') + printUnsigned(-value, base);
  }
  return printUnsigned(value, base);
}

std::string toString(int value) {
  char result[20 + 1];
  result[20] = 0;
  size_t pos = 20;
  do {
    int current = value;
    value /= 10;
    char number = current - 10 * value;
    if (number < 10)
      result[--pos] = number + '0';
    else
      result[--pos] = (number - 10) + 'A';
  } while (value != 0);
  return std::string(&result[pos], 20 - pos);
}
size_t SHI::Print::printUnsigned(uint64_t value, int base) {
  if (base <= 1) base = 10;
  uint8_t result[20 + 1];
  result[20] = 0;
  size_t pos = 20;
  do {
    uint64_t current = value;
    value /= base;
    char number = current - base * value;
    if (number < 10)
      result[--pos] = number + '0';
    else
      result[--pos] = (number - 10) + 'A';
  } while (value != 0);
  return write(&result[pos], static_cast<size_t>(20 - pos));
}

size_t SHI::Print::print(const char value[]) { return write(value); }
size_t SHI::Print::print(char value) { return write(value); }
size_t SHI::Print::print(uint8_t value, int base) {
  return printUnsigned(value, base);
}
size_t SHI::Print::print(int16_t value, int base) {
  return printSigned(value, base);
}
size_t SHI::Print::print(uint16_t value, int base) {
  return printUnsigned(value, base);
}
size_t SHI::Print::print(int32_t value, int base) {
  return printSigned(value, base);
}
size_t SHI::Print::print(uint32_t value, int base) {
  return printUnsigned(value, base);
}
size_t SHI::Print::print(int64_t value, int base) {
  return printSigned(value, base);
}
size_t SHI::Print::print(uint64_t value, int base) {
  return printUnsigned(value, base);
}
size_t SHI::Print::print(double value, int precision) {
  if (precision == 2)  // This is the default, so be smarter here
    return printf("%0.2f", value);
  std::string format = std::string("%0.") + toString(precision) + "f";
  return printf(format.c_str(), value);
}
size_t SHI::Print::print(struct tm* timeinfo, const char* format) { return 0; }

size_t SHI::Print::println(const char value[]) {
  return print(value) + println();
}
size_t SHI::Print::println(char value) { return print(value) + println(); }
size_t SHI::Print::println(uint8_t value, int base) {
  return print(value, base) + println();
}
size_t SHI::Print::println(int16_t value, int base) {
  return print(value, base) + println();
}
size_t SHI::Print::println(uint16_t value, int base) {
  return print(value, base) + println();
}
size_t SHI::Print::println(int32_t value, int base) {
  return print(value, base) + println();
}
size_t SHI::Print::println(uint32_t value, int base) {
  return print(value, base) + println();
}
size_t SHI::Print::println(int64_t value, int base) {
  return print(value, base) + println();
}
size_t SHI::Print::println(uint64_t value, int base) {
  return print(value, base) + println();
}
size_t SHI::Print::println(double value, int precision) {
  return print(value, precision) + println();
}
size_t SHI::Print::println(void) { return print('\n'); }
size_t SHI::Print::println(struct tm* timeinfo, const char* format) {
  return print(timeinfo, format) + println();
}
