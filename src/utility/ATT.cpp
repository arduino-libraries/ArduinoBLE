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

#include <Arduino.h>

#include "HCI.h"
#include "GATT.h"

#include "local/BLELocalAttribute.h"
#include "local/BLELocalCharacteristic.h"
#include "local/BLELocalDescriptor.h"
#include "local/BLELocalService.h"

#include "remote/BLERemoteDevice.h"
#include "remote/BLERemoteService.h"

#include "BLEProperty.h"

#include "ATT.h"

#define ATT_OP_ERROR              0x01
#define ATT_OP_MTU_REQ            0x02
#define ATT_OP_MTU_RESP           0x03
#define ATT_OP_FIND_INFO_REQ      0x04
#define ATT_OP_FIND_INFO_RESP     0x05
#define ATT_OP_FIND_BY_TYPE_REQ   0x06
#define ATT_OP_FIND_BY_TYPE_RESP  0x07
#define ATT_OP_READ_BY_TYPE_REQ   0x08
#define ATT_OP_READ_BY_TYPE_RESP  0x09
#define ATT_OP_READ_REQ           0x0a
#define ATT_OP_READ_RESP          0x0b
#define ATT_OP_READ_BLOB_REQ      0x0c
#define ATT_OP_READ_BLOB_RESP     0x0d
#define ATT_OP_READ_MULTI_REQ     0x0e
#define ATT_OP_READ_MULTI_RESP    0x0f
#define ATT_OP_READ_BY_GROUP_REQ  0x10
#define ATT_OP_READ_BY_GROUP_RESP 0x11
#define ATT_OP_WRITE_REQ          0x12
#define ATT_OP_WRITE_RESP         0x13
#define ATT_OP_WRITE_CMD          0x52
#define ATT_OP_PREP_WRITE_REQ     0x16
#define ATT_OP_PREP_WRITE_RESP    0x17
#define ATT_OP_EXEC_WRITE_REQ     0x18
#define ATT_OP_EXEC_WRITE_RESP    0x19
#define ATT_OP_HANDLE_NOTIFY      0x1b
#define ATT_OP_HANDLE_IND         0x1d
#define ATT_OP_HANDLE_CNF         0x1e
#define ATT_OP_SIGNED_WRITE_CMD   0xd2

#define ATT_ECODE_INVALID_HANDLE       0x01
#define ATT_ECODE_READ_NOT_PERM        0x02
#define ATT_ECODE_WRITE_NOT_PERM       0x03
#define ATT_ECODE_INVALID_PDU          0x04
#define ATT_ECODE_AUTHENTICATION       0x05
#define ATT_ECODE_REQ_NOT_SUPP         0x06
#define ATT_ECODE_INVALID_OFFSET       0x07
#define ATT_ECODE_AUTHORIZATION        0x08
#define ATT_ECODE_PREP_QUEUE_FULL      0x09
#define ATT_ECODE_ATTR_NOT_FOUND       0x0a
#define ATT_ECODE_ATTR_NOT_LONG        0x0b
#define ATT_ECODE_INSUFF_ENCR_KEY_SIZE 0x0c
#define ATT_ECODE_INVAL_ATTR_VALUE_LEN 0x0d
#define ATT_ECODE_UNLIKELY             0x0e
#define ATT_ECODE_INSUFF_ENC           0x0f
#define ATT_ECODE_UNSUPP_GRP_TYPE      0x10
#define ATT_ECODE_INSUFF_RESOURCES     0x11

ATTClass::ATTClass() :
  _maxMtu(23),
  _timeout(5000),
  _longWriteHandle(0x0000),
  _longWriteValue(NULL),
  _longWriteValueLength(0)
{
  for (int i = 0; i < ATT_MAX_PEERS; i++) {
    _peers[i].connectionHandle = 0xffff;
    _peers[i].role = 0x00;
    _peers[i].addressType = 0x00;
    memset(_peers[i].address, 0x00, sizeof(_peers[i].address));
    _peers[i].mtu = 23;
    _peers[i].device = NULL;
  }

  memset(_eventHandlers, 0x00, sizeof(_eventHandlers));
}

ATTClass::~ATTClass()
{
  if (_longWriteValue) {
    free(_longWriteValue);
  }
}

bool ATTClass::connect(uint8_t peerBdaddrType, uint8_t peerBdaddr[6])
{
  if (HCI.leCreateConn(0x0060, 0x0030, 0x00, peerBdaddrType, peerBdaddr, 0x00,
                        0x0006, 0x000c, 0x0000, 0x00c8, 0x0004, 0x0006) != 0) {
    return false;
  }

  bool isConnected = false;

  for (unsigned long start = millis(); (millis() - start) < _timeout;) {
    HCI.poll();

    isConnected = connected(peerBdaddrType, peerBdaddr);

    if (isConnected) {
      break;
    }
  }

  if (!isConnected) {
    HCI.leCancelConn();
  }

  return isConnected;
}

bool ATTClass::disconnect(uint8_t peerBdaddrType, uint8_t peerBdaddr[6])
{
  uint16_t connHandle = connectionHandle(peerBdaddrType, peerBdaddr);
  if (connHandle == 0xffff) {
    return false;
  }

  HCI.disconnect(connHandle);

  for (unsigned long start = millis(); (millis() - start) < _timeout;) {
    HCI.poll();

    if (!connected(connHandle)) {
      return true;
    }
  }

  return false;
}

bool ATTClass::discoverAttributes(uint8_t peerBdaddrType, uint8_t peerBdaddr[6], const char* serviceUuidFilter)
{
  uint16_t connHandle = connectionHandle(peerBdaddrType, peerBdaddr);
  if (connHandle == 0xffff) {
    return false;
  }

  // send MTU request
  if (!exchangeMtu(connHandle)) {
    return false;
  }

  // find the device entry for the peeer
  BLERemoteDevice* device = NULL;

  for (int i = 0; i < ATT_MAX_PEERS; i++) {
    if (_peers[i].connectionHandle == connHandle) {
      if (_peers[i].device == NULL) {
        _peers[i].device = new BLERemoteDevice();
      }

      device = _peers[i].device;

      break;
    }
  }

  if (device == NULL) {
    return false;
  }

  if (serviceUuidFilter == NULL) {
    // clear existing services
    device->clearServices();
  } else {
    int serviceCount = device->serviceCount();
  
    for (int i = 0; i < serviceCount; i++) {
      BLERemoteService* service = device->service(i);

      if (strcasecmp(service->uuid(), serviceUuidFilter) == 0) {
        // found an existing service with same UUID
        return true;
      }
    }
  }

  // discover services
  if (!discoverServices(connHandle, device, serviceUuidFilter)) {
    return false;
  }

  // discover characteristics
  if (!discoverCharacteristics(connHandle, device)) {
    return false;
  }

  // discover descriptors
  if (!discoverDescriptors(connHandle, device)) {
    return false;
  }

  return true;
}

void ATTClass::setMaxMtu(uint16_t maxMtu)
{
  _maxMtu = maxMtu;
}

void ATTClass::setTimeout(unsigned long timeout)
{
  _timeout = timeout;
}

