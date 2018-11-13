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

#include "local/BLELocalCharacteristic.h"
#include "local/BLELocalDescriptor.h"
#include "local/BLELocalService.h"

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
  _connectionHandle(0xffff),
  _mtu(23),
  _longWriteHandle(0x0000),
  _longWriteValue(NULL),
  _longWriteValueLength(0)
{
  memset(_peerAddress, 0x00, sizeof(_peerAddress));
  memset(_eventHandlers, 0x00, sizeof(_eventHandlers));
}

ATTClass::~ATTClass()
{
  if (_longWriteValue) {
    free(_longWriteValue);
  }
}

void ATTClass::setMaxMtu(uint16_t maxMtu)
{
  _maxMtu = maxMtu;
}

void ATTClass::addConnection(uint16_t handle, uint8_t role, uint8_t /*peerBdaddrType*/,
                              uint8_t peerBdaddr[6], uint16_t /*interval*/,
                              uint16_t /*latency*/, uint16_t /*supervisionTimeout*/,
                              uint8_t /*masterClockAccuracy*/)
{
  if (role == 1) {
    _connectionHandle = handle;
    _mtu = 23;
    memcpy(_peerAddress, peerBdaddr, sizeof(_peerAddress));

    if (_eventHandlers[BLEConnected]) {
      _eventHandlers[BLEConnected](BLEDevice(_connectionHandle, _peerAddress));
    }
  }
}

