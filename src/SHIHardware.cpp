/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include "SHIHardware.h"

#include <string.h>

#include "SHICommunicator.h"
#include "SHISensor.h"

void SHI::Hardware::internalLoop() {
  bool sensorHasFatalError = false;
  for (auto &&sensor : sensors) {
    logInfo(name, __func__, std::string("Reading sensor:") + sensor->getName());
    auto reading = sensor->readSensor();
    for (auto &&mb : reading) {
      for (auto &&comm : communicators) {
        comm->newReading(mb, *sensor);
      }
    }
    auto status = sensor->getStatusMessage();
    if (strcmp(status, STATUS_OK) != 0) {
      auto isFatal = sensor->errorIsFatal();
      for (auto &&comm : communicators) {
        comm->newStatus(*sensor, status, isFatal);
      }
      if (isFatal) {
        sensorHasFatalError = true;
        logError(name, __func__,
                 std::string("Sensor ") + sensor->getName() +
                     " reported error " + status);
      } else {
        logWarn(name, __func__,
                std::string("Sensor ") + sensor->getName() +
                    " reported warning " + status);
      }
    }
  }
  for (auto &&comm : communicators) {
    comm->loopCommunication();
  }
  while (sensorHasFatalError) {
    errLeds();
  }
}