void ATTClass::addConnection(uint16_t handle, uint8_t role, uint8_t peerBdaddrType,
                              uint8_t peerBdaddr[6], uint16_t /*interval*/,
                              uint16_t /*latency*/, uint16_t /*supervisionTimeout*/,
                              uint8_t /*masterClockAccuracy*/)
{
  int peerIndex = -1;

  for (int i = 0; i < ATT_MAX_PEERS; i++) {
    if (_peers[i].connectionHandle == 0xffff) {
      peerIndex = i;
      break;
    }
  }

  if (peerIndex == -1) {
    // bail, no space
    return;
  }

  _peers[peerIndex].connectionHandle = handle;
  _peers[peerIndex].role = role;
  _peers[peerIndex].mtu = 23;
  _peers[peerIndex].addressType = peerBdaddrType;
  memcpy(_peers[peerIndex].address, peerBdaddr, sizeof(_peers[peerIndex].address));

  if (_eventHandlers[BLEConnected]) {
    _eventHandlers[BLEConnected](BLEDevice(peerBdaddrType, peerBdaddr));
  }
}

void ATTClass::handleData(uint16_t connectionHandle, uint8_t dlen, uint8_t data[])
{
  uint8_t opcode = data[0];

  dlen--;
  data++;

  uint16_t mtu = this->mtu(connectionHandle);

  switch (opcode) {
    case ATT_OP_ERROR:
      error(connectionHandle, dlen, data);
      break;

    case ATT_OP_MTU_REQ:
      mtuReq(connectionHandle, dlen, data);
      break;

    case ATT_OP_MTU_RESP:
      mtuResp(connectionHandle, dlen, data);
      break;

    case ATT_OP_FIND_INFO_REQ:
      findInfoReq(connectionHandle, mtu, dlen, data);
      break;

    case ATT_OP_FIND_INFO_RESP:
      findInfoResp(connectionHandle, dlen, data);
      break;

    case ATT_OP_FIND_BY_TYPE_REQ:
      findByTypeReq(connectionHandle, mtu, dlen, data);
      break;

    case ATT_OP_READ_BY_TYPE_REQ:
      readByTypeReq(connectionHandle, mtu, dlen, data);
      break;

    case ATT_OP_READ_BY_TYPE_RESP:
      readByTypeResp(connectionHandle, dlen, data);
      break;

    case ATT_OP_READ_BY_GROUP_REQ:
      readByGroupReq(connectionHandle, mtu, dlen, data);
      break;

    case ATT_OP_READ_BY_GROUP_RESP:
      readByGroupResp(connectionHandle, dlen, data);
      break;

    case ATT_OP_READ_REQ:
    case ATT_OP_READ_BLOB_REQ:
      readOrReadBlobReq(connectionHandle, mtu, opcode, dlen, data);
      break;

    case ATT_OP_READ_RESP:
      readResp(connectionHandle, dlen, data);
      break;

    case ATT_OP_WRITE_REQ:
    case ATT_OP_WRITE_CMD:
      writeReqOrCmd(connectionHandle, mtu, opcode, dlen, data);
      break;

    case ATT_OP_WRITE_RESP:
      writeResp(connectionHandle, dlen, data);
      break;

    case ATT_OP_PREP_WRITE_REQ:
      prepWriteReq(connectionHandle, mtu, dlen, data);
      break;

    case ATT_OP_EXEC_WRITE_REQ:
      execWriteReq(connectionHandle, mtu, dlen, data);
      break;

    case ATT_OP_HANDLE_NOTIFY:
    case ATT_OP_HANDLE_IND:
      handleNotifyOrInd(connectionHandle, opcode, dlen, data);
      break;

    case ATT_OP_HANDLE_CNF:
      handleCnf(connectionHandle, dlen, data);
      break;

    case ATT_OP_READ_MULTI_REQ:
    case ATT_OP_SIGNED_WRITE_CMD:
    default:
      sendError(connectionHandle, opcode, 0x00, ATT_ECODE_REQ_NOT_SUPP);
      break;
  }
}

void ATTClass::removeConnection(uint16_t handle, uint8_t /*reason*/)
{
  int peerIndex = -1;
  int peerCount = 0;

  for (int i = 0; i < ATT_MAX_PEERS; i++) {
    if (_peers[i].connectionHandle == handle) {
      peerIndex = i;
    }

    if (_peers[i].connectionHandle != 0xffff) {
      peerCount++;
    }
  }

  if (peerIndex == -1) {
    // bail not found
    return;
  }

  BLEDevice bleDevice(_peers[peerIndex].addressType, _peers[peerIndex].address);

  if (peerCount == 1) {
    // clear CCCD values on disconnect
    for (uint16_t i = 0; i < GATT.attributeCount(); i++) {
      BLELocalAttribute* attribute = GATT.attribute(i);

      if (attribute->type() == BLETypeCharacteristic) {
        BLELocalCharacteristic* characteristic = (BLELocalCharacteristic*)attribute;

        characteristic->writeCccdValue(bleDevice, 0x0000);
      }
    }

    _longWriteHandle = 0x0000;
    _longWriteValueLength = 0;
  }

  if (_eventHandlers[BLEDisconnected]) {
    _eventHandlers[BLEDisconnected](bleDevice);
  }

  _peers[peerIndex].connectionHandle = 0xffff;
  _peers[peerIndex].role = 0x00;
  _peers[peerIndex].addressType = 0x00;
  memset(_peers[peerIndex].address, 0x00, sizeof(_peers[peerIndex].address));
  _peers[peerIndex].mtu = 23;

  if (_peers[peerIndex].device) {
    delete _peers[peerIndex].device;
  }
  _peers[peerIndex].device = NULL;
}

uint16_t ATTClass::connectionHandle(uint8_t addressType, const uint8_t address[6]) const
{
  for (int i = 0; i < ATT_MAX_PEERS; i++) {
    if (_peers[i].addressType == addressType && memcmp(_peers[i].address, address, 6) == 0) {
      return _peers[i].connectionHandle;
    }
  }

  return 0xffff;
}

BLERemoteDevice* ATTClass::device(uint8_t addressType, const uint8_t address[6]) const
{
  for (int i = 0; i < ATT_MAX_PEERS; i++) {
    if (_peers[i].addressType == addressType && memcmp(_peers[i].address, address, 6) == 0) {
      return _peers[i].device;
    }
  }

  return NULL;
}

bool ATTClass::connected() const
{
  for (int i = 0; i < ATT_MAX_PEERS; i++) {
    if (_peers[i].connectionHandle != 0xffff) {
      return true;
    }
  }

  return false;
}

bool ATTClass::connected(uint8_t addressType, const uint8_t address[6]) const
{
  return (connectionHandle(addressType, address) != 0xffff);
}

bool ATTClass::connected(uint16_t handle) const
{
  HCI.poll();

  for (int i = 0; i < ATT_MAX_PEERS; i++) {
    if (_peers[i].connectionHandle == handle) {
      return true;
    }
  }

  return false;
}

uint16_t ATTClass::mtu(uint16_t handle) const
{
  for (int i = 0; i < ATT_MAX_PEERS; i++) {
    if (_peers[i].connectionHandle == handle) {
      return _peers[i].mtu;
    }
  }

  return 23;
}

