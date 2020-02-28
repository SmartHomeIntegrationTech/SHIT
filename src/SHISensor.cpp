/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include "SHISensor.h"

#include <stdio.h>

namespace SHI {

const std::string STATUS_ITEM = "Status";  // NOLINT
const std::string STATUS_OK = "OK";        // NOLINT

}  // namespace SHI

void SHI::SensorGroup::accept(SHI::Visitor& visitor) {
  visitor.enterVisit(this);
  for (auto&& sensor : sensors) {
    sensor->accept(visitor);
  }
  visitor.leaveVisit(this);
}

void SHI::MeasurementMetaData::accept(SHI::Visitor& visitor) {
  visitor.visit(this);
}

void SHI::Sensor::accept(SHI::Visitor& visitor) {
  visitor.enterVisit(this);
  status->accept(visitor);
  for (auto&& meta : metaData) {
    meta->accept(visitor);
  }
  visitor.leaveVisit(this);
}

void SHI::SensorGroup::addSensor(std::shared_ptr<SHI::Sensor> sensor) {
  sensor->setParent(this);
  sensors.push_back(sensor);
}

std::string SHI::Measurement::toTransmitString() const {
  switch (metaData->type) {
    case SensorDataType::STRING:
    case SensorDataType::STATUS:
      return "\"" + stringRepresentation + "\"";
    default:
      return stringRepresentation;
  }
}

void SHI::Sensor::addMetaData(std::shared_ptr<SHI::MeasurementMetaData> meta) {
  meta->setParent(this);
  metaData.push_back(meta);
}

std::vector<std::shared_ptr<SHI::MeasurementMetaData>>*
SHI::Sensor::getMetaData() {
  return &metaData;
}

SHI::Measurement SHI::MeasurementMetaData::measuredFloat(float value) {
  return SHI::Measurement(value, this);
}
SHI::Measurement SHI::MeasurementMetaData::measuredInt(int value) {
  return SHI::Measurement(value, this);
}
SHI::Measurement SHI::MeasurementMetaData::measuredStr(std::string value,
                                                       bool error) {
  return SHI::Measurement(value, this, error);
}
SHI::Measurement SHI::MeasurementMetaData::measuredNoData() {
  return SHI::Measurement(this, false);
}
SHI::Measurement SHI::MeasurementMetaData::measuredError() {
  return SHI::Measurement(this, true);
}
