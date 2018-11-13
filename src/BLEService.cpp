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

#include "local/BLELocalService.h"

#include "BLEService.h"

BLEService::BLEService() :
  BLEService((BLELocalService*)NULL)
{
}

BLEService::BLEService(BLELocalService* local) :
  _local(local)
{
  if (_local) {
    _local->retain();
  }
}

BLEService::BLEService(const char* uuid) :
  BLEService(new BLELocalService(uuid))
{
}

BLEService::~BLEService()
{
  if (_local && _local->release() <= 0) {
    delete _local;
  }
}

const char* BLEService::uuid() const
{
  if (_local) {
    return _local->uuid();
  }

  return "";
}

void BLEService::addCharacteristic(BLECharacteristic& characteristic)
{
  if (_local) {
    _local->addCharacteristic(characteristic);
  }
}

BLELocalService* BLEService::local()
{
  return _local;
}
