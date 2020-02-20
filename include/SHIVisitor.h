/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#pragma once

namespace SHI {

class Sensor;
class Channel;
class Hardware;
class Communicator;
class MeasurementMetaData;

class Visitor {
 public:
  virtual void visit(SHI::Sensor *sensor) {}
  virtual void visit(SHI::Channel *channel) {}
  virtual void visit(SHI::Hardware *harwdware) {}
  virtual void visit(SHI::Communicator *communicator) {}
  virtual void visit(SHI::MeasurementMetaData *data) {}
};

}  // namespace SHI
