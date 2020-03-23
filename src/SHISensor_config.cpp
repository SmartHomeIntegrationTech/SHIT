/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

// WARNING, this is an automatically generated file!
// Don't change anything in here.
// Last update 2020-03-23

# include <iostream>
# include <string>


# include "SHISensor.h"
// Configuration implementation for class SHI::SensorGroupConfiguration

namespace {
    
}  // namespace

SHI::SensorGroupConfiguration::SensorGroupConfiguration(const JsonObject &obj):
      name(obj["name"] | "default")
  {}

void SHI::SensorGroupConfiguration::fillData(JsonObject &doc) const {
    doc["name"] = name;
}

int SHI::SensorGroupConfiguration::getExpectedCapacity() const {
  return JSON_OBJECT_SIZE(1);
}