void ATTClass::handleData(uint16_t connectionHandle, uint8_t dlen, uint8_t data[])
{
  uint8_t opcode = data[0];

  dlen--;
  data++;

  switch (opcode) {
    case ATT_OP_MTU_REQ:
      mtuReq(connectionHandle, dlen, data);
      break;

    case ATT_OP_FIND_INFO_REQ:
      findInfoReq(connectionHandle, dlen, data);
      break;

    case ATT_OP_FIND_BY_TYPE_REQ:
      findByTypeReq(connectionHandle, dlen, data);
      break;

    case ATT_OP_READ_BY_TYPE_REQ:
      readByTypeReq(connectionHandle, dlen, data);
      break;

    case ATT_OP_READ_BY_GROUP_REQ:
      readByGroupReq(connectionHandle, dlen, data);
      break;

    case ATT_OP_READ_REQ:
    case ATT_OP_READ_BLOB_REQ:
      readOrReadBlobReq(connectionHandle, opcode, dlen, data);
      break;

    case ATT_OP_WRITE_REQ:
    case ATT_OP_WRITE_CMD:
      writeReqOrCmd(connectionHandle, opcode, dlen, data);
      break;

    case ATT_OP_PREP_WRITE_REQ:
      prepWriteReq(connectionHandle, dlen, data);
      break;

    case ATT_OP_EXEC_WRITE_REQ:
      execWriteReq(connectionHandle, dlen, data);
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

void ATTClass::removeConnection(uint8_t handle, uint16_t /*reason*/)
{
  if (_connectionHandle == handle) {
    BLEDevice bleDevice(_connectionHandle, _peerAddress);

    // clear CCCD values on disconnect
    for (uint16_t i = 0; i < GATT.attributeCount(); i++) {
      BLEAttribute* attribute = GATT.attribute(i);

      if (attribute->type() == BLETypeCharacteristic) {
        BLELocalCharacteristic* characteristic = (BLELocalCharacteristic*)attribute;

        characteristic->writeCccdValue(bleDevice, 0x0000);
      }
    }

    if (_eventHandlers[BLEDisconnected]) {
      _eventHandlers[BLEDisconnected](bleDevice);
    }

    _connectionHandle = 0xffff;
    memset(_peerAddress, 0x00, sizeof(_peerAddress));
    _longWriteHandle = 0x0000;
    _longWriteValueLength = 0;
  }
}

bool ATTClass::connected() const
{
  return (_connectionHandle != 0xffff);
}

bool ATTClass::connected(uint16_t handle, const uint8_t address[6]) const
{
  return ((_connectionHandle == handle) && memcmp(_peerAddress, address, 6) == 0);
}

bool ATTClass::disconnect()
{
  if (_connectionHandle != 0xffff) {
    if (HCI.disconnect(_connectionHandle) != 0) {
      return false;
    }

    _connectionHandle = 0xffff;
    memset(_peerAddress, 0x00, sizeof(_peerAddress));
    return true;
  }

  return false;
}

BLEDevice ATTClass::central()
{
  if (connected()) {
    return BLEDevice(_connectionHandle, _peerAddress);
  }

  return BLEDevice();
}

bool ATTClass::handleNotify(uint16_t handle, const uint8_t* value, int length)
{
  if (_connectionHandle != 0xffff) {
    uint8_t notication[_mtu];
    uint16_t noticationLength = 0;

    notication[0] = ATT_OP_HANDLE_NOTIFY;
    noticationLength++;

    memcpy(&notication[1], &handle, sizeof(handle));
    noticationLength += sizeof(handle);

    length = min((uint16_t)(_mtu - noticationLength), (uint16_t)length);
    memcpy(&notication[noticationLength], value, length);
    noticationLength += length;

    HCI.sendAclPkt(_connectionHandle, ATT_CID, noticationLength, notication);

    return true;
  }

  return false;
}

bool ATTClass::handleInd(uint16_t handle, const uint8_t* value, int length)
{
  if (_connectionHandle != 0xffff) {
    uint8_t indication[_mtu];
    uint16_t indicationLength = 0;

    indication[0] = ATT_OP_HANDLE_IND;
    indicationLength++;

    memcpy(&indication[1], &handle, sizeof(handle));
    indicationLength += sizeof(handle);

    length = min((uint16_t)(_mtu - indicationLength), (uint16_t)length);
    memcpy(&indication[indicationLength], value, length);
    indicationLength += length;

    _cnf = false;

    HCI.sendAclPkt(_connectionHandle, ATT_CID, indicationLength, indication);

    while (!_cnf) {
      HCI.poll();

      if (!connected()) {
        return false;
      }
    }

    return true;
  }

  return false;
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

  _mtu = mtu;

  struct __attribute__ ((packed)) {
    uint8_t op;
    uint16_t mtu;
  } mtuResp = { ATT_OP_MTU_RESP, mtu };

  HCI.sendAclPkt(connectionHandle, ATT_CID, sizeof(mtuResp), &mtuResp);
}

void ATTClass::findInfoReq(uint16_t connectionHandle, uint8_t dlen, uint8_t data[])
{
  struct __attribute__ ((packed)) FindInfoReq {
    uint16_t startHandle;
    uint16_t endHandle;
  } *findInfoReq = (FindInfoReq*)data;

  if (dlen != sizeof(FindInfoReq)) {
    sendError(connectionHandle, ATT_OP_FIND_INFO_REQ, findInfoReq->startHandle, ATT_ECODE_INVALID_PDU);
    return;
  }

  uint8_t response[_mtu];
  uint16_t responseLength;

  response[0] = ATT_OP_FIND_INFO_RESP;
  response[1] = 0x00;
  responseLength = 2;

  for (uint16_t i = (findInfoReq->startHandle - 1); i < GATT.attributeCount() && i <= (findInfoReq->endHandle - 1); i++) {
    BLEAttribute* attribute = GATT.attribute(i);
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

    if ((responseLength + (2 + uuidLen)) > _mtu) {
      break;
    }
  }

  if (responseLength == 2) {
    sendError(connectionHandle, ATT_OP_FIND_INFO_REQ, findInfoReq->startHandle, ATT_ECODE_ATTR_NOT_FOUND);
  } else {
    HCI.sendAclPkt(connectionHandle, ATT_CID, responseLength, response);
  }
}

void ATTClass::findByTypeReq(uint16_t connectionHandle, uint8_t dlen, uint8_t data[])
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

  uint8_t response[_mtu];
  uint16_t responseLength;

  response[0] = ATT_OP_FIND_BY_TYPE_RESP;
  responseLength = 1;

  if (findByTypeReq->type == BLETypeService) {
    for (uint16_t i = (findByTypeReq->startHandle - 1); i < GATT.attributeCount() && i <= (findByTypeReq->endHandle - 1); i++) {
      BLEAttribute* attribute = GATT.attribute(i);

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

      if ((responseLength + 4) > _mtu) {
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

void ATTClass::readByGroupReq(uint16_t connectionHandle, uint8_t dlen, uint8_t data[])
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

  uint8_t response[_mtu];
  uint16_t responseLength;

  response[0] = ATT_OP_READ_BY_GROUP_RESP;
  response[1] = 0x00;
  responseLength = 2;

  for (uint16_t i = (readByGroupReq->startHandle - 1); i < GATT.attributeCount() && i <= (readByGroupReq->endHandle - 1); i++) {
    BLEAttribute* attribute = GATT.attribute(i);

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

    if ((responseLength + infoSize) > _mtu) {
      break;
    }
  }

  if (responseLength == 2) {
    sendError(connectionHandle, ATT_OP_READ_BY_GROUP_REQ, readByGroupReq->startHandle, ATT_ECODE_ATTR_NOT_FOUND);
  } else {
    HCI.sendAclPkt(connectionHandle, ATT_CID, responseLength, response);
  }
}

void ATTClass::readOrReadBlobReq(uint16_t connectionHandle, uint8_t opcode, uint8_t dlen, uint8_t data[])
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

  uint8_t response[_mtu];
  uint16_t responseLength;

  response[0] = (opcode == ATT_OP_READ_REQ) ? ATT_OP_READ_RESP : ATT_OP_READ_BLOB_RESP;
  responseLength = 1;

  BLEAttribute* attribute = GATT.attribute(handle - 1);
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

      valueLength = min(_mtu - responseLength, valueLength - offset);

      characteristic->readValue(BLEDevice(connectionHandle, _peerAddress), offset, &response[responseLength], valueLength);
      responseLength += valueLength;
    }
  } else if (attributeType == BLETypeDescriptor) {
    BLELocalDescriptor* descriptor = (BLELocalDescriptor*)attribute;
    
    uint16_t valueLength = descriptor->valueSize();

    if (offset >= valueLength) {
      sendError(connectionHandle, opcode, handle, ATT_ECODE_INVALID_OFFSET);
      return;
    }

    valueLength = min(_mtu - responseLength, valueLength - offset);

    memcpy(&response[responseLength], descriptor->value() + offset, valueLength);
    responseLength += valueLength;
  }
  
  HCI.sendAclPkt(connectionHandle, ATT_CID, responseLength, response);
}

void ATTClass::readByTypeReq(uint16_t connectionHandle, uint8_t dlen, uint8_t data[])
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

  uint8_t response[_mtu];
  uint16_t responseLength;

  response[0] = ATT_OP_READ_BY_TYPE_RESP;
  response[1] = 0x00;
  responseLength = 2;

  for (uint16_t i = (readByTypeReq->startHandle - 1); i < GATT.attributeCount() && i <= (readByTypeReq->endHandle - 1); i++) {
    BLEAttribute* attribute = GATT.attribute(i);
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

        if ((responseLength + typeSize) > _mtu) {
          break;
        }
      } else if (attribute->type() == 0x2902) {
        BLELocalDescriptor* descriptor = (BLELocalDescriptor*)attribute;

        // add the handle
        memcpy(&response[responseLength], &handle, sizeof(handle));
        responseLength += sizeof(handle);

        // add the value
        int valueSize = min((uint16_t)(_mtu - responseLength), (uint16_t)descriptor->valueSize());
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
      int valueLength = min((uint16_t)(_mtu - responseLength), (uint16_t)characteristic->valueLength());
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

void ATTClass::writeReqOrCmd(uint16_t connectionHandle, uint8_t op, uint8_t dlen, uint8_t data[])
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

  BLEAttribute* attribute = GATT.attribute(handle - 1);

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

    if (connectionHandle == _connectionHandle) {
      characteristic->writeValue(BLEDevice(connectionHandle, _peerAddress), value, valueLength);
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

    if (connectionHandle == _connectionHandle) {
      characteristic->writeCccdValue(BLEDevice(connectionHandle, _peerAddress), *((uint16_t*)value));
    }
  } else {
    if (withResponse) {
      sendError(connectionHandle, ATT_OP_WRITE_REQ, handle, ATT_ECODE_WRITE_NOT_PERM);
    }
    return;
  }

  if (withResponse) {
    uint8_t response[_mtu];
    uint16_t responseLength;

    response[0] = ATT_OP_WRITE_RESP;
    responseLength = 1;

    HCI.sendAclPkt(connectionHandle, ATT_CID, responseLength, response);
  }
}

void ATTClass::prepWriteReq(uint16_t connectionHandle, uint8_t dlen, uint8_t data[])
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

  BLEAttribute* attribute = GATT.attribute(handle - 1);

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

  uint8_t response[_mtu];
  uint16_t responseLength;

  response[0] = ATT_OP_PREP_WRITE_RESP;
  memcpy(&response[1], data, dlen);
  responseLength = dlen + 1;

  HCI.sendAclPkt(connectionHandle, ATT_CID, responseLength, response);
}

