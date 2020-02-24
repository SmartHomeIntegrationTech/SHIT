/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include "SHIObject.h"

#include <string>

std::string SHI::SHIObject::getQualifiedName(const char *seperator) {
  if (parent != nullptr) {
    return std::string(parent->getQualifiedName(seperator)) + seperator +
           getName();
  }
  return std::string(getName());
}