bool ATTClass::disconnect()
{
  int numDisconnects = 0;

  for (int i = 0; i < ATT_MAX_PEERS; i++) {
    if (_peers[i].connectionHandle == 0xffff) {
      continue;
    }

    if (HCI.disconnect(_peers[i].connectionHandle) != 0) {
      continue;
    }

    numDisconnects++;

    _peers[i].connectionHandle = 0xffff;
    _peers[i].role = 0x00;
    _peers[i].addressType = 0x00;
    memset(_peers[i].address, 0x00, sizeof(_peers[i].address));
    _peers[i].mtu = 23;

    if (_peers[i].device) {
      delete _peers[i].device;
    }
    _peers[i].device = NULL;
  }

  return (numDisconnects > 0);
}

BLEDevice ATTClass::central()
{
  for (int i = 0; i < ATT_MAX_PEERS; i++) {
    if (_peers[i].connectionHandle == 0xffff || _peers[i].role != 0x01) {
      continue;
    }

    return BLEDevice(_peers[i].addressType, _peers[i].address);
  }

  return BLEDevice();
}

bool ATTClass::handleNotify(uint16_t handle, const uint8_t* value, int length)
{
  int numNotifications = 0;

  for (int i = 0; i < ATT_MAX_PEERS; i++) {
    if (_peers[i].connectionHandle == 0xffff) {
      continue;
    }

    uint8_t notification[_peers[i].mtu];
    uint16_t notificationLength = 0;

    notification[0] = ATT_OP_HANDLE_NOTIFY;
    notificationLength++;

    memcpy(&notification[1], &handle, sizeof(handle));
    notificationLength += sizeof(handle);

    length = min((uint16_t)(_peers[i].mtu - notificationLength), (uint16_t)length);
    memcpy(&notification[notificationLength], value, length);
    notificationLength += length;

    HCI.sendAclPkt(_peers[i].connectionHandle, ATT_CID, notificationLength, notification);

    numNotifications++;
  }

  return (numNotifications > 0);
}

bool ATTClass::handleInd(uint16_t handle, const uint8_t* value, int length)
{
  int numIndications = 0;

  for (int i = 0; i < ATT_MAX_PEERS; i++) {
    if (_peers[i].connectionHandle == 0xffff) {
      continue;
    }

    uint8_t indication[_peers[i].mtu];
    uint16_t indicationLength = 0;

    indication[0] = ATT_OP_HANDLE_IND;
    indicationLength++;

    memcpy(&indication[1], &handle, sizeof(handle));
    indicationLength += sizeof(handle);

    length = min((uint16_t)(_peers[i].mtu - indicationLength), (uint16_t)length);
    memcpy(&indication[indicationLength], value, length);
    indicationLength += length;

    _cnf = false;

    HCI.sendAclPkt(_peers[i].connectionHandle, ATT_CID, indicationLength, indication);

    while (!_cnf) {
      HCI.poll();

      if (!connected(_peers[i].addressType, _peers[i].address)) {
        break;
      }
    }

    numIndications++;
  }

  return (numIndications > 0);
}

void ATTClass::error(uint16_t connectionHandle, uint8_t dlen, uint8_t data[])
{
  if (dlen != 4) {
    // drop
    return;
  } 

  struct __attribute__ ((packed)) AttError {
    uint8_t opcode;
    uint16_t handle;
    uint8_t code;
  } *attError = (AttError*)data;

  if (_pendingResp.connectionHandle == connectionHandle && (_pendingResp.op - 1) == attError->opcode) {
    _pendingResp.buffer[0] = ATT_OP_ERROR;
    memcpy(&_pendingResp.buffer[1], data, dlen);
    _pendingResp.length = dlen + 1;
  }
}

void ATTClass::mtuReq(uint16_t connectionHandle, uint8_t dlen, uint8_t data[])
{
  uint16_t mtu = *(uint16_t*)data;

  if (dlen != 2) {
    sendError(connectionHandle, ATT_OP_MTU_REQ, 0x0000, ATT_ECODE_INVALID_PDU);
    return;
  }

  if (mtu > _maxMtu) {
    mtu = _maxMtu;
  }

  for (int i = 0; i < ATT_MAX_PEERS; i++) {
    if (_peers[i].connectionHandle == connectionHandle) {
      _peers[i].mtu = mtu;
      break;
    }
  }

  struct __attribute__ ((packed)) {
    uint8_t op;
    uint16_t mtu;
  } mtuResp = { ATT_OP_MTU_RESP, mtu };

  HCI.sendAclPkt(connectionHandle, ATT_CID, sizeof(mtuResp), &mtuResp);
}

int ATTClass::mtuReq(uint16_t connectionHandle, uint16_t mtu, uint8_t responseBuffer[])
{
  struct __attribute__ ((packed)) {
    uint8_t op;
    uint16_t mtu;
  } mtuReq = { ATT_OP_MTU_REQ, mtu };

  return sendReq(connectionHandle, &mtuReq, sizeof(mtuReq), responseBuffer);
}

void ATTClass::mtuResp(uint16_t connectionHandle, uint8_t dlen, uint8_t data[])
{
  uint16_t mtu = *(uint16_t*)data;

  if (dlen != 2) {
    return;
  }

  for (int i = 0; i < ATT_MAX_PEERS; i++) {
    if (_peers[i].connectionHandle == connectionHandle) {
      _peers[i].mtu = mtu;
      break;
    }
  }

  if (connectionHandle == _pendingResp.connectionHandle && _pendingResp.op == ATT_OP_MTU_RESP) {
    _pendingResp.buffer[0] = ATT_OP_MTU_RESP;
    memcpy(&_pendingResp.buffer[1], data, dlen);
    _pendingResp.length = dlen + 1;
  }
}

void ATTClass::findInfoReq(uint16_t connectionHandle, uint16_t mtu, uint8_t dlen, uint8_t data[])
{
  struct __attribute__ ((packed)) FindInfoReq {
    uint16_t startHandle;
    uint16_t endHandle;
  } *findInfoReq = (FindInfoReq*)data;

  if (dlen != sizeof(FindInfoReq)) {
    sendError(connectionHandle, ATT_OP_FIND_INFO_REQ, findInfoReq->startHandle, ATT_ECODE_INVALID_PDU);
    return;
  }

  uint8_t response[mtu];
  uint16_t responseLength;

  response[0] = ATT_OP_FIND_INFO_RESP;
  response[1] = 0x00;
  responseLength = 2;

  for (uint16_t i = (findInfoReq->startHandle - 1); i < GATT.attributeCount() && i <= (findInfoReq->endHandle - 1); i++) {
    BLELocalAttribute* attribute = GATT.attribute(i);
    uint16_t handle = (i + 1);
    bool isValueHandle = (attribute->type() == BLETypeCharacteristic) && (((BLELocalCharacteristic*)attribute)->valueHandle() == handle);
    int uuidLen = isValueHandle ? 2 : attribute->uuidLength();
    int infoType = (uuidLen == 2) ? 0x01 : 0x02;

    if (response[1] == 0) {
      response[1] = infoType;
    }

    if (response[1] != infoType) {
      // different type
      break;
    }

    // add the handle
    memcpy(&response[responseLength], &handle, sizeof(handle));
    responseLength += sizeof(handle);

    if (isValueHandle || attribute->type() == BLETypeDescriptor) {
      // add the UUID
      memcpy(&response[responseLength], attribute->uuidData(), uuidLen);
      responseLength += uuidLen;
    } else {
      // add the type
      uint16_t type = attribute->type();

      memcpy(&response[responseLength], &type, sizeof(type));
      responseLength += sizeof(type);
    }

    if ((responseLength + (2 + uuidLen)) > mtu) {
      break;
    }
  }

  if (responseLength == 2) {
    sendError(connectionHandle, ATT_OP_FIND_INFO_REQ, findInfoReq->startHandle, ATT_ECODE_ATTR_NOT_FOUND);
  } else {
    HCI.sendAclPkt(connectionHandle, ATT_CID, responseLength, response);
  }
}

