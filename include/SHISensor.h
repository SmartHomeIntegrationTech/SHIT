/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#pragma once
#include <stdio.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "ArduinoJson.h"
#include "SHIEventBus.h"
#include "SHIFactory.h"
#include "SHIHardware.h"

// SHI stands for SmartHomeIntegration
namespace SHI {

extern const std::string STATUS_ITEM;
extern const std::string STATUS_OK;

enum class SensorDataType { INT, FLOAT, STRING, STATUS };

enum class MeasurementDataState { VALID, NO_DATA, ERROR };

class MeasurementMetaData;

class Measurement {
 public:
  Measurement(float value, MeasurementMetaData *metaData,
              const char *floatRepresentation = "%0.1f")
      : stringRepresentation(toString(value, floatRepresentation)),
        metaData(metaData),
        floatValue(value),
        state(MeasurementDataState::VALID) {}
  Measurement(int value, MeasurementMetaData *metaData)
      : stringRepresentation(toString(value)),
        metaData(metaData),
        intValue(value),
        state(MeasurementDataState::VALID) {}
  Measurement(std::string value, MeasurementMetaData *metaData,
              bool error = false)
      : stringRepresentation(value),
        metaData(metaData),
        intValue(0),
        state(error ? MeasurementDataState::ERROR
                    : MeasurementDataState::VALID) {}
  explicit Measurement(MeasurementMetaData *metaData, bool error = false)
      : stringRepresentation(error ? "<ERROR>" : "<NO_DATA>"),
        metaData(metaData),
        intValue(0),
        state(error ? MeasurementDataState::ERROR
                    : MeasurementDataState::NO_DATA) {}

  std::string toTransmitString() const;
  const MeasurementMetaData *getMetaData() const { return metaData; }
  const MeasurementDataState getDataState() const { return state; }
  int getIntValue() const;
  float getFloatValue() const;

  const std::string stringRepresentation;

 protected:
  const MeasurementMetaData *metaData;
  union {
    const float floatValue;
    const int intValue;
  };
  const MeasurementDataState state;

 private:
  static std::string toString(int value) {
    char buf[2 + 8 * sizeof(int)];
    snprintf(buf, sizeof(buf), "%d", value);
    return std::string(buf);
  }
  static std::string toString(float value, const char *floatRepresentation) {
    char buf[33];
    snprintf(buf, sizeof(buf), floatRepresentation, value);
    return std::string(buf);
  }
};

class MeasurementMetaData : public SHIObject {
 public:
  MeasurementMetaData(const std::string &name, const std::string &unit,
                      SensorDataType type)
      : SHIObject(name, false), unit(unit), type(type) {}
  const std::string unit;
  SensorDataType type;
  void accept(Visitor &visitor) override;

  Measurement measuredFloat(float value);
  Measurement measuredInt(int value);
  Measurement measuredStr(std::string value, bool error = false);
  Measurement measuredNoData();
  Measurement measuredError();
  Configuration *getConfig() const override { return nullptr; }
  bool reconfigure(Configuration *newConfig) override { return false; }
  EventBus::SubscriberBuilder getSubscriberBuilder();
  EventBus::EventBuilder getEventBuilder();
};

class MeasurementBundle {
 public:
  MeasurementBundle(std::initializer_list<Measurement> data, SHIObject *src)
      : timeStamp(hw->getEpochInMs()), data(data), src(src) {}
  MeasurementBundle(std::vector<Measurement> &data, SHIObject *src)
      : timeStamp(hw->getEpochInMs()), data(data), src(src) {}
  uint64_t timeStamp = 0;
  std::vector<Measurement> data = {};
  SHIObject *src;
};

class Sensor : public SHIObject {
 public:
  virtual std::vector<MeasurementBundle> readSensor() = 0;
  virtual bool setupSensor() = 0;
  virtual bool stopSensor() = 0;
  void accept(Visitor &visitor) override;
  virtual std::vector<std::shared_ptr<MeasurementMetaData>> *getMetaData();
  Sensor(const Sensor &) = delete;
  Sensor(Sensor &&) = delete;
  Sensor &operator=(const Sensor &) = delete;
  Sensor &operator=(Sensor &&) = delete;
  virtual ~Sensor() = default;

 protected:
  explicit Sensor(const std::string &name) : SHIObject(name) {}
  void addMetaData(std::shared_ptr<MeasurementMetaData> meta);
  std::vector<std::shared_ptr<MeasurementMetaData>> metaData;
};

class Configuration;

class SensorGroupConfiguration : public Configuration {
 public:
  explicit SensorGroupConfiguration(const JsonObject &obj);
  explicit SensorGroupConfiguration(const std::string &name) : name(name) {}
  void fillData(JsonObject &doc) const override;
  std::string name = "default";

 protected:
  int getExpectedCapacity() const override;
};

class SensorGroup : public SHIObject {
 public:
  explicit SensorGroup(const std::string &name)
      : SHIObject(name, false), config(SensorGroupConfiguration(name)) {}
  SensorGroup(const std::string &name,
              std::initializer_list<std::shared_ptr<Sensor>> sensors)
      : SHIObject(name), config(SensorGroupConfiguration(name)) {
    for (auto &&sensor : sensors) {
      addSensor(sensor);
    }
  }
  void accept(Visitor &visitor) override;
  void addSensor(std::shared_ptr<Sensor> sensor);
  std::vector<std::shared_ptr<Sensor>> *getSensors() { return &sensors; }
  const Configuration *getConfig() const override { return &config; }
  bool reconfigure(Configuration *newConfig) override {
    config = castConfig<SensorGroupConfiguration>(newConfig);
    return true;
  }
  SensorGroupConfiguration config;

 private:
  std::vector<std::shared_ptr<Sensor>> sensors;
};

}  // namespace SHI
