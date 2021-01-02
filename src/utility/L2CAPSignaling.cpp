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

#include "HCI.h"
#include "ATT.h"
#include "btct.h"
#include "L2CAPSignaling.h"
#include "keyDistribution.h"
#include "bitDescriptions.h"
#define CONNECTION_PARAMETER_UPDATE_REQUEST  0x12
#define CONNECTION_PARAMETER_UPDATE_RESPONSE 0x13

L2CAPSignalingClass::L2CAPSignalingClass() :
  _minInterval(0),
  _maxInterval(0),
  _supervisionTimeout(0)
{
}

L2CAPSignalingClass::~L2CAPSignalingClass()
{
}

void L2CAPSignalingClass::addConnection(uint16_t handle, uint8_t role, uint8_t /*peerBdaddrType*/,
                                        uint8_t /*peerBdaddr*/[6], uint16_t interval,
                                        uint16_t /*latency*/, uint16_t supervisionTimeout,
                                        uint8_t /*masterClockAccuracy*/)
{
  if (role != 1) {
    // ignore
    return;
  }

  bool updateParameters = false;
  uint16_t updatedMinInterval = interval;
  uint16_t updatedMaxInterval = interval;
  uint16_t updatedSupervisionTimeout = supervisionTimeout;

  if (_minInterval && _maxInterval) {
    if (interval < _minInterval || interval > _maxInterval) {
      updatedMinInterval = _minInterval;
      updatedMaxInterval = _maxInterval;
      updateParameters = true;
    }
  }

  if (_supervisionTimeout && supervisionTimeout != _supervisionTimeout) {
    updatedSupervisionTimeout = _supervisionTimeout;
    updateParameters = true;
  }

  if (updateParameters) {
    struct __attribute__ ((packed)) L2CAPConnectionParameterUpdateRequest {
      uint8_t code;
      uint8_t identifier;
      uint16_t length;
      uint16_t minInterval;
      uint16_t maxInterval;
      uint16_t latency;
      uint16_t supervisionTimeout;
    } request = { CONNECTION_PARAMETER_UPDATE_REQUEST, 0x01, 8,
                  updatedMinInterval, updatedMaxInterval, 0x0000, updatedSupervisionTimeout };

    HCI.sendAclPkt(handle, SIGNALING_CID, sizeof(request), &request);
  }
}

