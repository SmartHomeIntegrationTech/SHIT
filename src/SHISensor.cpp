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

void SHI::Channel::accept(SHI::Visitor& visitor) {
  visitor.visit(this);
  sensor->accept(visitor);
}

std::vector<SHI::MeasurementBundle> SHI::Channel::readSensor() {
  return sensor->readSensor();
}
bool SHI::Channel::setupSensor() { return sensor->setupSensor(); }
bool SHI::Channel::stopSensor() { return sensor->stopSensor(); }

const char* SHI::Channel::getStatusMessage() const {
  return sensor->getStatusMessage();
}
bool SHI::Channel::errorIsFatal() const { return sensor->errorIsFatal(); }
const char* SHI::Channel::getName() const { return internalName.c_str(); }
