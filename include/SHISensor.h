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

#include "SHIObject.h"

// SHI stands for SmartHomeIntegration
namespace SHI {

extern const char *STATUS_ITEM;
extern const char *STATUS_OK;

enum class SensorDataType { INT, FLOAT, STRING };

enum class MeasurementDataState { VALID, NO_DATA, ERROR };

class MeasurementMetaData;

class Measurement {
 public:
  Measurement(float value, MeasurementMetaData *metaData)
      : metaData(metaData),
        floatValue(value),
        state(MeasurementDataState::VALID),
        stringRepresentation(toString(value)) {}
  Measurement(int value, MeasurementMetaData *metaData)
      : metaData(metaData),
        intValue(value),
        state(MeasurementDataState::VALID),
        stringRepresentation(toString(value)) {}
  Measurement(std::string value, MeasurementMetaData *metaData)
      : metaData(metaData),
        intValue(0),
        state(MeasurementDataState::VALID),
        stringRepresentation(value) {}
  explicit Measurement(MeasurementMetaData *metaData, bool error = false)
      : metaData(metaData),
        intValue(0),
        state(error ? MeasurementDataState::ERROR
                    : MeasurementDataState::NO_DATA),
        stringRepresentation(error ? "<ERROR>" : "<NO_DATA>") {}
  Measurement(const Measurement &copy) = default;
  Measurement(Measurement &&move) = default;
  std::string toTransmitString() const { return stringRepresentation; }
  const MeasurementMetaData *getMetaData() const { return metaData; }
  const MeasurementDataState getDataState() const { return state; }

 protected:
  const MeasurementMetaData *metaData;
  union {
    const float floatValue;
    const int intValue;
  };
  const MeasurementDataState state;
  const std::string stringRepresentation;

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

class MeasurementMetaData {
 public:
  MeasurementMetaData(const char *name, const char *unit, SensorDataType type)
      : name(name), unit(unit), type(type) {}
  const char *name;
  const char *unit;
  SensorDataType type;
  Measurement measuredFloat(float value) { return Measurement(value, this); }
  Measurement measuredInt(int value) { return Measurement(value, this); }
  Measurement measuredStr(std::string value) {
    return Measurement(value, this);
  }
  Measurement measuredNoData() { return Measurement(this, false); }
  Measurement measuredError() { return Measurement(this, true); }
};

class MeasurementBundle {
 public:
  MeasurementBundle(std::vector<SHI::Measurement> &&data, SHI::SHIObject *src)
      : timeStamp(0), data(data), src(src) {}
  MeasurementBundle(const MeasurementBundle &readings) = default;
  MeasurementBundle(MeasurementBundle &&readings) = default;
  uint32_t timeStamp = 0;
  std::vector<SHI::Measurement> data = {};
  SHI::SHIObject *src;
};

class Sensor : public SHI::SHIObject {
 public:
  virtual std::vector<MeasurementBundle> readSensor() = 0;
  virtual bool setupSensor() = 0;
  virtual bool stopSensor() = 0;
  virtual const char *getStatusMessage() const { return statusMessage; }
  virtual bool errorIsFatal() const { return fatalError; }

 protected:
  explicit Sensor(const char *name) : SHIObject(name) {}
  const char *statusMessage = SHI::STATUS_OK;
  bool fatalError = false;
};

class Channel : public SHI::Sensor {
 public:
  Channel(std::shared_ptr<Sensor> sensor, const char *name)
      : Sensor(name), sensor(sensor) {
    internalName = std::string(name) + sensor->getName();
  }
  std::vector<MeasurementBundle> readSensor() override;
  bool setupSensor() override;
  bool stopSensor() override;
  void accept(SHI::Visitor &visitor) override;
  const char *getStatusMessage() const override;
  bool errorIsFatal() const override;
  const char *getName() const override;
  const std::shared_ptr<Sensor> sensor;

 private:
  std::string internalName;
};

}  // namespace SHI
