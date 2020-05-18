/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "SHIFactory.h"

#include <iostream>
#include <string>
#include <utility>

#include "ArduinoJson.h"
#include "SHICommunicator.h"
#include "SHIHardware.h"
#include "SHISensor.h"

using SHI::Communicator;
using SHI::Configuration;
using SHI::ConfigurationVisitor;
using SHI::Factory;
using SHI::FactoryErrors;
using SHI::Hardware;
using SHI::Sensor;
using SHI::SensorGroup;

void ConfigurationVisitor::enterVisit(Sensor *sensor) {
  auto ptr = std::make_shared<Record>();
  auto config = sensor->getConfig();
  auto sensorConfig = ptr->doc.createNestedObject(sensor->getName());
  if (config != nullptr) config->fillData(sensorConfig);
  currentNode->sensors.push_back(ptr);
}
void ConfigurationVisitor::leaveVisit(Sensor *sensor) {}

void ConfigurationVisitor::enterVisit(SensorGroup *group) {
  auto ptr = std::make_shared<Record>();
  auto config = group->getConfig();
  auto sensorConfig = ptr->doc.createNestedObject("sensorGroup");
  if (config != nullptr) config->fillData(sensorConfig);

  currentNode->groups.push_back(ptr);
  stack.push(currentNode);
  currentNode = ptr;
}
void ConfigurationVisitor::leaveVisit(SensorGroup *channel) {
  currentNode = stack.top();
  stack.pop();
}

void ConfigurationVisitor::visit(Communicator *communicator) {
  auto ptr = std::make_shared<Record>();
  auto config = communicator->getConfig();
  auto sensorConfig = ptr->doc.createNestedObject(communicator->getName());
  if (config != nullptr) config->fillData(sensorConfig);
  currentNode->comms.push_back(ptr);
}
void ConfigurationVisitor::enterVisit(Hardware *hardware) {
  auto ptr = std::make_shared<Record>();
  currentNode = ptr;
  auto hwNode = currentNode->doc.createNestedObject("hw");
  auto config = hardware->getConfig();
  if (config != nullptr) config->fillData(hwNode);
}
void ConfigurationVisitor::leaveVisit(Hardware *hardware) {}
void ConfigurationVisitor::visit(MeasurementMetaData *data) {}

JsonObject ConfigurationVisitor::Record::buildTree() {
  for (auto kv : doc.as<JsonObject>()) {
    auto rootNode = kv.value();
    if (comms.size() != 0) {
      auto commsArray = rootNode.createNestedArray("$comms");
      for (auto &&commNode : comms) {
        commsArray.add(commNode->buildTree());
      }
    }
    if (groups.size() != 0) {
      auto groupsArray = rootNode.createNestedArray("$groups");
      for (auto &&groupNode : groups) {
        groupsArray.add(groupNode->buildTree());
      }
    }
    if (sensors.size() != 0) {
      auto sensorsArray = rootNode.createNestedArray("$sensors");
      for (auto &&sensorNode : sensors) {
        sensorsArray.add(sensorNode->buildTree());
      }
    }
  }
  return doc.as<JsonObject>();
}

Factory *Factory::get() {
  if (instance == nullptr) instance = new Factory();
  return instance;
}

SHI::FactoryErrors Factory::getError(SHI::FactoryResult result) {
  return std::get<1>(result);
}

const char *Factory::errorToString(SHI::FactoryErrors error) {
  static const char *FACTORY_ERROR_NAMES[] = {"None",
                                              "FailureToParseJson",
                                              "NoHWKeyFound",
                                              "InvalidHWKeyFound",
                                              "MissingRegistryForHW",
                                              "MissingRegistryForEntry",
                                              "FailureToLoadFile"};
  return FACTORY_ERROR_NAMES[static_cast<int>(error)];
}

template <typename T>
T *Factory::getInstance(SHI::FactoryResult result) {
  return static_cast<T *>(std::get<0>(result));
}

