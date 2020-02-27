/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include "SHIHardware.h"

#include <string.h>

#include "SHICommunicator.h"
#include "SHISensor.h"

namespace {
class StatusVisitor : public SHI::Visitor {
 public:
  bool hasFatalError = false;
  void enterVisit(SHI::Sensor *sensor) { publishStatus(sensor); }
  void enterVisit(SHI::Hardware *hardware) { publishStatus(hardware); }
  void visit(SHI::Communicator *communicator) { publishStatus(communicator); }
  void publishStatus(SHI::SHIObject *obj) {
    auto status = obj->getStatus();
    if (status.getDataState() != SHI::MeasurementDataState::NO_DATA) {
      SHI::hw->publishStatus(status, obj);
      auto statusMsg = status.stringRepresentation;
      if (statusMsg != SHI::STATUS_OK) {
        auto isFatal =
            status.getDataState() == SHI::MeasurementDataState::ERROR;
        if (isFatal) {
          hasFatalError = true;
          SHI::hw->logError("StatusVisitor", __func__,
                            std::string("Object ") + obj->getName() +
                                " reported error " + statusMsg);
        } else {
          SHI::hw->logWarn("StatusVisitor", __func__,
                           std::string("Object ") + obj->getName() +
                               " reported warning " + statusMsg);
        }
      }
    }
  }
};
}  // namespace

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
  defaultGroup->setParent(this);
  for (auto &&sensorGroup : sensors) {
    for (auto &&sensor : *sensorGroup->getSensors()) {
      auto sensorName = sensor->getQualifiedName();
      SHI_LOGINFO("Setting up: " + sensorName);
      if (!sensor->setupSensor()) {
        SHI_LOGINFO(
            "Something went wrong when setting up sensor:" + sensorName + " " +
            sensor->getStatus().stringRepresentation);
        while (1) {
          errLeds();
        }
      }
      feedWatchdog();
      SHI_LOGINFO("Setup done of: " + sensorName);
    }
  }
}

void SHI::Hardware::setupCommunicators() {
  auto status = getStatus();
  for (auto &&comm : communicators) {
    comm->setupCommunication();
    comm->newStatus(status, this);
  }
}

void SHI::Hardware::internalLoop() {
  static int64_t lastStatusTime = 0;
  for (auto &&sensorGroup : sensors) {
    for (auto &&sensor : *sensorGroup->getSensors()) {
      auto sensorName = sensor->getQualifiedName();
      logInfo(name, __func__, std::string("Reading sensor:") + sensorName);
      auto reading = sensor->readSensor();
      for (auto &&mb : reading) {
        for (auto &&comm : communicators) {
          comm->newReading(mb);
        }
      }
    }
  }
  bool hasFatalError = false;
  if (getEpochInMs() - lastStatusTime > 60000) {
    logInfo(name, __func__, "Updating status of all");
    lastStatusTime = getEpochInMs();
    StatusVisitor visitor;
    SHI::hw->accept(visitor);
    hasFatalError = visitor.hasFatalError;
  }

  for (auto &&comm : communicators) {
    comm->loopCommunication();
  }
  while (hasFatalError) {
    errLeds();
  }
}

void SHI::Hardware::publishStatus(const SHI::Measurement &status,
                                  SHI::SHIObject *src) {
  for (auto &&comm : communicators) {
    comm->newStatus(status, src);
  }
}

void SHI::Hardware::accept(SHI::Visitor &visitor) {
  visitor.enterVisit(this);
  status->accept(visitor);
  for (auto &&comm : communicators) {
    comm->accept(visitor);
  }
  for (auto &&sensor : sensors) {
    sensor->accept(visitor);
  }
  visitor.leaveVisit(this);
}
