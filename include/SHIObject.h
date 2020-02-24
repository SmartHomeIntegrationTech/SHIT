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

class SHIObject {
 public:
  explicit SHIObject(const char *name) : name(name) {}
  virtual const char *getName() const { return name; }
  virtual void accept(SHI::Visitor &visitor) = 0;  // NOLINT (a warning about
                                                   // the non-const reference)
  virtual std::vector<std::pair<const char *, const char *>> getStatistics() {
    return {};
  }
  virtual void setParent(SHI::SHIObject *newParent) { parent = newParent; }
  virtual SHI::SHIObject *getParent() { return parent; }
  std::string getQualifiedName(const char *seperator = ".");

 protected:
  SHI::SHIObject *parent = nullptr;
  const char *name;
};

}  // namespace SHI