void L2CAPSignalingClass::handleData(uint16_t connectionHandle, uint8_t dlen, uint8_t data[])
{
  struct __attribute__ ((packed)) L2CAPSignalingHdr {
    uint8_t code;
    uint8_t identifier;
    uint16_t length;
  } *l2capSignalingHdr = (L2CAPSignalingHdr*)data;

  if (dlen < sizeof(L2CAPSignalingHdr)) {
    // too short, ignore
    return;
  }

  if (dlen != (sizeof(L2CAPSignalingHdr) + l2capSignalingHdr->length)) {
    // invalid length, ignore
    return;
  }

  uint8_t code = l2capSignalingHdr->code;
  uint8_t identifier = l2capSignalingHdr->identifier;
  uint16_t length = l2capSignalingHdr->length;
  data = &data[sizeof(L2CAPSignalingHdr)];

  if (code == CONNECTION_PARAMETER_UPDATE_REQUEST) {
    connectionParameterUpdateRequest(connectionHandle, identifier, length, data);
  } else if (code == CONNECTION_PARAMETER_UPDATE_RESPONSE) {
    connectionParameterUpdateResponse(connectionHandle, identifier, length, data);
  }
}
void L2CAPSignalingClass::handleSecurityData(uint16_t connectionHandle, uint8_t dlen, uint8_t data[])
{
  struct __attribute__ ((packed)) L2CAPSignalingHdr {
    uint8_t code;
    uint8_t data[64];
  } *l2capSignalingHdr = (L2CAPSignalingHdr*)data;
#ifdef _BLE_TRACE_
  Serial.print("dlen: ");
  Serial.println(dlen);
#endif
  uint8_t code = l2capSignalingHdr->code;

#ifdef _BLE_TRACE_
  Serial.print("handleSecurityData: code: 0x");
  Serial.println(code, HEX);
  Serial.print("rx security:");
  btct.printBytes(data,dlen);
#endif
  if (code == CONNECTION_PAIRING_REQUEST) {
    // 0x1
    struct __attribute__ ((packed)) PairingRequest {
      uint8_t ioCapability;
      uint8_t oobDataFlag;
      uint8_t authReq;
      uint8_t maxEncSize;
      uint8_t initiatorKeyDistribution;
      uint8_t responderKeyDistribution;
    } *pairingRequest = (PairingRequest*)l2capSignalingHdr->data;


    ATT.remoteKeyDistribution = KeyDistribution(pairingRequest->initiatorKeyDistribution);
    ATT.localKeyDistribution = KeyDistribution(pairingRequest->responderKeyDistribution);
    KeyDistribution rkd(pairingRequest->responderKeyDistribution);
    AuthReq req(pairingRequest->authReq);
#ifdef _BLE_TRACE_
    Serial.print("Req has properties: ");
    Serial.print(req.Bonding()?"bonding, ":"no bonding, ");
    Serial.print(req.CT2()?"CT2, ":"no CT2, ");
    Serial.print(req.KeyPress()?"KeyPress, ":"no KeyPress, ");
    Serial.print(req.MITM()?"MITM, ":"no MITM, ");
    Serial.print(req.SC()?"SC, ":"no SC, ");
#endif
    
    uint8_t peerIOCap[3];
    peerIOCap[0] = pairingRequest->authReq;
    peerIOCap[1] = pairingRequest->oobDataFlag;
    peerIOCap[2] = pairingRequest->ioCapability;
    ATT.setPeerIOCap(connectionHandle, peerIOCap);
    ATT.setPeerEncryption(connectionHandle, ATT.getPeerEncryption(connectionHandle) | PEER_ENCRYPTION::PAIRING_REQUEST);
#ifdef _BLE_TRACE_
    Serial.print("Peer encryption : 0b");
    Serial.println(ATT.getPeerEncryption(connectionHandle), BIN);
#endif
    struct __attribute__ ((packed)) PairingResponse {
      uint8_t code;
      uint8_t ioCapability;
      uint8_t oobDataFlag;
      uint8_t authReq;
      uint8_t maxEncSize;
      uint8_t initiatorKeyDistribution;
      uint8_t responderKeyDistribution;
    } response = { CONNECTION_PAIRING_RESPONSE, LOCAL_IOCAP, 0, LOCAL_AUTHREQ, 0x10, 0b1011, 0b1011};

    HCI.sendAclPkt(connectionHandle, SECURITY_CID, sizeof(response), &response);
  }
  else if (code == CONNECTION_PAIRING_RANDOM)
  {
    struct __attribute__ ((packed)) PairingRandom {
      uint8_t Na[16];
    } *pairingRandom = (PairingRandom*)l2capSignalingHdr->data;
    for(int i=0; i<16; i++){
      HCI.Na[15-i] = pairingRandom->Na[i];
    }
#ifdef _BLE_TRACE_
    Serial.println("[Info] Pairing random.");
#endif
    struct __attribute__ ((packed)) PairingResponse {
      uint8_t code;
      uint8_t Nb[16];
    } response = { CONNECTION_PAIRING_RANDOM, 0};
    for(int i=0; i< 16; i++) response.Nb[15-i] = HCI.Nb[i];

    HCI.sendAclPkt(connectionHandle, SECURITY_CID, sizeof(response), &response);
  }
  else if (code == CONNECTION_PAIRING_RESPONSE)
  {
  }
  else if(code == CONNECTION_PAIRING_FAILED)
  {
    struct __attribute__ ((packed)) PairingFailed
    {
      uint8_t code;
      uint8_t reason;
    } *pairingFailed = (PairingFailed*)data;
#ifdef _BLE_TRACE_
    Serial.print("Pairing failed with code: 0x");
    Serial.println(pairingFailed->reason,HEX);
#endif
  }
  else if (code == CONNECTION_IDENTITY_INFORMATION){
    struct __attribute__ ((packed)) IdentityInformation {
      uint8_t code;
      uint8_t PeerIRK[16];
    } *identityInformation = (IdentityInformation*)data;
    for(int i=0; i<16; i++) ATT.peerIRK[15-i] = identityInformation->PeerIRK[i];
#ifdef _BLE_TRACE_
    Serial.println("Saved peer IRK");
#endif
  }
  else if (code == CONNECTION_IDENTITY_ADDRESS){
    struct __attribute__ ((packed)) IdentityAddress {
      uint8_t code;
      uint8_t addressType;
      uint8_t address[6];
    } *identityAddress = (IdentityAddress*)data;
    // we can save this information now.
    uint8_t peerAddress[6];
    for(int i; i<6; i++) peerAddress[5-i] = identityAddress->address[i];

    HCI.saveNewAddress(identityAddress->addressType, peerAddress, ATT.peerIRK, ATT.localIRK);
    if(HCI._storeLTK!=0){
      HCI._storeLTK(peerAddress, HCI.LTK);
    }
  }
  else if (code == CONNECTION_PAIRING_PUBLIC_KEY){
    /// Received a public key
    struct __attribute__ ((packed)) ConnectionPairingPublicKey {
      uint8_t x[32];
      uint8_t y[32];
    } *connectionPairingPublicKey = (ConnectionPairingPublicKey*)l2capSignalingHdr->data;
    struct __attribute__ ((packed)) GenerateDHKeyCommand {
      uint8_t x[32];
      uint8_t y[32];
    } generateDHKeyCommand = {
      0x00,
      0x00,
    };
    memcpy(generateDHKeyCommand.x,connectionPairingPublicKey->x,32);
    memcpy(generateDHKeyCommand.y,connectionPairingPublicKey->y,32);
    struct __attribute__ ((packed)) ReadPublicKeyCommand {
      uint8_t code;
    } readPublicKeyCommand = {
      LE_COMMAND::READ_LOCAL_P256,
    };

    if(ATT.setPeerEncryption(connectionHandle, ATT.getPeerEncryption(connectionHandle) | PEER_ENCRYPTION::REQUESTED_ENCRYPTION)){
#ifdef _BLE_TRACE_
      Serial.println("[Info] Pairing public key");
      Serial.println("Requested encryption stored.");
#endif
    }else{
#ifdef _BLE_TRACE_
      Serial.println("[Info] Pairing public key");
      Serial.print("Failed to store encryption request with handle: 0x");
      Serial.println(connectionHandle,HEX);
#endif
    }
    
    memcpy(HCI.remotePublicKeyBuffer,&generateDHKeyCommand,sizeof(generateDHKeyCommand));
    HCI.sendCommand( (OGF_LE_CTL << 10 )| LE_COMMAND::READ_LOCAL_P256, 0);
  }
  else if(code == CONNECTION_PAIRING_DHKEY_CHECK)
  {
    uint8_t RemoteDHKeyCheck[16];
    uint8_t BD_ADDR_REMOTE[7];
    ATT.getPeerAddrWithType(connectionHandle, BD_ADDR_REMOTE);
    for(int i=0; i<16; i++) RemoteDHKeyCheck[15-i] = l2capSignalingHdr->data[i];
    uint8_t encryptionState = ATT.getPeerEncryption(connectionHandle) | PEER_ENCRYPTION::RECEIVED_DH_CHECK;


#ifdef _BLE_TRACE_
    Serial.println("[Info] DH Key check");
    Serial.print("Remote DHKey Check: ");
    btct.printBytes(RemoteDHKeyCheck, 16);
#endif

    HCI.readBdAddr();
    ATT.setPeerEncryption(connectionHandle, encryptionState);
    if((encryptionState & PEER_ENCRYPTION::DH_KEY_CALULATED) > 0){
      // We've already calculated the DHKey so we can calculate our check and send it.
      
      uint8_t MacKey[16];
      uint8_t localAddress[7];
      
      memcpy(&localAddress[1],HCI.localAddr,6);
      localAddress[0] = 0; // IOT 33 uses a static address

      btct.f5(HCI.DHKey,HCI.Na,HCI.Nb,BD_ADDR_REMOTE,localAddress,MacKey,HCI.LTK);

      uint8_t Ea[16];
      uint8_t Eb[16];
      uint8_t R[16];
      uint8_t MasterIOCap[3];
      uint8_t SlaveIOCap[3] = {LOCAL_AUTHREQ, 0x0, LOCAL_IOCAP};
      
      ATT.getPeerIOCap(connectionHandle, MasterIOCap);
      for(int i=0; i<16; i++) R[i] = 0;
      
      btct.f6(MacKey, HCI.Na,HCI.Nb,R, MasterIOCap, BD_ADDR_REMOTE, localAddress, Ea);
      btct.f6(MacKey, HCI.Nb,HCI.Na,R, SlaveIOCap, localAddress, BD_ADDR_REMOTE, Eb);


#ifdef _BLE_TRACE_
      Serial.println("Calculate f5, f6:");
      Serial.print("DH          : ");
      btct.printBytes(HCI.DHKey,32);
      Serial.println("Na        : ");
      btct.printBytes(HCI.Na,16);
      Serial.println("Nb        : ");
      btct.printBytes(HCI.Nb,16);
      Serial.print("MAC         : ");
      btct.printBytes(MacKey,16);
      // Serial.print("Expected MAC: ");
      // printBytes(EXPECTED_MAC, 16);
      Serial.print("LTK         : ");
      btct.printBytes(HCI.LTK,16);
      // Serial.print("Expected LTK: ");
      // printBytes(EXPECTED_LTK, 16);
      Serial.print("Expected Ex : ");
      btct.printBytes(RemoteDHKeyCheck, 16);
      Serial.print("Ea          : ");
      btct.printBytes(Ea, 16);
      Serial.print("Eb          : ");
      btct.printBytes(Eb,16);
      Serial.print("Local Addr  : ");
      btct.printBytes(localAddress, 7);
      Serial.print("LocalIOCap  : ");
      btct.printBytes(SlaveIOCap, 3);
      Serial.print("MasterAddr  : ");
      btct.printBytes(BD_ADDR_REMOTE, 7);
      Serial.print("MasterIOCAP : ");
      btct.printBytes(MasterIOCap, 3);
      Serial.println("Send Eb Back.");
#endif

      uint8_t ret[17];
      ret[0] = 0x0d;
      for(int i=0; i<sizeof(Eb); i++){
        ret[sizeof(Eb)-i] = Eb[i];
      }
      HCI.sendAclPkt(connectionHandle, 0x06, sizeof(ret), ret );
    }
  }
}

