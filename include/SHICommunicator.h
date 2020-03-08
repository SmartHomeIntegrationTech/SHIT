/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#pragma once
#include <string>

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
  Communicator(const Communicator&) = delete;
  Communicator(Communicator&&) = delete;
  Communicator& operator=(const Communicator&) = delete;
  Communicator& operator=(Communicator&&) = delete;
  virtual ~Communicator() = default;

 protected:
  explicit Communicator(const std::string &name) : SHIObject(name) {}
  bool isConnected = false;
};

}  // namespace SHI