int ATTClass::findInfoReq(uint16_t connectionHandle, uint16_t startHandle, uint16_t endHandle, uint8_t responseBuffer[])
{
  struct __attribute__ ((packed)) {
    uint8_t op;
    uint16_t startHandle;
    uint16_t endHandle;
  } findInfoReq = { ATT_OP_FIND_INFO_REQ, startHandle, endHandle };

  return sendReq(connectionHandle, &findInfoReq, sizeof(findInfoReq), responseBuffer);
}

void ATTClass::findInfoResp(uint16_t connectionHandle, uint8_t dlen, uint8_t data[])
{
  if (dlen < 2) {
    return; // invalid, drop
  }

  if (connectionHandle == _pendingResp.connectionHandle && _pendingResp.op == ATT_OP_FIND_INFO_RESP) {
    _pendingResp.buffer[0] = ATT_OP_FIND_INFO_RESP;
    memcpy(&_pendingResp.buffer[1], data, dlen);
    _pendingResp.length = dlen + 1;
  }
}

void ATTClass::findByTypeReq(uint16_t connectionHandle, uint16_t mtu, uint8_t dlen, uint8_t data[])
{
  struct __attribute__ ((packed)) FindByTypeReq {
    uint16_t startHandle;
    uint16_t endHandle;
    uint16_t type;
  } *findByTypeReq = (FindByTypeReq*)data;

  if (dlen < sizeof(FindByTypeReq)) {
    sendError(connectionHandle, ATT_OP_FIND_BY_TYPE_RESP, findByTypeReq->startHandle, ATT_ECODE_INVALID_PDU);
    return;
  }

  uint16_t valueLength = dlen - sizeof(*findByTypeReq);
  uint8_t* value = &data[sizeof(*findByTypeReq)];

  uint8_t response[mtu];
  uint16_t responseLength;

  response[0] = ATT_OP_FIND_BY_TYPE_RESP;
  responseLength = 1;

  if (findByTypeReq->type == BLETypeService) {
    for (uint16_t i = (findByTypeReq->startHandle - 1); i < GATT.attributeCount() && i <= (findByTypeReq->endHandle - 1); i++) {
      BLELocalAttribute* attribute = GATT.attribute(i);

      if ((attribute->type() == findByTypeReq->type) && (attribute->uuidLength() == valueLength) && memcmp(attribute->uuidData(), value, valueLength) == 0) {
        BLELocalService* service = (BLELocalService*)attribute;

        // add the start handle
        uint16_t startHandle = service->startHandle();
        memcpy(&response[responseLength], &startHandle, sizeof(startHandle));
        responseLength += sizeof(startHandle);

        // add the end handle
        uint16_t endHandle = service->endHandle();
        memcpy(&response[responseLength], &endHandle, sizeof(endHandle));
        responseLength += sizeof(endHandle);
      }

      if ((responseLength + 4) > mtu) {
        break;
      }
    }
  }

  if (responseLength == 1) {
    sendError(connectionHandle, ATT_OP_FIND_BY_TYPE_RESP, findByTypeReq->startHandle, ATT_ECODE_ATTR_NOT_FOUND);
  } else {
    HCI.sendAclPkt(connectionHandle, ATT_CID, responseLength, response);
  }
}

void ATTClass::readByGroupReq(uint16_t connectionHandle, uint16_t mtu, uint8_t dlen, uint8_t data[])
{
  struct __attribute__ ((packed)) ReadByGroupReq {
    uint16_t startHandle;
    uint16_t endHandle;
    uint16_t uuid;
  } *readByGroupReq = (ReadByGroupReq*)data;

  if (dlen != sizeof(ReadByGroupReq) || (readByGroupReq->uuid != BLETypeService && readByGroupReq->uuid != 0x2801)) {
    sendError(connectionHandle, ATT_OP_READ_BY_GROUP_REQ, readByGroupReq->startHandle, ATT_ECODE_UNSUPP_GRP_TYPE);
    return;
  }

  uint8_t response[mtu];
  uint16_t responseLength;

  response[0] = ATT_OP_READ_BY_GROUP_RESP;
  response[1] = 0x00;
  responseLength = 2;

  for (uint16_t i = (readByGroupReq->startHandle - 1); i < GATT.attributeCount() && i <= (readByGroupReq->endHandle - 1); i++) {
    BLELocalAttribute* attribute = GATT.attribute(i);

    if (readByGroupReq->uuid != attribute->type()) {
      // not the type
      continue;
    }

    int uuidLen = attribute->uuidLength();
    int infoSize = (uuidLen == 2) ? 6 : 20;

    if (response[1] == 0) {
      response[1] = infoSize;
    }

    if (response[1] != infoSize) {
      // different size
      break;
    }

    BLELocalService* service = (BLELocalService*)attribute;

    // add the start handle
    uint16_t startHandle = service->startHandle();
    memcpy(&response[responseLength], &startHandle, sizeof(startHandle));
    responseLength += sizeof(startHandle);

    // add the end handle
    uint16_t endHandle = service->endHandle();
    memcpy(&response[responseLength], &endHandle, sizeof(endHandle));
    responseLength += sizeof(endHandle);

    // add the UUID
    memcpy(&response[responseLength], service->uuidData(), uuidLen);
    responseLength += uuidLen;

    if ((responseLength + infoSize) > mtu) {
      break;
    }
  }

  if (responseLength == 2) {
    sendError(connectionHandle, ATT_OP_READ_BY_GROUP_REQ, readByGroupReq->startHandle, ATT_ECODE_ATTR_NOT_FOUND);
  } else {
    HCI.sendAclPkt(connectionHandle, ATT_CID, responseLength, response);
  }
}

int ATTClass::readByGroupReq(uint16_t connectionHandle, uint16_t startHandle, uint16_t endHandle, uint16_t uuid, uint8_t responseBuffer[])
{
  struct __attribute__ ((packed)) {
    uint8_t op;
    uint16_t startHandle;
    uint16_t endHandle;
    uint16_t uuid;
  } readByGroupReq = { ATT_OP_READ_BY_GROUP_REQ, startHandle, endHandle, uuid };

  return sendReq(connectionHandle, &readByGroupReq, sizeof(readByGroupReq), responseBuffer);
}

