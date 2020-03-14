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
class Configuration;

extern const std::string STATUS_OK;

class SHIObject {
 public:
  explicit SHIObject(const std::string &name, bool initStatus = true);
  SHIObject(const SHIObject &) = delete;
  SHIObject(SHIObject &&) = delete;
  SHIObject &operator=(const SHIObject &) = delete;
  SHIObject &operator=(SHIObject &&) = delete;
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
  template <typename T>
  constexpr const T getConfigAs() const {
    static_assert(std::is_base_of<Configuration, T>::value,
                  "Type needs to derive of Config");
    return *static_cast<const T *>(getConfig());
  }
  virtual const Configuration *getConfig() const = 0;
  virtual bool reconfigure(Configuration *newConfig) = 0;

 protected:
  template <typename T>
  static constexpr T castConfig(Configuration *config) {
    static_assert(std::is_base_of<Configuration, T>::value,
                  "Type needs to derive of Config");
    return *static_cast<T *>(config);
  }
  SHIObject *parent = nullptr;
  std::string name;
  std::string statusMessage = STATUS_OK;
  bool fatalError = false;
  std::shared_ptr<MeasurementMetaData> status;
};

}  // namespace SHI
