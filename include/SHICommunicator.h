/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#pragma once
#include "SHIObject.h"
#include "SHISensor.h"

namespace SHI {

class Communicator : public SHI::SHIObject {
 public:
  virtual void networkConnected() { isConnected = true; }
  virtual void networkDisconnected() { isConnected = false; }
  virtual void setupCommunication() = 0;
  virtual void loopCommunication() = 0;
  virtual void newReading(const SHI::MeasurementBundle &reading,
                          const SHI::Sensor &sensor) {}
  virtual void newStatus(const SHI::Sensor &sensor, const char *message,
                         bool isFatal) {}
  virtual void newHardwareStatus(const char *message) {}

 protected:
  explicit Communicator(const char *name) : SHIObject(name) {}
  bool isConnected = false;
};

}  // namespace SHI