void ATTClass::readByGroupResp(uint16_t connectionHandle, uint8_t dlen, uint8_t data[])
{
  if (dlen < 2) {
    return; // invalid, drop
  }

  if (connectionHandle == _pendingResp.connectionHandle && _pendingResp.op == ATT_OP_READ_BY_GROUP_RESP) {
    _pendingResp.buffer[0] = ATT_OP_READ_BY_GROUP_RESP;
    memcpy(&_pendingResp.buffer[1], data, dlen);
    _pendingResp.length = dlen + 1;
  }
}

void ATTClass::readOrReadBlobReq(uint16_t connectionHandle, uint16_t mtu, uint8_t opcode, uint8_t dlen, uint8_t data[])
{
  if (opcode == ATT_OP_READ_REQ) {
    if (dlen != sizeof(uint16_t)) {
      sendError(connectionHandle, ATT_OP_READ_REQ, 0x0000, ATT_ECODE_INVALID_PDU);
      return;
    }
  } else {
    if (dlen != (sizeof(uint16_t) + sizeof(uint16_t))) {
      sendError(connectionHandle, ATT_OP_READ_BLOB_REQ, 0x0000, ATT_ECODE_INVALID_PDU);
      return;
    }
  }

  uint16_t handle = *(uint16_t*)data;
  uint16_t offset = (opcode == ATT_OP_READ_REQ) ? 0 : *(uint16_t*)&data[sizeof(handle)];

  if ((uint16_t)(handle - 1) > GATT.attributeCount()) {
    sendError(connectionHandle, opcode, handle, ATT_ECODE_ATTR_NOT_FOUND);
    return;
  }

  uint8_t response[mtu];
  uint16_t responseLength;

  response[0] = (opcode == ATT_OP_READ_REQ) ? ATT_OP_READ_RESP : ATT_OP_READ_BLOB_RESP;
  responseLength = 1;

  BLELocalAttribute* attribute = GATT.attribute(handle - 1);
  enum BLEAttributeType attributeType = attribute->type();

  if (attributeType == BLETypeService) {
    if (offset) {
      sendError(connectionHandle, ATT_ECODE_ATTR_NOT_LONG, handle, ATT_ECODE_INVALID_PDU);
      return;
    }

    BLELocalService* service = (BLELocalService*)attribute;

    // add the UUID
    uint8_t uuidLen = service->uuidLength();
    memcpy(&response[responseLength], service->uuidData(), uuidLen);
    responseLength += uuidLen;
  } else if (attributeType == BLETypeCharacteristic) {
    BLELocalCharacteristic* characteristic = (BLELocalCharacteristic*)attribute;

    if (characteristic->handle() == handle) {
      if (offset) {
        sendError(connectionHandle, opcode, handle, ATT_ECODE_ATTR_NOT_LONG);
        return;
      }

      // add the properties
      response[responseLength++] = characteristic->properties();

      // add the value handle
      uint16_t valueHandle = characteristic->valueHandle();
      memcpy(&response[responseLength], &valueHandle, sizeof(valueHandle));
      responseLength += sizeof(valueHandle);

      // add the UUID
      uint8_t uuidLen = characteristic->uuidLength();
      memcpy(&response[responseLength], characteristic->uuidData(), uuidLen);
      responseLength += uuidLen;
    } else {
      if ((characteristic->properties() & BLERead) == 0) {
        sendError(connectionHandle, opcode, handle, ATT_ECODE_READ_NOT_PERM);
        return;
      }

      uint16_t valueLength = characteristic->valueLength();

      if (offset >= valueLength) {
        sendError(connectionHandle, opcode, handle, ATT_ECODE_INVALID_OFFSET);
        return;
      }

      valueLength = min(mtu - responseLength, valueLength - offset);

      for (int i = 0; i < ATT_MAX_PEERS; i++) {
        if (_peers[i].connectionHandle == connectionHandle) {
          characteristic->readValue(BLEDevice(_peers[i].addressType, _peers[i].address), offset, &response[responseLength], valueLength);
          responseLength += valueLength;
        }
      }
    }
  } else if (attributeType == BLETypeDescriptor) {
    BLELocalDescriptor* descriptor = (BLELocalDescriptor*)attribute;
    
    uint16_t valueLength = descriptor->valueSize();

    if (offset >= valueLength) {
      sendError(connectionHandle, opcode, handle, ATT_ECODE_INVALID_OFFSET);
      return;
    }

    valueLength = min(mtu - responseLength, valueLength - offset);

    memcpy(&response[responseLength], descriptor->value() + offset, valueLength);
    responseLength += valueLength;
  }
  
  HCI.sendAclPkt(connectionHandle, ATT_CID, responseLength, response);
}

void ATTClass::readResp(uint16_t connectionHandle, uint8_t dlen, uint8_t data[])
{
  if (connectionHandle == _pendingResp.connectionHandle && _pendingResp.op == ATT_OP_READ_RESP) {
    _pendingResp.buffer[0] = ATT_OP_READ_RESP;
    memcpy(&_pendingResp.buffer[1], data, dlen);
    _pendingResp.length = dlen + 1;
  }
}

void ATTClass::readByTypeReq(uint16_t connectionHandle, uint16_t mtu, uint8_t dlen, uint8_t data[])
{
  struct __attribute__ ((packed)) ReadByTypeReq {
    uint16_t startHandle;
    uint16_t endHandle;
    uint16_t uuid;
  } *readByTypeReq = (ReadByTypeReq*)data;

  if (dlen != sizeof(ReadByTypeReq)) {
    sendError(connectionHandle, ATT_OP_READ_BY_TYPE_REQ, readByTypeReq->startHandle, ATT_ECODE_INVALID_PDU);
    return;
  }

  uint8_t response[mtu];
  uint16_t responseLength;

  response[0] = ATT_OP_READ_BY_TYPE_RESP;
  response[1] = 0x00;
  responseLength = 2;

  for (uint16_t i = (readByTypeReq->startHandle - 1); i < GATT.attributeCount() && i <= (readByTypeReq->endHandle - 1); i++) {
    BLELocalAttribute* attribute = GATT.attribute(i);
    uint16_t handle = (i + 1);

    if (attribute->type() == readByTypeReq->uuid) {
      if (attribute->type() == BLETypeCharacteristic) {
        BLELocalCharacteristic* characteristic = (BLELocalCharacteristic*)attribute;

        if (characteristic->valueHandle() == handle) {
          // value handle, skip
          continue;
        }

        int uuidLen = attribute->uuidLength();
        int typeSize = (uuidLen == 2) ? 7 : 21;

        if (response[1] == 0) {
          response[1] = typeSize;
        }

        if (response[1] != typeSize) {
          // all done, wrong size
          break;
        }

        // add the handle
        memcpy(&response[responseLength], &handle, sizeof(handle));
        responseLength += sizeof(handle);

        // add the properties
        response[responseLength++] = characteristic->properties();

        // add the value handle
        uint16_t valueHandle = (handle + 1);
        memcpy(&response[responseLength], &valueHandle, sizeof(valueHandle));
        responseLength += sizeof(valueHandle);

        // add the UUID
        memcpy(&response[responseLength], characteristic->uuidData(), uuidLen);
        responseLength += uuidLen;

        // skip the next handle, it's a value handle
        i++;

        if ((responseLength + typeSize) > mtu) {
          break;
        }
      } else if (attribute->type() == 0x2902) {
        BLELocalDescriptor* descriptor = (BLELocalDescriptor*)attribute;

        // add the handle
        memcpy(&response[responseLength], &handle, sizeof(handle));
        responseLength += sizeof(handle);

        // add the value
        int valueSize = min((uint16_t)(mtu - responseLength), (uint16_t)descriptor->valueSize());
        memcpy(&response[responseLength], descriptor->value(), valueSize);
        responseLength += valueSize;

        response[1] = 2 + valueSize;

        break; // all done
      }
    } else if (attribute->type() == BLETypeCharacteristic && attribute->uuidLength() == 2 && memcmp(&readByTypeReq->uuid, attribute->uuidData(), 2) == 0) {
      BLELocalCharacteristic* characteristic = (BLELocalCharacteristic*)attribute;

      // add the handle
      memcpy(&response[responseLength], &handle, sizeof(handle));
      responseLength += sizeof(handle);

      // add the value
      int valueLength = min((uint16_t)(mtu - responseLength), (uint16_t)characteristic->valueLength());
      memcpy(&response[responseLength], characteristic->value(), valueLength);
      responseLength += valueLength;

      response[1] = 2 + valueLength;

      break; // all done
    }
  }

  if (responseLength == 2) {
    sendError(connectionHandle, ATT_OP_READ_BY_TYPE_REQ, readByTypeReq->startHandle, ATT_ECODE_ATTR_NOT_FOUND);
  } else {
    HCI.sendAclPkt(connectionHandle, ATT_CID, responseLength, response);
  }
}

