/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "SHIFactory.h"
#include "SHIObject.h"

namespace SHI {

#define HEX 16
#define DEC 10
#define OCT 8
#define BIN 2

class Print {
 private:
  int writeError;
  size_t printUnsigned(uint64_t value, int base);
  size_t printSigned(int64_t value, int base);
  template <typename T>
  size_t internalPrint(T val, std::true_type, int base) {
    return printUnsigned(static_cast<uint64_t>(val), base);
  }
  template <typename T>
  size_t internalPrint(T val, std::false_type, int base) {
    return printSigned(static_cast<int64_t>(val), base);
  }

  template <typename T>
  size_t internalPrintln(T val, std::true_type, int base) {
    return printUnsigned(static_cast<uint64_t>(val), base) + println();
  }
  template <typename T>
  size_t internalPrintln(T val, std::false_type, int base) {
    return printSigned(static_cast<int64_t>(val), base) + println();
  }

 public:
  int getWriteError();
  void clearWriteError();

  virtual size_t write(uint8_t) = 0;
  size_t write(const char str[]);
  virtual size_t write(const uint8_t* buffer, size_t size) = 0;
  size_t write(const char* buffer, size_t size);

  template <typename T>
  size_t write(T c) {
    return write(static_cast<uint8_t>(c));
  }
  // Enable write(char) to fall through to write(uint8_t)
  size_t write(char c);

  size_t printf(const char* format, ...) __attribute__((format(printf, 2, 3)));
  size_t print(const char value[]);
  size_t print(char value);
  template <typename T>
  size_t print(T value, int base = DEC) {
    return internalPrint(value, std::is_unsigned<T>(), base);
  }
  size_t print(double value, int precision = 2);
  size_t print(struct tm* timeinfo, const char* format);

  size_t println(const char value[]);
  size_t println(char value);
  template <typename T>
  size_t println(T value, int base = DEC) {
    return internalPrintln(value, std::is_unsigned<T>(), base);
  }
  size_t println(double value, int precision = 2);
  size_t println(void);
  size_t println(struct tm* timeinfo, const char* format);
  virtual void flush() {}
};

class Bus : public SHIObject {
  virtual void begin(Configuration* config) = 0;
  virtual void stop() = 0;
  virtual void loop() = 0;
  virtual std::vector<std::pair<int, std::string>> getUsedPins() = 0;
};

class SerialBus : public Bus, Print {
  virtual int available(void) = 0;
  virtual int availableForWrite(void) = 0;
  virtual int peek(void) = 0;
  virtual int read(void) = 0;
  virtual size_t read(uint8_t* buffer, size_t size) = 0;

  virtual void flush(bool txOnly) = 0;
  virtual size_t write(uint8_t) = 0;
  virtual size_t write(const uint8_t* buffer, size_t size) = 0;
};

class SPIBus : public Bus {
  virtual void beginTransaction(Configuration* settings) = 0;
  virtual void endTransaction(void) = 0;
  virtual void transfer(uint8_t* data, uint32_t size) = 0;
  virtual uint8_t transfer(uint8_t data) = 0;
  virtual uint16_t transfer16(uint16_t data) = 0;
  virtual uint32_t transfer32(uint32_t data) = 0;

  virtual void transferBytes(const uint8_t* data, uint8_t* out,
                             uint32_t size) = 0;
  virtual void transferBits(uint32_t data, uint32_t* out, uint8_t bits) = 0;

  virtual void write(uint8_t data) = 0;
  virtual void write16(uint16_t data) = 0;
  virtual void write32(uint32_t data) = 0;
  virtual void writeBytes(const uint8_t* data, uint32_t size) = 0;
  virtual void writePattern(const uint8_t* data, uint8_t size,
                            uint32_t repeat) = 0;
};

enum class I2CError {
  I2C_ERROR_OK = 0,
  I2C_ERROR_DEV,
  I2C_ERROR_ACK,
  I2C_ERROR_TIMEOUT,
  I2C_ERROR_BUS,
  I2C_ERROR_BUSY,
  I2C_ERROR_MEMORY,
  I2C_ERROR_CONTINUE,
  I2C_ERROR_NO_BEGIN
};

class I2CBus : public Bus, Print {
  virtual uint8_t lastError() = 0;
  virtual char* getErrorText(uint8_t err) = 0;

  virtual I2CError writeTransmission(uint16_t address, uint8_t* buff,
                                     uint16_t size, bool sendStop = true) = 0;
  virtual I2CError readTransmission(uint16_t address, uint8_t* buff,
                                    uint16_t size, bool sendStop = true,
                                    uint32_t* readCount = NULL) = 0;

  virtual void beginTransmission(uint16_t address) = 0;
  virtual uint8_t endTransmission(bool sendStop = true) = 0;

  virtual uint8_t requestFrom(uint16_t address, uint8_t size,
                              bool sendStop) = 0;

  virtual size_t write(uint8_t) = 0;
  virtual size_t write(const uint8_t*, size_t) = 0;
  virtual int available(void) = 0;
  virtual int read(void) = 0;
  virtual int peek(void) = 0;
  virtual void flush(void) = 0;

  virtual void onReceive(void (*)(int)) = 0;
  virtual void onRequest(void (*)(void)) = 0;

  virtual bool busy() = 0;
};

}  // namespace SHI
