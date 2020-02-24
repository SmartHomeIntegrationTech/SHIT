/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#pragma once

namespace SHI {

class Sensor;
class SensorGroup;
class Hardware;
class Communicator;
class MeasurementMetaData;

class Visitor {
 public:
  virtual void enterVisit(SHI::Sensor *sensor) {}
  virtual void leaveVisit(SHI::Sensor *sensor) {}
  virtual void enterVisit(SHI::SensorGroup *channel) {}
  virtual void leaveVisit(SHI::SensorGroup *channel) {}
  virtual void enterVisit(SHI::Hardware *harwdware) {}
  virtual void leaveVisit(SHI::Hardware *harwdware) {}
  virtual void visit(SHI::Communicator *communicator) {}
  virtual void visit(SHI::MeasurementMetaData *data) {}
};

}  // namespace SHI
