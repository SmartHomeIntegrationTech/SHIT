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

#define SHI_LOGINFO(message) ::SHI::hw->logInfo(name, __func__, message)
#define SHI_LOGWARN(message) ::SHI::hw->logWarn(name, __func__, message)
#define SHI_LOGERROR(message) ::SHI::hw->logError(name, __func__, message)

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

  void addSensorGroup(std::shared_ptr<SHI::SensorGroup> sensor);
  void addSensor(std::shared_ptr<SHI::Sensor> sensor);
  void addCommunicator(std::shared_ptr<SHI::Communicator> communicator);

  virtual void setup(const char *defaultName) = 0;
  virtual void loop() = 0;

  virtual void logInfo(const char *name, const char *func, std::string message);
  virtual void logWarn(const char *name, const char *func, std::string message);
  virtual void logError(const char *name, const char *func,
                        std::string message);

  void accept(SHI::Visitor &visitor) override;

  virtual int64_t getEpochInMs() = 0;

 protected:
  std::vector<std::shared_ptr<SHI::SensorGroup>> sensors = {
      std::make_shared<SHI::SensorGroup>("default")};
  std::vector<std::shared_ptr<SHI::Communicator>> communicators = {};

  explicit Hardware(const char *name) : SHIObject(name) {}
  virtual void log(const char *message) = 0;

  void internalLoop();
  void setupSensors();
  void setupCommunicators(const char *hwStatus);
};

extern SHI::Hardware *hw;

}  // namespace SHI
