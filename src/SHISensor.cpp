/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include "SHISensor.h"

#include <stdio.h>

namespace SHI {

const char* STATUS_ITEM = "Status";
const char* STATUS_OK = "OK";

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
  if (metaData->type == SensorDataType::STRING)
    return "\"" + stringRepresentation + "\"";
  return stringRepresentation;
}

void SHI::Sensor::addMetaData(std::shared_ptr<SHI::MeasurementMetaData> meta) {
  meta->setParent(this);
  metaData.push_back(meta);
}
