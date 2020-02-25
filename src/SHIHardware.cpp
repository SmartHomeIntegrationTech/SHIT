/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include "SHIHardware.h"

#include <string.h>

#include "SHICommunicator.h"
#include "SHISensor.h"

void SHI::Hardware::logInfo(const char *name, const char *func,
                            std::string message) {
  log((std::string("INFO: ") + name + "." + func + "() " + message).c_str());
}
void SHI::Hardware::logWarn(const char *name, const char *func,
                            std::string message) {
  log((std::string("WARN: ") + name + "." + func + "() " + message).c_str());
}
void SHI::Hardware::logError(const char *name, const char *func,
                             std::string message) {
  log((std::string("ERROR: ") + name + "." + func + "() " + message).c_str());
}

void SHI::Hardware::addSensorGroup(
    std::shared_ptr<SHI::SensorGroup> sensorGroup) {
  sensorGroup->setParent(this);
  sensors.push_back(sensorGroup);
}

void SHI::Hardware::addSensor(std::shared_ptr<SHI::Sensor> sensor) {
  sensors[0]->addSensor(sensor);
}

void SHI::Hardware::addCommunicator(
    std::shared_ptr<SHI::Communicator> communicator) {
  communicator->setParent(this);
  communicators.push_back(communicator);
}

void SHI::Hardware::setupSensors() {
  for (auto &&sensorGroup : sensors) {
    for (auto &&sensor : *sensorGroup->getSensors()) {
      auto sensorName = sensor->getQualifiedName();
      SHI_LOGINFO("Setting up: " + sensorName);
      if (!sensor->setupSensor()) {
        SHI_LOGINFO("Something went wrong when setting up sensor:" +
                    sensorName + " " + sensor->getStatusMessage());
        while (1) {
          errLeds();
        }
      }
      feedWatchdog();
      SHI_LOGINFO("Setup done of: " + sensorName);
    }
  }
}

void SHI::Hardware::setupCommunicators(const char *hwStatus) {
  for (auto &&comm : communicators) {
    comm->setupCommunication();
    comm->newHardwareStatus(hwStatus);
  }
}

void SHI::Hardware::internalLoop() {
  bool sensorHasFatalError = false;
  for (auto &&sensorGroup : sensors) {
    for (auto &&sensor : *sensorGroup->getSensors()) {
      auto sensorName = sensor->getQualifiedName();
      logInfo(name, __func__, std::string("Reading sensor:") + sensorName);
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
                   std::string("Sensor ") + sensorName + " reported error " +
                       status);
        } else {
          logWarn(name, __func__,
                  std::string("Sensor ") + sensorName + " reported warning " +
                      status);
        }
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

void SHI::Hardware::accept(SHI::Visitor &visitor) {
  visitor.enterVisit(this);
  for (auto &&comm : communicators) {
    comm->accept(visitor);
  }
  for (auto &&sensor : sensors) {
    sensor->accept(visitor);
  }
  visitor.leaveVisit(this);
}
