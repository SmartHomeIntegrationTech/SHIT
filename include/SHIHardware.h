/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#pragma once
#include <memory>
#include <string>
#include <vector>

#include "SHIObject.h"

namespace SHI {

extern const uint8_t MAJOR_VERSION;
extern const uint8_t MINOR_VERSION;
extern const uint8_t PATCH_VERSION;
extern const char *VERSION;

class Hardware : public SHI::SHIObject {
 public:
  virtual void resetWithReason(const char *reason, bool restart) = 0;
  virtual void errLeds(void) = 0;

  virtual void setupWatchdog() = 0;
  virtual void feedWatchdog() = 0;
  virtual void disableWatchdog() = 0;

  virtual const char *getNodeName() = 0;
  virtual const char *getResetReason() = 0;
  virtual void resetConfig() = 0;
  virtual void printConfig() = 0;

  void addSensor(std::shared_ptr<SHI::Sensor> sensor) {
    sensors.push_back(sensor);
  }
  void addCommunicator(std::shared_ptr<SHI::Communicator> communicator) {
    communicators.push_back(communicator);
  }

  virtual void setup(const char *defaultName) = 0;
  virtual void loop() = 0;

  void logInfo(const char *name, const char *func, std::string message) {
    log((std::string("INFO: ") + name + "." + func + "() " + message).c_str());
  }
  void logWarn(const char *name, const char *func, std::string message) {
    log((std::string("WARN: ") + name + "." + func + "() " + message).c_str());
  }
  void logError(const char *name, const char *func, std::string message) {
    log((std::string("ERROR: ") + name + "." + func + "() " + message).c_str());
  }

 protected:
  std::vector<std::shared_ptr<SHI::Sensor>> sensors = {};
  std::vector<std::shared_ptr<SHI::Communicator>> communicators = {};

  explicit Hardware(const char *name) : SHIObject(name) {}
  virtual void log(const char *message) = 0;

  void internalLoop();
};

extern SHI::Hardware *hw;

}  // namespace SHI