int ATTClass::readByTypeReq(uint16_t connectionHandle, uint16_t startHandle, uint16_t endHandle, uint16_t type, uint8_t responseBuffer[])
{
  struct __attribute__ ((packed)) {
    uint8_t op;
    uint16_t startHandle;
    uint16_t endHandle;
    uint16_t type;
  } readByTypeReq = { ATT_OP_READ_BY_TYPE_REQ, startHandle, endHandle, type };

  return sendReq(connectionHandle, &readByTypeReq, sizeof(readByTypeReq), responseBuffer);
}

void ATTClass::readByTypeResp(uint16_t connectionHandle, uint8_t dlen, uint8_t data[])
{
  if (dlen < 1) {
    return; // invalid, drop
  }

  if (connectionHandle == _pendingResp.connectionHandle && _pendingResp.op == ATT_OP_READ_BY_TYPE_RESP) {
    _pendingResp.buffer[0] = ATT_OP_READ_BY_TYPE_RESP;
    memcpy(&_pendingResp.buffer[1], data, dlen);
    _pendingResp.length = dlen + 1;
  }
}

void ATTClass::writeReqOrCmd(uint16_t connectionHandle, uint16_t mtu, uint8_t op, uint8_t dlen, uint8_t data[])
{
  boolean withResponse = (op == ATT_OP_WRITE_REQ);

  if (dlen < sizeof(uint16_t)) {
    if (withResponse) {
      sendError(connectionHandle, ATT_OP_WRITE_REQ, 0x0000, ATT_ECODE_INVALID_PDU);
    }
    return;
  }

  uint16_t handle = *(uint16_t*)data;

  if ((uint16_t)(handle - 1) > GATT.attributeCount()) {
    if (withResponse) {
      sendError(connectionHandle, ATT_OP_WRITE_REQ, handle, ATT_ECODE_ATTR_NOT_FOUND);
    }
    return;
  }

  uint8_t valueLength = dlen - sizeof(handle);
  uint8_t* value = &data[sizeof(handle)];

  BLELocalAttribute* attribute = GATT.attribute(handle - 1);

  if (attribute->type() == BLETypeCharacteristic) {
    BLELocalCharacteristic* characteristic = (BLELocalCharacteristic*)attribute;
    
    if (handle != characteristic->valueHandle() || 
      withResponse ? ((characteristic->properties() & BLEWrite) == 0) : 
                     ((characteristic->properties() & BLEWriteWithoutResponse) == 0)) {
      if (withResponse) {
        sendError(connectionHandle, ATT_OP_WRITE_REQ, handle, ATT_ECODE_WRITE_NOT_PERM);
      }
      return;
    }

    for (int i = 0; i < ATT_MAX_PEERS; i++) {
      if (_peers[i].connectionHandle == connectionHandle) {
        characteristic->writeValue(BLEDevice(_peers[i].addressType, _peers[i].address), value, valueLength);
        break;
      }
    }
  } else if (attribute->type() == BLETypeDescriptor) {
    BLELocalDescriptor* descriptor = (BLELocalDescriptor*)attribute;

    // only CCCD's are writable
    if (descriptor->uuidLength() != 2 || *((uint16_t*)(descriptor->uuidData())) != 0x2902) {
      if (withResponse) {
        sendError(connectionHandle, ATT_OP_WRITE_REQ, handle, ATT_ECODE_WRITE_NOT_PERM);
      }
      return;
    }

    // get the previous handle, should be the characteristic for the CCCD
    attribute = GATT.attribute(handle - 2);

    if (attribute->type() != BLETypeCharacteristic) {
      if (withResponse) {
        sendError(connectionHandle, ATT_OP_WRITE_REQ, handle, ATT_ECODE_WRITE_NOT_PERM);
      }
      return;
    }

    BLELocalCharacteristic* characteristic = (BLELocalCharacteristic*)attribute;

    for (int i = 0; i < ATT_MAX_PEERS; i++) {
      if (_peers[i].connectionHandle == connectionHandle) {
        characteristic->writeCccdValue(BLEDevice(_peers[i].addressType, _peers[i].address), *((uint16_t*)value));
        break;
      }
    }
  } else {
    if (withResponse) {
      sendError(connectionHandle, ATT_OP_WRITE_REQ, handle, ATT_ECODE_WRITE_NOT_PERM);
    }
    return;
  }

  if (withResponse) {
    uint8_t response[mtu];
    uint16_t responseLength;

    response[0] = ATT_OP_WRITE_RESP;
    responseLength = 1;

    HCI.sendAclPkt(connectionHandle, ATT_CID, responseLength, response);
  }
}

void ATTClass::writeResp(uint16_t connectionHandle, uint8_t dlen, uint8_t data[])
{
  if (dlen != 0) {
    return; // drop
  }

  if (connectionHandle == _pendingResp.connectionHandle && _pendingResp.op == ATT_OP_WRITE_RESP) {
    _pendingResp.buffer[0] = ATT_OP_WRITE_RESP;
    memcpy(&_pendingResp.buffer[1], data, dlen);
    _pendingResp.length = dlen + 1;
  }
}