void L2CAPSignalingClass::removeConnection(uint8_t /*handle*/, uint16_t /*reason*/)
{
}

void L2CAPSignalingClass::setConnectionInterval(uint16_t minInterval, uint16_t maxInterval)
{
  _minInterval = minInterval;
  _maxInterval = maxInterval;
}

void L2CAPSignalingClass::setSupervisionTimeout(uint16_t supervisionTimeout)
{
  _supervisionTimeout = supervisionTimeout;
}

void L2CAPSignalingClass::connectionParameterUpdateRequest(uint16_t handle, uint8_t identifier, uint8_t dlen, uint8_t data[])
{
  struct __attribute__ ((packed)) L2CAPConnectionParameterUpdateRequest {
    uint16_t minInterval;
    uint16_t maxInterval;
    uint16_t latency;
    uint16_t supervisionTimeout;
  } *request = (L2CAPConnectionParameterUpdateRequest*)data;

  if (dlen < sizeof(L2CAPConnectionParameterUpdateRequest)) {
    // too short, ignore
    return;
  }

  struct __attribute__ ((packed)) L2CAPConnectionParameterUpdateResponse {
    uint8_t code;
    uint8_t identifier;
    uint16_t length;
    uint16_t value;
  } response = { CONNECTION_PARAMETER_UPDATE_RESPONSE, identifier, 2, 0x0000 };

  if (_minInterval && _maxInterval) {
    if (request->minInterval < _minInterval || request->maxInterval > _maxInterval) {
      response.value = 0x0001; // reject
    }
  }

  if  (_supervisionTimeout) {
    if (request->supervisionTimeout != _supervisionTimeout) {
      response.value = 0x0001; // reject
    }
  }

  HCI.sendAclPkt(handle, SIGNALING_CID, sizeof(response), &response);

  if (response.value == 0x0000) {
    HCI.leConnUpdate(handle, request->minInterval, request->maxInterval, request->latency, request->supervisionTimeout);
  }
}

void L2CAPSignalingClass::connectionParameterUpdateResponse(uint16_t /*handle*/, uint8_t /*identifier*/, uint8_t /*dlen*/, uint8_t /*data*/[])
{
}

#if !defined(FAKE_L2CAP)
L2CAPSignalingClass L2CAPSignalingObj;
L2CAPSignalingClass& L2CAPSignaling = L2CAPSignalingObj;
#endif
