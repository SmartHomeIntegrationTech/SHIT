/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include "SHISensor.h"

#include <stdio.h>

using SHI::Measurement;
using SHI::MeasurementMetaData;
using SHI::Sensor;
using SHI::SensorGroup;

namespace SHI {

const std::string STATUS_ITEM = "Status";  // NOLINT
const std::string STATUS_OK = "OK";        // NOLINT

}  // namespace SHI

void SensorGroup::accept(Visitor& visitor) {
  visitor.enterVisit(this);
  for (auto&& sensor : sensors) {
    sensor->accept(visitor);
  }
  visitor.leaveVisit(this);
}

void MeasurementMetaData::accept(Visitor& visitor) { visitor.visit(this); }

void Sensor::accept(Visitor& visitor) {
  visitor.enterVisit(this);
  status->accept(visitor);
  for (auto&& meta : metaData) {
    meta->accept(visitor);
  }
  visitor.leaveVisit(this);
}

void SensorGroup::addSensor(std::shared_ptr<Sensor> sensor) {
  sensor->setParent(this);
  sensors.push_back(sensor);
}

std::string Measurement::toTransmitString() const {
  switch (metaData->type) {
    case SensorDataType::STRING:
    case SensorDataType::STATUS:
      return "\"" + stringRepresentation + "\"";
    default:
      return stringRepresentation;
  }
}

int Measurement::getIntValue() const {
  switch (metaData->type) {
    case SensorDataType::FLOAT: {
      float value = floatValue;
      return value;
    }
    case SensorDataType::INT: {
      return intValue;
    }
    case SensorDataType::STRING:
    case SensorDataType::STATUS:
    default:
      return 0;
  }
}
float Measurement::getFloatValue() const {
  switch (metaData->type) {
    case SensorDataType::FLOAT: {
      return floatValue;
    }
    case SensorDataType::INT: {
      // use proper rounding
      int value = floatValue + 0.5;
      return value;
    }
    case SensorDataType::STATUS:
    case SensorDataType::STRING:
    default:
      return 0;
  }
}

void Sensor::addMetaData(std::shared_ptr<MeasurementMetaData> meta) {
  meta->setParent(this);
  metaData.push_back(meta);
}

std::vector<std::shared_ptr<MeasurementMetaData>>* Sensor::getMetaData() {
  return &metaData;
}

Measurement MeasurementMetaData::measuredFloat(float value) {
  return Measurement(value, this);
}
Measurement MeasurementMetaData::measuredInt(int value) {
  return Measurement(value, this);
}
Measurement MeasurementMetaData::measuredStr(std::string value, bool error) {
  return Measurement(value, this, error);
}
Measurement MeasurementMetaData::measuredNoData() {
  return Measurement(this, false);
}
Measurement MeasurementMetaData::measuredError() {
  return Measurement(this, true);
}
