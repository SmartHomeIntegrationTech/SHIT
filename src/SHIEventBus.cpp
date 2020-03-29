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

SubscriberBuilder::SubscriberBuilder(bool everything) {
  if (everything) {
    sourceMask = Subscriber::ALL_SOURCES;
    eventMask = Subscriber::ALL_EVENTS;
    dataTypeMask = Subscriber::ALL_DATA;
    customFieldsMask = Subscriber::ALL_CUSTOM_FIELDS;
    hashedNameMask = Subscriber::ALL_HASHES;
  }
}

SubscriberBuilder SubscriberBuilder::empty() { return SubscriberBuilder(); }

SubscriberBuilder SubscriberBuilder::everything() {
  return SubscriberBuilder(true);
}

SubscriberBuilder SubscriberBuilder::forEvent(const Event &event) {
  auto builder = empty();
  builder = builder.addSource(event.sourceType);
  builder = builder.addEvent(event.eventType);
  builder = builder.setExactCustomField(event.customFields);
  builder = builder.setHashedName(event.hashedName);
  return builder;
}

SubscriberBuilder SubscriberBuilder::allSources() {
  auto _sourceMask = Subscriber::ALL_SOURCES;
  return SubscriberBuilder(_sourceMask, eventMask, dataTypeMask,
                           customFieldsMask, hashedNameMask);
}

SubscriberBuilder SubscriberBuilder::setSource(SourceType source) {
  auto _sourceMask = 1 << static_cast<uint8_t>(source);
  return SubscriberBuilder(_sourceMask, eventMask, dataTypeMask,
                           customFieldsMask, hashedNameMask);
}

SubscriberBuilder SubscriberBuilder::addSource(SourceType source) {
  auto _sourceMask = sourceMask | 1 << static_cast<uint8_t>(source);
  return SubscriberBuilder(_sourceMask, eventMask, dataTypeMask,
                           customFieldsMask, hashedNameMask);
}

SubscriberBuilder SubscriberBuilder::excludeSource(SourceType source) {
  auto _sourceMask = sourceMask & ~(1 << static_cast<uint8_t>(source));
  return SubscriberBuilder(_sourceMask, eventMask, dataTypeMask,
                           customFieldsMask, hashedNameMask);
}

SubscriberBuilder SubscriberBuilder::allEvents() {
  auto _eventMask = Subscriber::ALL_EVENTS;
  return SubscriberBuilder(sourceMask, _eventMask, dataTypeMask,
                           customFieldsMask, hashedNameMask);
}

SubscriberBuilder SubscriberBuilder::setEvent(EventType event) {
  auto _eventMask = 1 << static_cast<uint8_t>(event);
  return SubscriberBuilder(sourceMask, _eventMask, dataTypeMask,
                           customFieldsMask, hashedNameMask);
}

SubscriberBuilder SubscriberBuilder::addEvent(EventType event) {
  auto _eventMask = eventMask | 1 << static_cast<uint8_t>(event);
  return SubscriberBuilder(sourceMask, _eventMask, dataTypeMask,
                           customFieldsMask, hashedNameMask);
}

SubscriberBuilder SubscriberBuilder::excludeEvent(EventType event) {
  auto _eventMask = eventMask & ~(1 << static_cast<uint8_t>(event));
  return SubscriberBuilder(sourceMask, _eventMask, dataTypeMask,
                           customFieldsMask, hashedNameMask);
}

SubscriberBuilder SubscriberBuilder::allDataTypes() {
  auto _dataTypeMask = Subscriber::ALL_DATA;
  return SubscriberBuilder(sourceMask, eventMask, _dataTypeMask,
                           customFieldsMask, hashedNameMask);
}

SubscriberBuilder SubscriberBuilder::setDataType(DataType dataType) {
  auto _dataTypeMask = static_cast<uint8_t>(dataType);
  return SubscriberBuilder(sourceMask, eventMask, _dataTypeMask,
                           customFieldsMask, hashedNameMask);
}

SubscriberBuilder SubscriberBuilder::allCustomFields() {
  auto _customFieldsMask = Subscriber::ALL_CUSTOM_FIELDS;
  return SubscriberBuilder(sourceMask, eventMask, dataTypeMask,
                           _customFieldsMask, hashedNameMask);
}

SubscriberBuilder SubscriberBuilder::setCustomFieldMask(uint8_t mask) {
  int masks = 0;
  auto _customFieldsMask = masks | (mask << 8);
  return SubscriberBuilder(sourceMask, eventMask, dataTypeMask,
                           _customFieldsMask, hashedNameMask);
}

