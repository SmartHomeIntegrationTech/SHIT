/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

// WARNING, this is an automatically generated file!
// Don't change anything in here.
// Last update 2020-03-01

# include <iostream>
# include <string>


#include "SHISensor.h"
// Configuration implementation for class SHI::SensorGroupConfiguration

SHI::SensorGroupConfiguration::SensorGroupConfiguration(const JsonObject &obj):
       name(obj["name"] | "default")
  {}

void SHI::SensorGroupConfiguration::fillData(JsonDocument &doc) {
    doc["name"] = name;
}

namespace {
  const size_t SensorGroupConfigurationCapacity = JSON_OBJECT_SIZE(1);
}

std::string SHI::SensorGroupConfiguration::toJson() {
  DynamicJsonDocument doc(SensorGroupConfigurationCapacity);
  fillData(doc);
  char output[2000];
  serializeJson(doc, output, sizeof(output));
  return std::string(output);
}
void SHI::SensorGroupConfiguration::printJson(std::ostream printer) {
  DynamicJsonDocument doc(SensorGroupConfigurationCapacity);
  fillData(doc);
  serializeJson(doc, printer);
}
