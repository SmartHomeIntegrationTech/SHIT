/**
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <queue>
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
  /// Life-cycle management
  LIFECYCLE,
  /// Initial value for undefined fields
  UNDEFINED = 255
};

enum class DataType : uint8_t {
  UNDEFINED,
  INT,
  FLOAT,
  STRING,
  MEASUREMENT,
  MEASUREMENT_BUNDLE,
  CONFIGURATION,
  OTHER_DATA = 127,
  /// Vector can be OR'ed onto the dataType to indicate a vector of it
  VECTOR = 128
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
  std::shared_ptr<const void> data;
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
  EventBuilder event(EventType event);
  EventBuilder data(DataType data);

  EventBuilder customField(uint8_t field);
  EventBuilder hash(std::string name);
  EventBuilder hash(const char *name);
  EventBuilder hash(uint32_t hash);
  std::shared_ptr<Event> build(std::shared_ptr<void> data);

 private:
  EventBuilder() {}
  EventBuilder(SourceType source, EventType event, DataType data, uint8_t field,
               uint32_t hash)
      : _source(source),
        _event(event),
        _dataType(data),
        _field(field),
        _hash(hash) {}
  static std::hash<std::string> hasher;
  SourceType _source = SourceType::UNDEFINED;
  EventType _event = EventType::UNDEFINED;
  DataType _dataType = DataType::UNDEFINED;
  uint8_t _field = 0;
  uint32_t _hash = 0;
};

class Subscriber {
 public:
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

  std::queue<std::shared_ptr<const Event>> inbox;
  Subscriber(uint8_t sourceMask, uint8_t eventMask, uint8_t dataTypeMask,
             uint16_t customFieldsMask, uint32_t hashedNameMask)
      : sourceMask(sourceMask),
        eventMask(eventMask),
        dataTypeMask(dataTypeMask),
        customFieldsMask(customFieldsMask),
        hashedNameMask(hashedNameMask) {}
  Subscriber() {}
  bool matches(const Event &event);
  operator std::string() const;
};

class SubscriberBuilder {
 public:
  static SubscriberBuilder empty();
  static SubscriberBuilder everything();
  static SubscriberBuilder forEvent(const Event &event);

  SubscriberBuilder allSources();
  SubscriberBuilder setSource(SourceType source);
  SubscriberBuilder addSource(SourceType source);
  SubscriberBuilder excludeSource(SourceType source);

  SubscriberBuilder allEvents();
  SubscriberBuilder setEvent(EventType source);
  SubscriberBuilder addEvent(EventType source);
  SubscriberBuilder excludeEvent(EventType source);

  SubscriberBuilder allDataTypes();
  SubscriberBuilder setDataType(DataType dataType);

  SubscriberBuilder allCustomFields();
  SubscriberBuilder setCustomFieldMask(uint8_t mask);
  SubscriberBuilder addCustomFieldMask(uint8_t mask);
  SubscriberBuilder excludeCustomFieldMask(uint8_t mask);
  SubscriberBuilder setExactCustomField(uint8_t exact);

  SubscriberBuilder allHashedNames();
  SubscriberBuilder setHashedName(uint32_t hash);

  std::shared_ptr<Subscriber> build();

 private:
  uint8_t sourceMask = 0;
  uint8_t eventMask = 0;
  uint8_t dataTypeMask = 0;
  uint16_t customFieldsMask = 0;
  uint32_t hashedNameMask = 0;

  explicit SubscriberBuilder(bool everything = false);
  SubscriberBuilder(uint8_t sourceMask, uint8_t eventMask, uint8_t dataTypeMask,
                    uint16_t customFieldsMask, uint32_t hashedNameMask)
      : sourceMask(sourceMask),
        eventMask(eventMask),
        dataTypeMask(dataTypeMask),
        customFieldsMask(customFieldsMask),
        hashedNameMask(hashedNameMask) {}
};

class Bus {
 public:
  static Bus *get();
  static void reset() {
    delete instance;
    instance = nullptr;
  }
  void publish(const std::shared_ptr<const Event> &event);
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
