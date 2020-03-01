/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include <stdio.h>

#include <functional>
#include <map>
#include <string>

#include "ArduinoJson.h"
#include "SHIObject.h"

namespace SHI {

class Configuration {
  virtual std::string toJson() = 0;
  virtual void printJson(std::ostream printer) = 0;
  virtual void fillData(
      JsonDocument &doc) = 0;  // NOLINT Yes, non constant reference
};

typedef std::function<SHI::SHIObject *(const JsonObject &obj)> factoryFunction;

class Factory {
 public:
  static Factory *get();
  bool registerFactory(const std::string &name, factoryFunction factory);
  void construct(const std::string &json);

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