void ATTClass::execWriteReq(uint16_t connectionHandle, uint8_t dlen, uint8_t data[])
{
  if (dlen != sizeof(uint8_t)) {
    sendError(connectionHandle, ATT_OP_EXEC_WRITE_REQ, 0x0000, ATT_ECODE_INVALID_PDU);
    return;
  }

  uint8_t flag = data[0];

  if (_longWriteHandle && (flag & 0x01)) {
    BLELocalCharacteristic* characteristic = (BLELocalCharacteristic*)GATT.attribute(_longWriteHandle - 1);

    if (connectionHandle == _connectionHandle) {
      characteristic->writeValue(BLEDevice(connectionHandle, _peerAddress), _longWriteValue, _longWriteValueLength);
    }
  }

  _longWriteHandle = 0x0000;
  _longWriteValueLength = 0;

  uint8_t response[_mtu];
  uint16_t responseLength;

  response[0] = ATT_OP_EXEC_WRITE_RESP;
  responseLength = 1;

  HCI.sendAclPkt(connectionHandle, ATT_CID, responseLength, response);
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

void ATTClass::setEventHandler(BLEDeviceEvent event, BLEDeviceEventHandler eventHandler)
{
  if (event < (sizeof(_eventHandlers) / (sizeof(_eventHandlers[0])))) {
    _eventHandlers[event] = eventHandler;
  }
}

ATTClass ATT;
