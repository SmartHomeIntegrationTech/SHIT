/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include "SHIHardware.h"

#include <string.h>

#include "SHICommunicator.h"
#include "SHISensor.h"

using SHI::Communicator;
using SHI::Hardware;
using SHI::MeasurementDataState;
using SHI::Sensor;
using SHI::SHIObject;
using SHI::Visitor;

namespace {
class StatusVisitor : public Visitor {
 public:
  bool hasFatalError = false;
  void enterVisit(Sensor *sensor) { publishStatus(sensor); }
  void enterVisit(Hardware *hardware) { publishStatus(hardware); }
  void visit(Communicator *communicator) { publishStatus(communicator); }
  void publishStatus(SHIObject *obj) {
    auto status = obj->getStatus();
    if (status.getDataState() != MeasurementDataState::NO_DATA) {
      SHI::hw->publishStatus(status, obj);
      auto statusMsg = status.stringRepresentation;
      if (statusMsg != SHI::STATUS_OK) {
        auto isFatal = status.getDataState() == MeasurementDataState::ERROR;
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

Hardware::Hardware(const std::string &name) : SHIObject(name) {
  defaultGroup = std::make_shared<SensorGroup>("default");
  sensors.push_back(defaultGroup);
}

void Hardware::logInfo(const std::string &name, const char *func,
                       std::string message) {
  log((std::string("INFO: ") + name + "." + func + "() " + message).c_str());
}
void Hardware::logWarn(const std::string &name, const char *func,
                       std::string message) {
  log((std::string("WARN: ") + name + "." + func + "() " + message).c_str());
}
void Hardware::logError(const std::string &name, const char *func,
                        std::string message) {
  log((std::string("ERROR: ") + name + "." + func + "() " + message).c_str());
}

void Hardware::addSensorGroup(std::shared_ptr<SensorGroup> sensorGroup) {
  if ("default" == sensorGroup->getName()) {
    for (auto &&sensor : *sensorGroup->getSensors()) {
      defaultGroup->addSensor(sensor);
    }
  } else {
    sensorGroup->setParent(this);
    sensors.push_back(sensorGroup);
  }
}

void Hardware::addSensor(std::shared_ptr<Sensor> sensor) {
  sensors[0]->addSensor(sensor);
}

void Hardware::addCommunicator(std::shared_ptr<Communicator> communicator) {
  communicator->setParent(this);
  communicators.push_back(communicator);
}

void Hardware::setupSensors() {
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

void Hardware::setupCommunicators() {
  auto status = getStatus();
  for (auto &&comm : communicators) {
    comm->setupCommunication();
    comm->newStatus(status, this);
  }
}

void Hardware::internalLoop() {
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
    hw->accept(visitor);
    hasFatalError = visitor.hasFatalError;
  }

  for (auto &&comm : communicators) {
    comm->loopCommunication();
  }
  while (hasFatalError) {
    errLeds();
  }
}

void Hardware::publishStatus(const Measurement &status, SHIObject *src) {
  for (auto &&comm : communicators) {
    comm->newStatus(status, src);
  }
}

void Hardware::accept(Visitor &visitor) {
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
