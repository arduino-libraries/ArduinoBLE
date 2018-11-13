/*
  This file is part of the ArduinoBLE library.
  Copyright (c) 2018 Arduino SA. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "local/BLELocalCharacteristic.h"

#include "BLECharacteristic.h"

BLECharacteristic::BLECharacteristic() :
  BLECharacteristic(NULL)
{
}

BLECharacteristic::BLECharacteristic(BLELocalCharacteristic* local) :
  _local(local)
{
  if (_local) {
    _local->retain();
  }
}

BLECharacteristic::BLECharacteristic(const char* uuid, uint8_t properties, int valueSize, bool fixedLength) :
  BLECharacteristic(new BLELocalCharacteristic(uuid, properties, valueSize, fixedLength))
{
}

BLECharacteristic::BLECharacteristic(const char* uuid, uint8_t properties, const char* value) :
  BLECharacteristic(new BLELocalCharacteristic(uuid, properties, value))
{
}

BLECharacteristic::~BLECharacteristic()
{
  if (_local && _local->release() <= 0) {
    delete _local;
  }
}

uint8_t BLECharacteristic::properties() const
{
  if (_local) {
    return _local->properties();
  }

  return 0;
}

int BLECharacteristic::valueSize() const
{
  if (_local) {
    return _local->valueSize();
  }

  return 0;
}

const uint8_t* BLECharacteristic::value() const
{
  if (_local) {
    return _local->value();
  }

  return NULL;
}

int BLECharacteristic::valueLength() const
{
  if (_local) {
    return _local->valueLength();
  }

  return 0;
}

uint8_t BLECharacteristic::operator[] (int offset) const
{
  if (_local) {
    return (*_local)[offset];
  }

  return 0;
}

int BLECharacteristic::writeValue(const uint8_t value[], int length)
{
  if (_local) {
    return _local->writeValue(value, length);
  }

  return 0;
}

int BLECharacteristic::writeValue(const char* value)
{
  if (_local) {
    return _local->writeValue(value);
  }

  return 0;
}

int BLECharacteristic::broadcast()
{
  if (_local) {
    return _local->broadcast();
  }

  return 0;
}

bool BLECharacteristic::written()
{
  if (_local) {
    return _local->written();
  }

  return false;
}

bool BLECharacteristic::subscribed()
{
  if (_local) {
    return _local->subscribed();
  }

  return false;
}

void BLECharacteristic::addDescriptor(BLEDescriptor& descriptor)
{
  if (_local) {
    return _local->addDescriptor(descriptor);
  }
}

BLECharacteristic::operator bool() const
{
  return (_local != NULL);
}

BLELocalCharacteristic* BLECharacteristic::local()
{
  return _local;
}

void BLECharacteristic::setEventHandler(int event, BLECharacteristicEventHandler eventHandler)
{
  if (_local) {
    _local->setEventHandler((BLECharacteristicEvent)event, eventHandler);
  }
}
