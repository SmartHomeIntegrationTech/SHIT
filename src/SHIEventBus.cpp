/**
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "SHIEventBus.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

using SHI::EventBus::Bus;

using SHI::EventBus::Event;
using SHI::EventBus::EventBuilder;

using SHI::EventBus::Subscriber;
using SHI::EventBus::SubscriberBuilder;

SubscriberBuilder::SubscriberBuilder(SubscriberCallBack callBack,
                                     bool everything)
    : callBack(callBack) {
  if (everything) {
    sourceMask = Subscriber::ALL_SOURCES;
    eventMask = Subscriber::ALL_EVENTS;
    dataTypeMask = Subscriber::ALL_DATA;
    customFieldsMask = Subscriber::ALL_CUSTOM_FIELDS;
    hashedNameMask = Subscriber::ALL_HASHES;
  }
}

SubscriberBuilder SubscriberBuilder::empty(SubscriberCallBack callBack) {
  return SubscriberBuilder(callBack);
}

SubscriberBuilder SubscriberBuilder::everything(SubscriberCallBack callBack) {
  return SubscriberBuilder(callBack, true);
}

SubscriberBuilder *SubscriberBuilder::addSource(SourceType source) {
  sourceMask |= 1 << static_cast<uint8_t>(source);
  return this;
}

SubscriberBuilder *SubscriberBuilder::excludeSource(SourceType source) {
  sourceMask &= ~(1 << static_cast<uint8_t>(source));
  return this;
}

SubscriberBuilder *SubscriberBuilder::allEvents() {
  eventMask = Subscriber::ALL_EVENTS;
  return this;
}

SubscriberBuilder *SubscriberBuilder::addEvent(EventType event) {
  eventMask |= 1 << static_cast<uint8_t>(event);
  return this;
}

SubscriberBuilder *SubscriberBuilder::excludeEvent(EventType event) {
  eventMask &= ~(1 << static_cast<uint8_t>(event));
  return this;
}

SubscriberBuilder *SubscriberBuilder::allDataTypes() {
  dataTypeMask = Subscriber::ALL_DATA;
  return this;
}

SubscriberBuilder *SubscriberBuilder::setDataType(DataType dataType) {
  dataTypeMask = static_cast<uint8_t>(dataType);
  return this;
}

SubscriberBuilder *SubscriberBuilder::allCustomFields() {
  customFieldsMask = Subscriber::ALL_CUSTOM_FIELDS;
  return this;
}

SubscriberBuilder *SubscriberBuilder::addCustomFieldMask(uint8_t mask) {
  int masks = customFieldsMask & 0xFF00;
  customFieldsMask = masks | (mask << 8);
  return this;
}

SubscriberBuilder *SubscriberBuilder::excludeCustomFieldMask(uint8_t mask) {
  int masks = customFieldsMask & 0xFF00;
  customFieldsMask = masks & ~(mask << 8);
  return this;
}

SubscriberBuilder *SubscriberBuilder::setExactCustomField(uint8_t exact) {
  customFieldsMask = exact;
  return this;
}

SubscriberBuilder *SubscriberBuilder::allHashedNames() {
  hashedNameMask = Subscriber::ALL_HASHES;
  return this;
}

SubscriberBuilder *SubscriberBuilder::setHashedName(uint32_t hash) {
  hashedNameMask = hash;
  return this;
}

std::shared_ptr<Subscriber> SubscriberBuilder::build() {
  if (sourceMask == 0) return std::shared_ptr<Subscriber>(nullptr);
  if (eventMask == 0) return std::shared_ptr<Subscriber>(nullptr);
  return std::make_shared<Subscriber>(sourceMask, eventMask, dataTypeMask,
                                      customFieldsMask, hashedNameMask,
                                      callBack);
}

Subscriber Subscriber::withCallBack(SubscriberCallBack newCallBack) {
  return Subscriber(sourceMask, eventMask, dataTypeMask, customFieldsMask,
                    hashedNameMask, newCallBack);
}

bool Subscriber::matches(const Event &event) {
  if (((1 << static_cast<uint8_t>(event.sourceType)) & sourceMask) == 0)
    return false;
  if (((1 << static_cast<uint8_t>(event.eventType)) & eventMask) == 0)
    return false;
  if ((static_cast<uint8_t>(event.dataType) | dataTypeMask) !=
      static_cast<uint8_t>(event.dataType))
    return false;
  if (customFieldsMask < 256) {
    if (event.customFields != customFieldsMask) return false;
  } else {
    if ((event.customFields & (customFieldsMask >> 8)) == 0) return false;
  }
  std::cout << "Field matched\n";
  if ((hashedNameMask != 0) && (event.hashedName != hashedNameMask))
    return false;
  std::cout << "Hash matched, all matched\n";
  return true;
}

EventBuilder EventBuilder::source(SourceType source) {
  EventBuilder result;
  result._source = source;
  return result;
}
EventBuilder *EventBuilder::event(EventType event) {
  _event = event;
  return this;
}
EventBuilder *EventBuilder::data(DataType data) {
  _dataType = data;
  return this;
}
EventBuilder *EventBuilder::customField(uint8_t field) {
  _field = field;
  return this;
}
EventBuilder *EventBuilder::hash(std::string name) {
  uint32_t hashValue = hasher(name);
  // We use hash 0 for matching, hopefully this will be rare
  if (hashValue == 0) hashValue = 1;
  return hash(hashValue);
}
EventBuilder *EventBuilder::hash(const char *name) {
  return hash(std::string(name));
}
EventBuilder *EventBuilder::hash(uint32_t hash) {
  _hash = hash;
  return this;
}
std::unique_ptr<Event> EventBuilder::build(std::shared_ptr<void> data) {
  if (_source == SourceType::UNDEFINED) return std::unique_ptr<Event>(nullptr);
  if (_event == EventType::UNDEFINED) return std::unique_ptr<Event>(nullptr);
  if (_dataType == DataType::UNDEFINED) return std::unique_ptr<Event>(nullptr);
  if (_hash == 0) return std::unique_ptr<Event>(nullptr);
  auto ptr = std::unique_ptr<Event>(
      new Event(_source, _event, _dataType, _field, _hash, data));
  return ptr;
}

std::hash<std::string> EventBuilder::hasher;

Bus *Bus::instance;

Bus *Bus::get() {
  if (instance == nullptr) {
    instance = new Bus();
  }
  return instance;
}
void Bus::publish(const Event &event) {
  int eventType = static_cast<int>(event.eventType);
  std::cout << "Publish event " << event << "\n";
  auto it = subscribers[eventType].begin();
  while (it != subscribers[eventType].end()) {
    auto sub = *it;
    std::cout << "Checking subscriber\n";
    auto tryResolve = sub.lock();
    if (tryResolve) {
      std::cout << "Subscriber resolved " << *tryResolve << "\n";
      if (tryResolve->matches(event)) tryResolve->callBack(event);
      ++it;
    } else {
      it = subscribers[eventType].erase(it);
    }
  }
}
void Bus::subscribe(const std::shared_ptr<Subscriber> &subscriber) {
  int mask = subscriber->eventMask;
  std::cout << *subscriber << std::endl;
  for (int i = 0; i < 8; i++) {
    if (((1 << i) & mask) != 0) {
      std::cout << "Adding to " << i << "\n";
      subscribers[i].push_back(std::weak_ptr<Subscriber>(subscriber));
    }
  }
}

Event::operator std::string() const {
  std::stringstream ss;
  ss << "Event [sourceType:" << static_cast<int>(sourceType)
     << " eventType:" << static_cast<int>(eventType)
     << " dataType:" << static_cast<int>(dataType)
     << " field:" << static_cast<int>(customFields)
     << " hash:" << static_cast<int>(hashedName) << "]";
  return ss.str();
}

Subscriber::operator std::string() const {
  std::stringstream ss;
  ss << "Subscriber [sourceMask:" << static_cast<int>(sourceMask)
     << " eventMask:" << static_cast<int>(eventMask)
     << " dataMask:" << static_cast<int>(dataTypeMask)
     << " fieldMask:" << static_cast<int>(customFieldsMask)
     << " hashMask:" << static_cast<int>(hashedNameMask) << "]";
  return ss.str();
}

std::ostream &operator<<(std::ostream &os, Event const &e) {
  std::string es = e;
  return os << es;
}

std::ostream &operator<<(std::ostream &os, Subscriber const &s) {
  std::string es = s;
  return os << es;
}
