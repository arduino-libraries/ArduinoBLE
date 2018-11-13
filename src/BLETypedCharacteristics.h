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

#ifndef _BLE_TYPED_CHARACTERISTICS_H_
#define _BLE_TYPED_CHARACTERISTICS_H_

#include "BLETypedCharacteristic.h"

class BLEBoolCharacteristic : public BLETypedCharacteristic<bool> {
public:
  BLEBoolCharacteristic(const char* uuid, unsigned char properties);
};

class BLEBooleanCharacteristic : public BLETypedCharacteristic<boolean> {
public:
  BLEBooleanCharacteristic(const char* uuid, unsigned char properties);
};

class BLECharCharacteristic : public BLETypedCharacteristic<char> {
public:
  BLECharCharacteristic(const char* uuid, unsigned char properties);
};

class BLEUnsignedCharCharacteristic : public BLETypedCharacteristic<unsigned char> {
public:
  BLEUnsignedCharCharacteristic(const char* uuid, unsigned char properties);
};

class BLEByteCharacteristic : public BLETypedCharacteristic<byte> {
public:
  BLEByteCharacteristic(const char* uuid, unsigned char properties);
};

class BLEShortCharacteristic : public BLETypedCharacteristic<short> {
public:
  BLEShortCharacteristic(const char* uuid, unsigned char properties);
};

class BLEUnsignedShortCharacteristic : public BLETypedCharacteristic<unsigned short> {
public:
  BLEUnsignedShortCharacteristic(const char* uuid, unsigned char properties);
};

class BLEWordCharacteristic : public BLETypedCharacteristic<word> {
public:
  BLEWordCharacteristic(const char* uuid, unsigned char properties);
};

class BLEIntCharacteristic : public BLETypedCharacteristic<int> {
public:
  BLEIntCharacteristic(const char* uuid, unsigned char properties);
};

class BLEUnsignedIntCharacteristic : public BLETypedCharacteristic<unsigned int> {
public:
  BLEUnsignedIntCharacteristic(const char* uuid, unsigned char properties);
};

class BLELongCharacteristic : public BLETypedCharacteristic<long> {
public:
  BLELongCharacteristic(const char* uuid, unsigned char properties);
};

class BLEUnsignedLongCharacteristic : public BLETypedCharacteristic<unsigned long> {
public:
  BLEUnsignedLongCharacteristic(const char* uuid, unsigned char properties);
};

class BLEFloatCharacteristic : public BLETypedCharacteristic<float> {
public:
  BLEFloatCharacteristic(const char* uuid, unsigned char properties);
};

class BLEDoubleCharacteristic : public BLETypedCharacteristic<double> {
public:
  BLEDoubleCharacteristic(const char* uuid, unsigned char properties);
};

#endif