SubscriberBuilder SubscriberBuilder::addCustomFieldMask(uint8_t mask) {
  int masks = customFieldsMask & 0xFF00;
  auto _customFieldsMask = masks | (mask << 8);
  return SubscriberBuilder(sourceMask, eventMask, dataTypeMask,
                           _customFieldsMask, hashedNameMask);
}

SubscriberBuilder SubscriberBuilder::excludeCustomFieldMask(uint8_t mask) {
  int masks = customFieldsMask & 0xFF00;
  auto _customFieldsMask = masks & ~(mask << 8);
  return SubscriberBuilder(sourceMask, eventMask, dataTypeMask,
                           _customFieldsMask, hashedNameMask);
}

SubscriberBuilder SubscriberBuilder::setExactCustomField(uint8_t exact) {
  auto _customFieldsMask = exact;
  return SubscriberBuilder(sourceMask, eventMask, dataTypeMask,
                           _customFieldsMask, hashedNameMask);
}

SubscriberBuilder SubscriberBuilder::allHashedNames() {
  auto _hashedNameMask = Subscriber::ALL_HASHES;
  return SubscriberBuilder(sourceMask, eventMask, dataTypeMask,
                           customFieldsMask, _hashedNameMask);
}

SubscriberBuilder SubscriberBuilder::setHashedName(uint32_t hash) {
  auto _hashedNameMask = hash;
  return SubscriberBuilder(sourceMask, eventMask, dataTypeMask,
                           customFieldsMask, _hashedNameMask);
}

std::shared_ptr<Subscriber> SubscriberBuilder::build() {
  if (sourceMask == 0) return std::shared_ptr<Subscriber>(nullptr);
  if (eventMask == 0) return std::shared_ptr<Subscriber>(nullptr);
  return std::make_shared<Subscriber>(sourceMask, eventMask, dataTypeMask,
                                      customFieldsMask, hashedNameMask);
}

bool Subscriber::matches(const Event &event) {
  // std::cout << event << "\n" << *this << "\n\n";
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
  if ((hashedNameMask != 0) && (event.hashedName != hashedNameMask))
    return false;
  return true;
}

EventBuilder EventBuilder::source(SourceType source) {
  EventBuilder result;
  result._source = source;
  return result;
}
EventBuilder EventBuilder::event(EventType event) {
  auto _event = event;
  return EventBuilder(_source, _event, _dataType, _field, _hash);
}
EventBuilder EventBuilder::data(DataType data) {
  auto _dataType = data;
  return EventBuilder(_source, _event, _dataType, _field, _hash);
}
EventBuilder EventBuilder::customField(uint8_t field) {
  auto _field = field;
  return EventBuilder(_source, _event, _dataType, _field, _hash);
}
EventBuilder EventBuilder::hash(std::string name) {
  uint32_t hashValue = static_cast<uint32_t>(hasher(name));
  // We use hash 0 for matching, hopefully this will be rare
  if (hashValue == 0) hashValue = 1;
  return hash(hashValue);
}
EventBuilder EventBuilder::hash(const char *name) {
  return hash(std::string(name));
}
EventBuilder EventBuilder::hash(uint32_t hash) {
  auto _hash = hash;
  return EventBuilder(_source, _event, _dataType, _field, _hash);
}
std::shared_ptr<Event> EventBuilder::build(std::shared_ptr<void> data) {
  if (_source == SourceType::UNDEFINED) return std::shared_ptr<Event>(nullptr);
  if (_event == EventType::UNDEFINED) return std::shared_ptr<Event>(nullptr);
  if (_dataType == DataType::UNDEFINED) return std::shared_ptr<Event>(nullptr);
  if (_hash == 0) return std::shared_ptr<Event>(nullptr);
  auto ptr =
      std::make_shared<Event>(_source, _event, _dataType, _field, _hash, data);
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
void Bus::publish(const std::shared_ptr<const Event> &event) {
  int eventType = static_cast<int>(event->eventType);
  auto resolvedEvent = *event;
  std::cout << "Publish event " << resolvedEvent << "\n";
  auto it = subscribers[eventType].begin();
  while (it != subscribers[eventType].end()) {
    auto sub = *it;
    auto tryResolve = sub.lock();
    if (tryResolve) {
      if (tryResolve->matches(resolvedEvent)) tryResolve->inbox.push(event);
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
