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

class Hardware : public SHIObject {
 public:
  virtual void resetWithReason(const std::string &reason, bool restart) = 0;
  virtual void errLeds(void) = 0;

  virtual void setupWatchdog() = 0;
  virtual void feedWatchdog() = 0;
  virtual void disableWatchdog() = 0;

  virtual std::string getNodeName() = 0;
  virtual std::string getResetReason() = 0;
  virtual void resetConfig() = 0;
  virtual void printConfig() = 0;

  void addSensorGroup(std::shared_ptr<SensorGroup> sensor);
  void addSensor(std::shared_ptr<Sensor> sensor);
  void addCommunicator(std::shared_ptr<Communicator> communicator);

  virtual void setup(const std::string &defaultName) = 0;
  virtual void loop() = 0;

  virtual void logInfo(const std::string &name, const char *func,
                       std::string message);
  virtual void logWarn(const std::string &name, const char *func,
                       std::string message);
  virtual void logError(const std::string &name, const char *func,
                        std::string message);

  void accept(Visitor &visitor) override;

  virtual int64_t getEpochInMs() = 0;

  void publishStatus(const SHI::Measurement &status, SHI::SHIObject *src);

  virtual ~Hardware() = default;

 protected:
  std::shared_ptr<SensorGroup> defaultGroup;
  std::vector<std::shared_ptr<SensorGroup>> sensors;
  std::vector<std::shared_ptr<Communicator>> communicators = {};

  explicit Hardware(const std::string &name);
  virtual void log(const std::string &message) = 0;

  void internalLoop();
  void setupSensors();
  void setupCommunicators();
};

extern Hardware *hw;

}  // namespace SHI