SHI::FactoryResult Factory::construct(const std::string &json) {
  DeserializationError err = deserializeJson(doc, json);
  if (err) {
    return errorToResult(FactoryErrors::FailureToParseJson);
  }
  JsonObject obj = doc.as<JsonObject>();
  if (!obj.containsKey("hw")) return errorToResult(FactoryErrors::NoHWKeyFound);
  auto hwObj = obj["hw"];
  if (!hwObj.is<JsonObject>())
    return errorToResult(FactoryErrors::InvalidHWKeyFound);
  auto factoryFind = factories.find("hw");
  if (factoryFind != factories.end()) {
    auto factory = factoryFind->second;
    FactoryResult result = factory(hwObj);
    if (getError(result) == FactoryErrors::None)
      hw = getInstance<Hardware>(result);
    return result;
  }
  return errorToResult(FactoryErrors::MissingRegistryForHW);
}

bool Factory::registerFactory(const std::string &name,
                              factoryFunction factory) {
  factories[name] = factory;
  return true;
}

Factory *Factory::instance = nullptr;

SHI::FactoryResult Factory::callFactory(
    const ArduinoJson::JsonObject &arguments, const std::string &className) {
  if (factories.find(className) == factories.end())
    return errorToResult(FactoryErrors::MissingRegistryForEntry);
  auto factory = factories[className];
  return factory(arguments);
}

SHI::FactoryResult Factory::defaultHardwareFactory(Hardware *hardware,
                                                   const JsonObject &obj) {
  hw = hardware;
  JsonArray sensors = obj["$sensors"];
  for (JsonObject sensorObj : sensors) {
    for (auto kv : sensorObj) {
      std::string className = kv.key().c_str();
      JsonObject arguments = kv.value();
      SHI::FactoryResult result = callFactory(arguments, className);
      if (getError(result) != FactoryErrors::None) return errorToResult(result);
      hardware->addSensor(std::shared_ptr<Sensor>(getInstance<Sensor>(result)));
    }
  }
  JsonArray sensorGroups = obj["$groups"];
  for (JsonObject sensorGroupObj : sensorGroups) {
    for (auto kv : sensorGroupObj) {
      std::string className = kv.key().c_str();
      JsonObject arguments = kv.value();
      SHI::FactoryResult result = callFactory(arguments, className);
      if (getError(result) != FactoryErrors::None) return errorToResult(result);
      hardware->addSensorGroup(
          std::shared_ptr<SensorGroup>(getInstance<SensorGroup>(result)));
    }
  }
  JsonArray comms = obj["$comms"];
  for (JsonObject commObj : comms) {
    for (auto kv : commObj) {
      std::string className = kv.key().c_str();
      JsonObject arguments = kv.value();
      auto result = callFactory(arguments, className);
      if (getError(result) != FactoryErrors::None) return errorToResult(result);
      hardware->addCommunicator(
          std::shared_ptr<Communicator>(getInstance<Communicator>(result)));
    }
  }
  return objToResult(hw);
}

SHI::FactoryResult Factory::defaultCommunicatorFactory(Communicator *comm,
                                                       const JsonObject &obj) {
  return objToResult(comm);
}

SHI::FactoryResult Factory::defaultSensorGroupFactory(const JsonObject &obj) {
  std::string name = obj["name"];
  auto group = new SensorGroup(name.c_str());
  JsonArray sensors = obj["$sensors"];
  for (JsonObject sensorObj : sensors) {
    for (auto kv : sensorObj) {
      std::string className = kv.key().c_str();
      JsonObject arguments = kv.value();
      auto result = callFactory(arguments, className);
      if (getError(result) != FactoryErrors::None) {
        delete group;
        return errorToResult(result);
      }
      group->addSensor(std::shared_ptr<Sensor>(getInstance<Sensor>(result)));
    }
  }
  return objToResult(group);
}

SHI::FactoryResult Factory::defaultSensorFactory(Sensor *sensor,
                                                 const JsonObject &obj) {
  return objToResult(sensor);
}

std::string Configuration::toJson() const {
  DynamicJsonDocument doc(getExpectedCapacity());
  auto root = doc.as<JsonObject>();
  fillData(root);
  char output[2000];
  serializeJson(doc, output, sizeof(output));
  return std::string(output);
}

std::string ConfigurationVisitor::toJson() const {
  char output[2000];
  auto node = currentNode->buildTree();
  serializeJsonPretty(node, output, sizeof(output));
  return std::string(output);
}

void Configuration::printJson(std::ostream printer) const {
  DynamicJsonDocument doc(getExpectedCapacity());
  auto root = doc.as<JsonObject>();
  fillData(root);
  serializeJson(doc, printer);
}
