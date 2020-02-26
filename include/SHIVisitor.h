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
  virtual void enterVisit(Sensor *sensor) {}
  virtual void leaveVisit(Sensor *sensor) {}
  virtual void enterVisit(SensorGroup *channel) {}
  virtual void leaveVisit(SensorGroup *channel) {}
  virtual void enterVisit(Hardware *harwdware) {}
  virtual void leaveVisit(Hardware *harwdware) {}
  virtual void visit(Communicator *communicator) {}
  virtual void visit(MeasurementMetaData *data) {}
};

}  // namespace SHI
