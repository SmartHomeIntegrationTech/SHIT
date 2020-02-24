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

void SHI::SensorGroup::addSensor(std::shared_ptr<SHI::Sensor> sensor) {
  sensor->setParent(this);
  sensors.push_back(sensor);
}
