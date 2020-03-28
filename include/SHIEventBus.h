/**
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace SHI {
namespace EventBus {
enum class SourceType : uint8_t {
  /// Source is SHI::Sensor
  SENSOR,
  /// Source is SHI::Actuator
  ACTUATOR,
  /// Source is SHI::Communicator
  COMMUNICATOR,
  /// Source is SHI::Filter
  FILTER,
  /// Source is a SHI::Hardware
  HARDWARE,
  /// Source is not a SHI::Object
  OTHER,
  UNDEFINED = 255
};

enum class EventType : uint8_t {
  /// Type is SHI::Measurement with type SHI::SensorDataType::Status
  STATUS_UPDATE,
  /// Type is a SHI::MeasurementBundle as received from the sensor
  MEASUREMENT,
  /// Type is a Request to perform some action
  REQUEST,
  /// This is a response to a request
  REQUEST_RESPONSE,
  /// This is a log entry
  LOGGING,
  /// Any other kind of data
  DATA,
  /// Other events
  EVENT,
  UNDEFINED = 255
};

enum class DataType : uint8_t {
  INT = 1,
  FLOAT,
  STRING,
  MEASUREMENT,
  MEASUREMENT_BUNDLE,
  CONFIGURATION,
  OTHER_DATA = 127,
  /// Vector can be OR'ed onto the dataType to indicate a vector of it
  VECTOR = 128,
  UNDEFINED = 255
};

struct Event {
  Event(SourceType sourceType, EventType eventType, DataType dataType,
        uint8_t customFields, uint32_t hashedName,
        const std::shared_ptr<void> &data)
      : sourceType(sourceType),
        eventType(eventType),
        dataType(dataType),
        customFields(customFields),
        hashedName(hashedName),
        data(data) {}
  SourceType sourceType = SourceType::UNDEFINED;
  EventType eventType = EventType::UNDEFINED;
  DataType dataType = DataType::UNDEFINED;
  uint8_t customFields = 0;
  uint32_t hashedName = 0;
  std::shared_ptr<void> data;
  operator std::string() const;
};

/*
template <typename T>
class EventAccesor {
 public:
  /// This accessor can be used when this subscriber is matching. The callback
  /// is ignored
  explicit EventAccesor(Subscriber subscriber) : subscriber(subscriber) {}
  T *getData(const Event &event) {
    if (subscriber.matches(event)) {
      reinterpret_cast<T *>(event.data);
    }
    return nullptr;
  }

 private:
  Subscriber subscriber;
}; */

class EventBuilder {
 public:
  static EventBuilder source(SourceType source);
  EventBuilder *event(EventType event);
  EventBuilder *data(DataType data);

  EventBuilder *customField(uint8_t field);
  EventBuilder *hash(std::string name);
  EventBuilder *hash(const char *name);
  EventBuilder *hash(uint32_t hash);
  std::unique_ptr<Event> build(std::shared_ptr<void> data);

 private:
  static std::hash<std::string> hasher;
  SourceType _source = SourceType::UNDEFINED;
  EventType _event = EventType::UNDEFINED;
  DataType _dataType = DataType::UNDEFINED;
  uint8_t _field = 0;
  uint32_t _hash = 0;
};

using SubscriberCallBack = std::function<void(const Event &)>;

struct Subscriber {
  static const int ALL_SOURCES = 0xFF;
  static const int ALL_EVENTS = 0xFF;
  static const int ALL_DATA = 0;
  static const int ALL_CUSTOM_FIELDS = 0xFF00;
  static const int ALL_HASHES = 0;

  uint8_t sourceMask = 0;
  uint8_t eventMask = 0;
  uint8_t dataTypeMask = 0;
  uint16_t customFieldsMask = 0;
  uint32_t hashedNameMask = 0;
  SubscriberCallBack callBack;
  Subscriber(uint8_t sourceMask, uint8_t eventMask, uint8_t dataTypeMask,
             uint16_t customFieldsMask, uint32_t hashedNameMask,
             SubscriberCallBack callBack)
      : sourceMask(sourceMask),
        eventMask(eventMask),
        dataTypeMask(dataTypeMask),
        customFieldsMask(customFieldsMask),
        hashedNameMask(hashedNameMask),
        callBack(callBack) {}
  explicit Subscriber(SubscriberCallBack callBack) : callBack(callBack) {}
  Subscriber withCallBack(SubscriberCallBack newCallBack);
  bool matches(const Event &event);
  operator std::string() const;
};

class SubscriberBuilder {
 public:
  static SubscriberBuilder empty(SubscriberCallBack callback);
  static SubscriberBuilder everything(SubscriberCallBack callback);

  SubscriberBuilder *allSources();
  SubscriberBuilder *addSource(SourceType source);
  SubscriberBuilder *excludeSource(SourceType source);

  SubscriberBuilder *allEvents();
  SubscriberBuilder *addEvent(EventType source);
  SubscriberBuilder *excludeEvent(EventType source);

  SubscriberBuilder *allDataTypes();
  SubscriberBuilder *setDataType(DataType dataType);

  SubscriberBuilder *allCustomFields();
  SubscriberBuilder *addCustomFieldMask(uint8_t mask);
  SubscriberBuilder *excludeCustomFieldMask(uint8_t mask);
  SubscriberBuilder *setExactCustomField(uint8_t exact);

  SubscriberBuilder *allHashedNames();
  SubscriberBuilder *setHashedName(uint32_t hash);

  std::shared_ptr<Subscriber> build();

 private:
  explicit SubscriberBuilder(SubscriberCallBack callback,
                             bool everything = false);
  uint8_t sourceMask = 0;
  uint8_t eventMask = 0;
  uint8_t dataTypeMask = 0;
  uint16_t customFieldsMask = 0;
  uint32_t hashedNameMask = 0;
  SubscriberCallBack callBack;
};

class Bus {
 public:
  static Bus *get();
  static void reset() {
    delete instance;
    instance = nullptr;
  }
  void publish(const Event &event);
  void subscribe(const std::shared_ptr<SHI::EventBus::Subscriber> &subscriber);

 private:
  static Bus *instance;
  Bus() {}
  std::vector<std::weak_ptr<Subscriber>> subscribers[8];
};

}  // namespace EventBus
}  // namespace SHI

std::ostream &operator<<(std::ostream &os, SHI::EventBus::Event const &s);
std::ostream &operator<<(std::ostream &os, SHI::EventBus::Subscriber const &s);
