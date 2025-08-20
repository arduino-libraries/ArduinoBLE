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

#ifndef _BLE_FORMAT_TYPES_H_
#define _BLE_FORMAT_TYPES_H_

const char BleBoolean[] = "\x01"; /* unsigned 1-bit integer, 0-false, 1-true */
const char Ble2bit[] = "\x02"; /* unsigned 2-bit integer */
const char BleNibble[] = "\x03"; /* unsigned 4-bit integer */
const char BleUint8[] = "\x04"; /* unsigned 8-bit integer */
const char BleUint12[] = "\x05"; /* unsigned 12-bit integer */
const char BleUint16[] = "\x06"; /* unsigned 16-bit integer */
const char BleUint24[] = "\x07"; /* unsigned 24-bit integer */
const char BleUint32[] = "\x08"; /* unsigned 32-bit integer */
const char BleUint48[] = "\x09"; /* unsigned 48-bit integer */
const char BleUint64[] = "\x0A"; /* unsigned 64-bit integer */
const char BleUint128[] = "\x0B"; /* unsigned 128-bit integer */
const char BleInt8[] = "\x0C"; /* signed 8-bit integer */
const char BleInt12[] = "\x0D"; /* signed 12-bit integer */
const char BleInt16[] = "\x0E"; /* signed 16-bit integer */
const char BleInt24[] = "\x0F"; /* signed 24-bit integer */
const char BleInt32[] = "\x10"; /* signed 32-bit integer */
const char BleInt48[] = "\x11"; /* signed 48-bit integer */
const char BleInt64[] = "\x12"; /* signed 64-bit integer */
const char BleInt128[] = "\x13"; /* signed 128-bit integer */

#endif
