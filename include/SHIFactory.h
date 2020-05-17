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
#include <stack>
#include <string>
#include <tuple>
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
  MissingRegistryForHW,
  /// There is no entry for the requested hardware
  MissingRegistryForEntry
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

typedef std::tuple<SHIObject *, SHI::FactoryErrors> FactoryResult;
typedef std::function<FactoryResult(const JsonObject &obj)> factoryFunction;

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
  FactoryResult construct(const std::string &json);
  FactoryResult defaultHardwareFactory(SHI::Hardware *hardware,
                                       const JsonObject &obj);
  FactoryResult defaultCommunicatorFactory(SHI::Communicator *comm,
                                           const JsonObject &obj);
  FactoryResult defaultSensorGroupFactory(const JsonObject &obj);
  FactoryResult defaultSensorFactory(SHI::Sensor *hardware,
                                     const JsonObject &obj);
  static SHI::FactoryErrors getError(FactoryResult result);
  template <typename T>
  static T *getInstance(FactoryResult result);
  FactoryResult objToResult(SHIObject *obj) {
    return std::tuple<SHI::SHIObject *, FactoryErrors>(obj,
                                                       FactoryErrors::None);
  }
  FactoryResult errorToResult(FactoryErrors result) {
    return std::tuple<SHI::SHIObject *, FactoryErrors>(nullptr, result);
  }
  FactoryResult errorToResult(FactoryResult result) {
    return std::tuple<SHI::SHIObject *, FactoryErrors>(nullptr,
                                                       getError(result));
  }

 private:
  Factory() {}
  Factory(const Factory &copy) = delete;
  ~Factory() {}
  static Factory *instance;
  std::map<std::string, factoryFunction> factories;
  std::tuple<SHIObject *, SHI::FactoryErrors> callFactory(
      const ArduinoJson::JsonObject &arguments, const std::string &className);
  DynamicJsonDocument doc{5000};
};

class ConfigurationVisitor : public Visitor {
 public:
  std::string toJson() const;

 protected:
  void enterVisit(Sensor *sensor) override;
  void leaveVisit(Sensor *sensor) override;
  void enterVisit(SensorGroup *channel) override;
  void leaveVisit(SensorGroup *channel) override;
  void enterVisit(Hardware *harwdware) override;
  void leaveVisit(Hardware *harwdware) override;
  void visit(Communicator *communicator) override;
  void visit(MeasurementMetaData *data) override;

 private:
  struct Record {
    std::vector<std::shared_ptr<Record>> sensors;
    std::vector<std::shared_ptr<Record>> comms;
    std::vector<std::shared_ptr<Record>> groups;
    DynamicJsonDocument doc{5000};
    JsonObject buildTree();
  };
  std::stack<std::shared_ptr<Record>> stack;
  std::shared_ptr<Record> currentNode;
};

}  // namespace SHI
