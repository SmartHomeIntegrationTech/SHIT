/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "SHIVisitor.h"

namespace SHI {

class SHIObject {
 public:
  explicit SHIObject(const char *name) : name(name) {}
  virtual const char *getName() const { return name; }
  virtual void accept(SHI::Visitor &visitor) = 0;
  virtual std::vector<std::pair<const char *, const char *>> getStatistics() {
    return {};
  }

 protected:
  const char *name;
};

}  // namespace SHI
