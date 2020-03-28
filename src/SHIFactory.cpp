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
  auto config = sensor->getConfig();
  if (config != nullptr) {
    sensors.emplace_back(std::pair<const std::string, const Configuration *>{
        sensor->getName(), config});
  }
}
void ConfigurationVisitor::leaveVisit(Sensor *sensor) {}
void ConfigurationVisitor::enterVisit(SensorGroup *channel) {
  auto config = channel->getConfig();
  if (config != nullptr) {
    sensors.emplace_back(std::pair<std::string, const Configuration *>{
        channel->getName(), config});
  }
}
void ConfigurationVisitor::leaveVisit(SensorGroup *channel) {}
void ConfigurationVisitor::visit(Communicator *communicator) {
  auto config = communicator->getConfig();
  if (config != nullptr) {
    sensors.emplace_back(std::pair<std::string, const Configuration *>{
        communicator->getName(), config});
  }
}
void ConfigurationVisitor::enterVisit(Hardware *hardware) {}
void ConfigurationVisitor::leaveVisit(Hardware *hardware) {
  auto root = doc.as<JsonObject>();
  auto hw = root.createNestedObject("hw");
  hardware->getConfig()->fillData(hw);
}
void ConfigurationVisitor::visit(MeasurementMetaData *data) {}

Factory *Factory::get() {
  if (instance == nullptr) instance = new Factory();
  return instance;
}

FactoryErrors Factory::construct(const std::string &json) {
  DeserializationError err = deserializeJson(doc, json);
  if (err) {
    return FactoryErrors::FailureToParseJson;
  }
  JsonObject obj = doc.as<JsonObject>();
  if (!obj.containsKey("hw")) return FactoryErrors::NoHWKeyFound;
  auto hwObj = obj["hw"];
  if (!hwObj.is<JsonObject>()) return FactoryErrors::InvalidHWKeyFound;
  auto factoryFind = factories.find("hw");
  if (factoryFind != factories.end()) {
    auto facObj = factoryFind->second(hwObj);
    hw = static_cast<Hardware *>(facObj);
    return FactoryErrors::None;
  }
  return FactoryErrors::MissingRegistryForHW;
}

bool Factory::registerFactory(const std::string &name,
                              factoryFunction factory) {
  factories[name] = factory;
  return true;
}

Factory *Factory::instance = nullptr;

Hardware *Factory::defaultHardwareFactory(Hardware *hardware,
                                          const JsonObject &obj) {
  hw = hardware;
  std::cout << __func__ << std::endl;
  JsonArray sensors = obj["$sensors"];
  for (JsonObject sensorObj : sensors) {
    std::cout << __func__ << sensorObj << std::endl;
    for (auto kv : sensorObj) {
      std::string className = kv.key().c_str();
      JsonObject arguments = kv.value();
      Sensor *rawSensor =
          static_cast<Sensor *>(instance->factories[className](arguments));
      hardware->addSensor(std::shared_ptr<Sensor>(rawSensor));
    }
  }
  JsonArray sensorGroups = obj["$groups"];
  for (JsonObject sensorGroupObj : sensorGroups) {
    std::cout << __func__ << sensorGroupObj << std::endl;
    for (auto kv : sensorGroupObj) {
      std::string className = kv.key().c_str();
      JsonObject arguments = kv.value();
      SensorGroup *rawSensorGroup =
          static_cast<SensorGroup *>(instance->factories[className](arguments));
      hardware->addSensorGroup(std::shared_ptr<SensorGroup>(rawSensorGroup));
    }
  }
  JsonArray comms = obj["$comms"];
  for (JsonObject commObj : comms) {
    std::cout << __func__ << commObj << std::endl;
    for (auto kv : commObj) {
      std::string className = kv.key().c_str();
      JsonObject arguments = kv.value();
      Communicator *rawSensor = static_cast<Communicator *>(
          instance->factories[className](arguments));
      hardware->addCommunicator(std::shared_ptr<Communicator>(rawSensor));
    }
  }
  return hardware;
}

Communicator *Factory::defaultCommunicatorFactory(Communicator *comm,
                                                  const JsonObject &obj) {
  std::cout << __func__ << std::endl;
  return comm;
}

SensorGroup *Factory::defaultSensorGroupFactory(const JsonObject &obj) {
  std::string name = obj["name"];
  std::cout << __func__ << name << std::endl;
  auto group = new SensorGroup(name.c_str());
  JsonArray sensors = obj["$sensors"];
  for (JsonObject sensorObj : sensors) {
    std::cout << __func__ << sensorObj << std::endl;
    for (auto kv : sensorObj) {
      std::string className = kv.key().c_str();
      JsonObject arguments = kv.value();
      Sensor *rawSensor =
          static_cast<Sensor *>(instance->factories[className](arguments));
      group->addSensor(std::shared_ptr<Sensor>(rawSensor));
    }
  }
  return group;
}

Sensor *Factory::defaultSensorFactory(Sensor *sensor, const JsonObject &obj) {
  std::cout << __func__ << std::endl;
  return sensor;
}

std::string Configuration::toJson() const {
  DynamicJsonDocument doc(getExpectedCapacity());
  auto root = doc.as<JsonObject>();
  fillData(root);
  char output[2000];
  serializeJson(doc, output, sizeof(output));
  return std::string(output);
}

void Configuration::printJson(std::ostream printer) const {
  DynamicJsonDocument doc(getExpectedCapacity());
  auto root = doc.as<JsonObject>();
  fillData(root);
  serializeJson(doc, printer);
}
