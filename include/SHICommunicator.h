/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#pragma once
#include "SHIObject.h"
#include "SHISensor.h"

namespace SHI {

class Communicator : public SHIObject {
 public:
  virtual void networkConnected() { isConnected = true; }
  virtual void networkDisconnected() { isConnected = false; }
  virtual void setupCommunication() = 0;
  virtual void loopCommunication() = 0;
  virtual void newReading(const MeasurementBundle &reading) {}
  virtual void newStatus(const Measurement &status, SHIObject *src) {}

  void accept(Visitor &visitor) override;

 protected:
  explicit Communicator(const char *name) : SHIObject(name) {}
  bool isConnected = false;
};

}  // namespace SHI
