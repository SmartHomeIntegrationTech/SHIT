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

#include "SHIHardware.h"
#include "SHIObject.h"

// SHI stands for SmartHomeIntegration
namespace SHI {

extern const char *STATUS_ITEM;
extern const char *STATUS_OK;

enum class SensorDataType { INT, FLOAT, STRING, STATUS };

enum class MeasurementDataState { VALID, NO_DATA, ERROR };

class MeasurementMetaData;

class Measurement {
 public:
  Measurement(float value, MeasurementMetaData *metaData)
      : stringRepresentation(toString(value)),
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
  Measurement(const Measurement &copy) = default;
  Measurement(Measurement &&move) = default;
  std::string toTransmitString() const;
  const MeasurementMetaData *getMetaData() const { return metaData; }
  const MeasurementDataState getDataState() const { return state; }

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
  static std::string toString(float value) {
    char buf[33];
    snprintf(buf, sizeof(buf), "%0.1f", value);
    return std::string(buf);
  }
};

class MeasurementMetaData : public SHIObject {
 public:
  MeasurementMetaData(const char *name, const char *unit, SensorDataType type)
      : SHIObject(name, false), unit(unit), type(type) {}
  MeasurementMetaData(const MeasurementMetaData &meta) = default;
  MeasurementMetaData(MeasurementMetaData &&meta) = default;
  const char *unit;
  SensorDataType type;
  void accept(Visitor &visitor) override;

  Measurement measuredFloat(float value);
  Measurement measuredInt(int value);
  Measurement measuredStr(std::string value, bool error = false);
  Measurement measuredNoData();
  Measurement measuredError();
};

class MeasurementBundle {
 public:
  MeasurementBundle(std::initializer_list<Measurement> data, SHIObject *src)
      : timeStamp(hw->getEpochInMs()), data(data), src(src) {}
  MeasurementBundle(std::vector<Measurement> &data, SHIObject *src)
      : timeStamp(hw->getEpochInMs()), data(data), src(src) {}
  MeasurementBundle(const MeasurementBundle &readings) = default;
  MeasurementBundle(MeasurementBundle &&readings) = default;
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

 protected:
  explicit Sensor(const char *name) : SHIObject(name) {}
  void addMetaData(std::shared_ptr<MeasurementMetaData> meta);
  std::vector<std::shared_ptr<MeasurementMetaData>> metaData;
};

class SensorGroup : public SHIObject {
 public:
  explicit SensorGroup(const char *name) : SHIObject(name, false) {}
  SensorGroup(const char *name,
              std::initializer_list<std::shared_ptr<Sensor>> sensors)
      : SHIObject(name) {
    for (auto &&sensor : sensors) {
      addSensor(sensor);
    }
  }
  void accept(Visitor &visitor) override;
  void addSensor(std::shared_ptr<Sensor> sensor);
  std::vector<std::shared_ptr<Sensor>> *getSensors() { return &sensors; }

 private:
  std::vector<std::shared_ptr<Sensor>> sensors;
};

}  // namespace SHI
