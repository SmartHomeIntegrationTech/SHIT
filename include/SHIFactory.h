/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include <stdio.h>

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ArduinoJson.h"
#include "SHIObject.h"

namespace SHI {

enum class FactoryErrors {
  /// All went according to plan
  None = 0,
  /// The JSON presented to construct was not valid
  FailureToParseJson,
  /// The JSON for construct did not contain the root hw element
  NoHWKeyFound,
  /// The type of the hw root element was incorrect
  InvalidHWKeyFound,
  /// There seems to be no hardware registered
  MissingRegistryForHW
};

class Configuration {
 public:
  virtual std::string toJson() const;
  virtual void printJson(std::ostream printer) const;
  virtual void fillData(
      JsonObject &obj) const = 0;  // NOLINT Yes, non constant reference
 protected:
  virtual int getExpectedCapacity() const = 0;
};

typedef std::function<SHI::SHIObject *(const JsonObject &obj)> factoryFunction;

class Factory {
 public:
  static Factory *get();
  /// This is used for testing to ensure that all tests are happening under the
  /// same conditions
  static void reset() {
    delete instance;
    instance = nullptr;
  }
  bool registerFactory(const std::string &name, factoryFunction factory);
  FactoryErrors construct(const std::string &json);

  static SHI::Hardware *defaultHardwareFactory(SHI::Hardware *hardware,
                                               const JsonObject &obj);
  static SHI::Communicator *defaultCommunicatorFactory(SHI::Communicator *comm,
                                                       const JsonObject &obj);
  static SHI::SensorGroup *defaultSensorGroupFactory(const JsonObject &obj);
  static SHI::Sensor *defaultSensorFactory(SHI::Sensor *hardware,
                                           const JsonObject &obj);

 private:
  Factory() {}
  Factory(const Factory &copy) = delete;
  ~Factory() {}
  static Factory *instance;
  std::map<std::string, factoryFunction> factories;

  DynamicJsonDocument doc{5000};
};

}  // namespace SHI