void ATTClass::prepWriteReq(uint16_t connectionHandle, uint16_t mtu, uint8_t dlen, uint8_t data[])
{
  struct __attribute__ ((packed)) PrepWriteReq {
    uint16_t handle;
    uint16_t offset;
  } *prepWriteReq = (PrepWriteReq*)data;

  if (dlen < sizeof(PrepWriteReq)) {
    sendError(connectionHandle, ATT_OP_PREP_WRITE_REQ, 0x0000, ATT_ECODE_INVALID_PDU);
    return;
  }

  uint16_t handle = prepWriteReq->handle;
  uint16_t offset = prepWriteReq->offset;

  if ((uint16_t)(handle - 1) > GATT.attributeCount()) {
    sendError(connectionHandle, ATT_OP_PREP_WRITE_REQ, handle, ATT_ECODE_ATTR_NOT_FOUND);
    return;
  }

  BLELocalAttribute* attribute = GATT.attribute(handle - 1);

  if (attribute->type() != BLETypeCharacteristic) {
    sendError(connectionHandle, ATT_OP_PREP_WRITE_REQ, handle, ATT_ECODE_ATTR_NOT_LONG);
    return;
  }

  BLELocalCharacteristic* characteristic = (BLELocalCharacteristic*)attribute;

  if (handle != characteristic->valueHandle()) {
    sendError(connectionHandle, ATT_OP_PREP_WRITE_REQ, handle, ATT_ECODE_ATTR_NOT_LONG);
    return;
  }

  if ((characteristic->properties() & BLEWrite) == 0) {
    sendError(connectionHandle, ATT_OP_PREP_WRITE_REQ, handle, ATT_ECODE_WRITE_NOT_PERM);
    return;
  }

  if (_longWriteHandle == 0) {
    int valueSize = characteristic->valueSize();

    _longWriteValue = (uint8_t*)realloc(_longWriteValue, valueSize);
    _longWriteValueLength = 0;
    _longWriteHandle = handle;

    memset(_longWriteValue, 0x00, valueSize);
  } else if (_longWriteHandle != handle) {
    sendError(connectionHandle, ATT_OP_PREP_WRITE_REQ, handle, ATT_ECODE_UNLIKELY);
    return;
  }

  uint8_t valueLength = dlen - sizeof(PrepWriteReq);
  uint8_t* value = &data[sizeof(PrepWriteReq)];

  if ((offset != _longWriteValueLength) || ((offset + valueLength) > (uint16_t)characteristic->valueSize())) {
    sendError(connectionHandle, ATT_OP_PREP_WRITE_REQ, handle, ATT_ECODE_INVALID_OFFSET);
    return;
  }

  memcpy(_longWriteValue + offset, value, valueLength);
  _longWriteValueLength += valueLength;

  uint8_t response[mtu];
  uint16_t responseLength;

  response[0] = ATT_OP_PREP_WRITE_RESP;
  memcpy(&response[1], data, dlen);
  responseLength = dlen + 1;

  HCI.sendAclPkt(connectionHandle, ATT_CID, responseLength, response);
}

void ATTClass::execWriteReq(uint16_t connectionHandle, uint16_t mtu, uint8_t dlen, uint8_t data[])
{
  if (dlen != sizeof(uint8_t)) {
    sendError(connectionHandle, ATT_OP_EXEC_WRITE_REQ, 0x0000, ATT_ECODE_INVALID_PDU);
    return;
  }

  uint8_t flag = data[0];

  if (_longWriteHandle && (flag & 0x01)) {
    BLELocalCharacteristic* characteristic = (BLELocalCharacteristic*)GATT.attribute(_longWriteHandle - 1);

    for (int i = 0; i < ATT_MAX_PEERS; i++) {
      if (_peers[i].connectionHandle == connectionHandle) {
        characteristic->writeValue(BLEDevice(_peers[i].addressType, _peers[i].address), _longWriteValue, _longWriteValueLength);
        break;
      }
    }
  }

  _longWriteHandle = 0x0000;
  _longWriteValueLength = 0;

  uint8_t response[mtu];
  uint16_t responseLength;

  response[0] = ATT_OP_EXEC_WRITE_RESP;
  responseLength = 1;

  HCI.sendAclPkt(connectionHandle, ATT_CID, responseLength, response);
}

void ATTClass::handleNotifyOrInd(uint16_t connectionHandle, uint8_t opcode, uint8_t dlen, uint8_t data[])
{
  if (dlen < 2) {
    return; // drop
  }

  struct __attribute__ ((packed)) HandleNotifyOrInd {
    uint16_t handle;
  } *handleNotifyOrInd = (HandleNotifyOrInd*)data;

  uint8_t handle = handleNotifyOrInd->handle;

  for (int i = 0; i < ATT_MAX_PEERS; i++) {
    if (_peers[i].connectionHandle != connectionHandle) {
      continue;
    }

    BLERemoteDevice* device = _peers[i].device;

    if (!device) {
      break;
    }

    int serviceCount = device->serviceCount();

    for (int i = 0; i < serviceCount; i++) {
      BLERemoteService* s = device->service(i);

      if (s->startHandle() < handle && s->endHandle() >= handle) {
        int characteristicCount = s->characteristicCount();

        for (int j = 0; j < characteristicCount; j++) {
          BLERemoteCharacteristic* c = s->characteristic(j);

          if (c->valueHandle() == handle) {
            c->writeValue(BLEDevice(_peers[i].addressType, _peers[i].address), &data[2], dlen - 2);
          }
        }

        break;
      }
    }
  }

  if (opcode == ATT_OP_HANDLE_IND) {
    // send CNF for IND

    uint8_t cnf = ATT_OP_HANDLE_CNF;

    HCI.sendAclPkt(connectionHandle, ATT_CID, sizeof(cnf), &cnf);
  }
}

void ATTClass::handleCnf(uint16_t /*connectionHandle*/, uint8_t /*dlen*/, uint8_t /*data*/[])
{
  _cnf = true;
}

void ATTClass::sendError(uint16_t connectionHandle, uint8_t opcode, uint16_t handle, uint8_t code)
{
  struct __attribute__ ((packed)) {
    uint8_t op;
    uint8_t opcode;
    uint16_t handle;
    uint8_t code;
  } attError = { ATT_OP_ERROR, opcode, handle, code };

  HCI.sendAclPkt(connectionHandle, ATT_CID, sizeof(attError), &attError);
}


bool ATTClass::exchangeMtu(uint16_t connectionHandle)
{
  uint8_t responseBuffer[_maxMtu];

  if (!mtuReq(connectionHandle, _maxMtu, responseBuffer)) {
    return false;
  }

  return true;
}

bool ATTClass::discoverServices(uint16_t connectionHandle, BLERemoteDevice* device, const char* serviceUuidFilter)
{
  uint16_t reqStartHandle = 0x0001;
  uint16_t reqEndHandle = 0xffff;

  uint8_t responseBuffer[_maxMtu];

  BLEUuid serviceUuid(serviceUuidFilter);

  while (reqEndHandle == 0xffff) {
    int respLength = readByGroupReq(connectionHandle, reqStartHandle, reqEndHandle, BLETypeService, responseBuffer);

    if (respLength == 0) {
      return false;
    }

    if (responseBuffer[0] == ATT_OP_READ_BY_GROUP_RESP) {
      uint16_t lengthPerService = responseBuffer[1];
      uint8_t uuidLen = lengthPerService - 4;

      for (int i = 2; i < respLength; i += lengthPerService) {
        struct __attribute__ ((packed)) RawService {
          uint16_t startHandle;
          uint16_t endHandle;
          uint8_t uuid[16];
        } *rawService = (RawService*)&responseBuffer[i];

        if (serviceUuidFilter == NULL || 
            (uuidLen == serviceUuid.length() && memcmp(rawService->uuid, serviceUuid.data(), uuidLen) == 0)) {
        
          BLERemoteService* service = new BLERemoteService(rawService->uuid, uuidLen,
                                                            rawService->startHandle,
                                                            rawService->endHandle);

          if (service == NULL) {
            return false;
          }

          device->addService(service);

        }

        reqStartHandle = rawService->endHandle + 1;

        if (reqStartHandle == 0x0000) {
          reqEndHandle = 0x0000;
        }
      }
    } else {
      reqEndHandle = 0x0000;
    }
  }

  return true;
}

