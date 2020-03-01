/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "SHIFactory.h"

#include <iostream>
#include <string>

#include "ArduinoJson.h"
#include "SHICommunicator.h"
#include "SHIHardware.h"
#include "SHISensor.h"

SHI::Factory *SHI::Factory::get() {
  if (instance == nullptr) instance = new Factory();
  return instance;
}

void SHI::Factory::construct(const std::string &json) {
  DeserializationError err = deserializeJson(doc, json);
  if (err) {
    // DO SOMETHING!
    // SHI::hw is not initialized yet, so...
    return;
  }
  JsonObject obj = doc.as<JsonObject>();
  auto hwObj = obj["hw"];
  auto facObj = factories["hw"](hwObj);
  SHI::hw = static_cast<SHI::Hardware *>(facObj);
}

bool SHI::Factory::registerFactory(const std::string &name,
                                   SHI::factoryFunction factory) {
  factories[name] = factory;
  return true;
}

SHI::Factory *SHI::Factory::instance = nullptr;

SHI::Hardware *SHI::Factory::defaultHardwareFactory(SHI::Hardware *hardware,
                                                    const JsonObject &obj) {
  std::cout << __func__ << std::endl;
  JsonArray sensors = obj["$sensors"];
  for (JsonObject sensorObj : sensors) {
    std::cout << __func__ << sensorObj << std::endl;
    for (auto kv : sensorObj) {
      std::string className = kv.key().c_str();
      JsonObject arguments = kv.value();
      SHI::Sensor *rawSensor =
          static_cast<SHI::Sensor *>(instance->factories[className](arguments));
      hardware->addSensor(std::shared_ptr<SHI::Sensor>(rawSensor));
    }
  }
  JsonArray sensorGroups = obj["$groups"];
  for (JsonObject sensorGroupObj : sensorGroups) {
    std::cout << __func__ << sensorGroupObj << std::endl;
    for (auto kv : sensorGroupObj) {
      std::string className = kv.key().c_str();
      JsonObject arguments = kv.value();
      SHI::SensorGroup *rawSensorGroup = static_cast<SHI::SensorGroup *>(
          instance->factories[className](arguments));
      hardware->addSensorGroup(
          std::shared_ptr<SHI::SensorGroup>(rawSensorGroup));
    }
  }
  JsonArray comms = obj["$comms"];
  for (JsonObject commObj : comms) {
    std::cout << __func__ << commObj << std::endl;
    for (auto kv : commObj) {
      std::string className = kv.key().c_str();
      JsonObject arguments = kv.value();
      SHI::Communicator *rawSensor = static_cast<SHI::Communicator *>(
          instance->factories[className](arguments));
      hardware->addCommunicator(std::shared_ptr<SHI::Communicator>(rawSensor));
    }
  }
  return hardware;
}
SHI::Communicator *SHI::Factory::defaultCommunicatorFactory(
    SHI::Communicator *comm, const JsonObject &obj) {
  std::cout << __func__ << std::endl;
  return comm;
}
SHI::SensorGroup *SHI::Factory::defaultSensorGroupFactory(
    const JsonObject &obj) {
  std::string name = obj["name"];
  std::cout << __func__ << name << std::endl;
  auto group = new SHI::SensorGroup(name.c_str());
  JsonArray sensors = obj["$sensors"];
  for (JsonObject sensorObj : sensors) {
    std::cout << __func__ << sensorObj << std::endl;
    for (auto kv : sensorObj) {
      std::string className = kv.key().c_str();
      JsonObject arguments = kv.value();
      SHI::Sensor *rawSensor =
          static_cast<SHI::Sensor *>(instance->factories[className](arguments));
      group->addSensor(std::shared_ptr<SHI::Sensor>(rawSensor));
    }
  }
  return group;
}
SHI::Sensor *SHI::Factory::defaultSensorFactory(SHI::Sensor *sensor,
                                                const JsonObject &obj) {
  std::cout << __func__ << std::endl;
  return sensor;
}
