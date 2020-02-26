/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include "SHIObject.h"

#include <string>

#include "SHISensor.h"

const SHI::Measurement NODATA(nullptr, false);

SHI::SHIObject::SHIObject(const char *name, bool initStatus) : name(name) {
  if (initStatus) {
    status = std::make_shared<MeasurementMetaData>(STATUS_ITEM, "",
                                                   SensorDataType::STATUS);
    status->setParent(this);
  }
}

std::string SHI::SHIObject::getQualifiedName(const char *seperator) const {
  if (parent != nullptr) {
    return std::string(parent->getQualifiedName(seperator)) + seperator +
           getName();
  }
  return std::string(getName());
}

SHI::Measurement SHI::SHIObject::getStatus() {
  if (status.get() == nullptr) return NODATA;
  return status->measuredStr(statusMessage, fatalError);
}