bool ATTClass::discoverCharacteristics(uint16_t connectionHandle, BLERemoteDevice* device)
{
  uint16_t reqStartHandle = 0x0001;
  uint16_t reqEndHandle = 0xffff;

  uint8_t responseBuffer[_maxMtu];

  int serviceCount = device->serviceCount();
  
  for (int i = 0; i < serviceCount; i++) {
    BLERemoteService* service = device->service(i);

    reqStartHandle = service->startHandle();
    reqEndHandle = service->endHandle();

    while (1) {
      int respLength = readByTypeReq(connectionHandle, reqStartHandle, reqEndHandle, BLETypeCharacteristic, responseBuffer);

      if (respLength == 0) {
        return false;
      }

      if (responseBuffer[0] == ATT_OP_READ_BY_TYPE_RESP) {
        uint16_t lengthPerCharacteristic = responseBuffer[1];
        uint8_t uuidLen = lengthPerCharacteristic - 5;

        for (int i = 2; i < respLength; i += lengthPerCharacteristic) {
          struct __attribute__ ((packed)) RawCharacteristic {
            uint16_t startHandle;
            uint8_t properties;
            uint16_t valueHandle;
            uint8_t uuid[16];
          } *rawCharacteristic = (RawCharacteristic*)&responseBuffer[i];

          BLERemoteCharacteristic* characteristic = new BLERemoteCharacteristic(rawCharacteristic->uuid, uuidLen,
                                                                                connectionHandle,
                                                                                rawCharacteristic->startHandle,
                                                                                rawCharacteristic->properties,
                                                                                rawCharacteristic->valueHandle);

          if (characteristic == NULL) {
            return false;
          }

          service->addCharacteristic(characteristic);

          reqStartHandle = rawCharacteristic->valueHandle + 1;
        }
      } else {
        break;
      }
    }
  }

  return true;
}

bool ATTClass::discoverDescriptors(uint16_t connectionHandle, BLERemoteDevice* device)
{
  uint16_t reqStartHandle = 0x0001;
  uint16_t reqEndHandle = 0xffff;

  uint8_t responseBuffer[_maxMtu];

  int serviceCount = device->serviceCount();  

  for (int i = 0; i < serviceCount; i++) {
    BLERemoteService* service = device->service(i);

    uint16_t serviceEndHandle = service->endHandle();

    int characteristicCount = service->characteristicCount();

    for (int j = 0; j < characteristicCount; j++) {
      BLERemoteCharacteristic* characteristic = service->characteristic(j);
      BLERemoteCharacteristic* nextCharacteristic = (j == (characteristicCount - 1)) ? NULL : service->characteristic(j);

      reqStartHandle = characteristic->valueHandle() + 1;
      reqEndHandle = nextCharacteristic ? nextCharacteristic->valueHandle() : serviceEndHandle;

      if (reqStartHandle > reqEndHandle) {
        continue;
      }

      while (1) {
        int respLength = findInfoReq(connectionHandle, reqStartHandle, reqEndHandle, responseBuffer);

        if (respLength == 0) {
          return false;
        }

        if (responseBuffer[0] == ATT_OP_FIND_INFO_RESP) {
          uint16_t lengthPerDescriptor = responseBuffer[1] * 4;
          uint8_t uuidLen = 2;

          for (int i = 2; i < respLength; i += lengthPerDescriptor) {
            struct __attribute__ ((packed)) RawDescriptor {
              uint16_t handle;
              uint8_t uuid[16];
            } *rawDescriptor = (RawDescriptor*)&responseBuffer[i];

            BLERemoteDescriptor* descriptor = new BLERemoteDescriptor(rawDescriptor->uuid, uuidLen,
                                                                      connectionHandle,
                                                                      rawDescriptor->handle);

            if (descriptor == NULL) {
              return false;
            }

            characteristic->addDescriptor(descriptor);

            reqStartHandle = rawDescriptor->handle + 1;
          }
        } else {
          break;
        }
      }
    }
  }

  return true;
}

int ATTClass::sendReq(uint16_t connectionHandle, void* requestBuffer, int requestLength, uint8_t responseBuffer[])
{
  _pendingResp.connectionHandle = connectionHandle;
  _pendingResp.op = ((uint8_t*)requestBuffer)[0] + 1;
  _pendingResp.buffer = responseBuffer;
  _pendingResp.length = 0;

  HCI.sendAclPkt(connectionHandle, ATT_CID, requestLength, requestBuffer);

  if (responseBuffer == NULL) {
    // not waiting response
    return 0;
  } 

  for (unsigned long start = millis(); (millis() - start) < _timeout;) {
    HCI.poll();

    if (!connected(connectionHandle)) {
      break;
    }

    if (_pendingResp.length != 0) {
      _pendingResp.connectionHandle = 0xffff;
      return _pendingResp.length;
    }
  }

  _pendingResp.connectionHandle = 0xffff;
  return 0;
}

void ATTClass::setEventHandler(BLEDeviceEvent event, BLEDeviceEventHandler eventHandler)
{
  if (event < (sizeof(_eventHandlers) / (sizeof(_eventHandlers[0])))) {
    _eventHandlers[event] = eventHandler;
  }
}

int ATTClass::readReq(uint16_t connectionHandle, uint16_t handle, uint8_t responseBuffer[])
{
  struct __attribute__ ((packed)) {
    uint8_t op;
    uint16_t handle;
  } readReq = { ATT_OP_READ_REQ, handle };

  return sendReq(connectionHandle, &readReq, sizeof(readReq), responseBuffer);
}

int ATTClass::writeReq(uint16_t connectionHandle, uint16_t handle, const uint8_t* data, uint8_t dataLen, uint8_t responseBuffer[])
{
  struct __attribute__ ((packed)) {
    uint8_t op;
    uint16_t handle;
    uint8_t data[255];
  } writeReq;

  writeReq.op = ATT_OP_WRITE_REQ;
  writeReq.handle = handle;
  memcpy(writeReq.data, data, dataLen);

  return sendReq(connectionHandle, &writeReq, 3 + dataLen, responseBuffer);
}

void ATTClass::writeCmd(uint16_t connectionHandle, uint16_t handle, const uint8_t* data, uint8_t dataLen)
{
  struct __attribute__ ((packed)) {
    uint8_t op;
    uint16_t handle;
    uint8_t data[255];
  } writeReq;

  writeReq.op = ATT_OP_WRITE_CMD;
  writeReq.handle = handle;
  memcpy(writeReq.data, data, dataLen);

  sendReq(connectionHandle, &writeReq, 3 + dataLen, NULL);
}

ATTClass ATT;
