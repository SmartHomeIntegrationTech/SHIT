/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "SHIVisitor.h"

namespace SHI {

class Measurement;
class MeasurementMetaData;
extern const std::string STATUS_OK;

class SHIObject {
 public:
  explicit SHIObject(const std::string &name, bool initStatus = true);
  virtual ~SHIObject() = default;
  virtual const std::string getName() const { return name; }
  virtual void accept(Visitor &visitor) = 0;  // NOLINT (a warning about
                                              // the non-const reference)
  virtual std::vector<std::pair<std::string, std::string>> getStatistics() {
    return {};
  }
  virtual void setParent(SHIObject *newParent) { parent = newParent; }
  virtual SHIObject *getParent() const { return parent; }
  virtual std::string getQualifiedName(
      const std::string &seperator = ".") const;
  virtual Measurement getStatus();

 protected:
  SHIObject *parent = nullptr;
  const std::string name;
  std::string statusMessage = STATUS_OK;
  bool fatalError = false;
  std::shared_ptr<MeasurementMetaData> status;
};

}  // namespace SHI
