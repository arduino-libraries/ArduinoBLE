##########################################################################

set(CMAKE_VERBOSE_MAKEFILE ON)
cmake_minimum_required(VERSION 2.8)

##########################################################################

project(testArduinoBLE)

##########################################################################

include_directories(include)
include_directories(include/util)
include_directories(../../src)
include_directories(../../src/local)
include_directories(../../src/remote)
include_directories(../../src/utility)
include_directories(external/catch/v2.12.1/include)
include_directories(external/fakeit/v2.0.5/include)

##########################################################################

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

##########################################################################

set(TEST_TARGET_UUID_SRCS
  src/Arduino.cpp
  src/util/itoa.c
  src/util/TestUtil.cpp
  src/test_main.cpp
  src/test_uuid/test_uuid.cpp
  ../../src/utility/BLEUuid.cpp
  src/util/String.cpp
  src/util/Common.cpp
)

set(TEST_TARGET_DISC_DEVICE_SRCS
  src/Arduino.cpp
  src/util/itoa.c
  src/util/TestUtil.cpp
  src/util/String.cpp
  src/util/Common.cpp
  src/test_main.cpp
  src/test_discovered_device/test_discovered_device.cpp
  ../../src/utility/BLEUuid.cpp
  ../../src/BLEDevice.cpp
  ../../src/BLECharacteristic.cpp
  ../../src/BLEDescriptor.cpp
  ../../src/BLEService.cpp
  ../../src/BLEAdvertisingData.cpp
  ../../src/utility/ATT.cpp
  #../../src/utility/GAP.cpp
  ../../src/utility/HCI.cpp
  ../../src/utility/GATT.cpp
  ../../src/utility/L2CAPSignaling.cpp
  ../../src/local/BLELocalAttribute.cpp
  ../../src/local/BLELocalCharacteristic.cpp
  ../../src/local/BLELocalDescriptor.cpp
  ../../src/local/BLELocalDevice.cpp
  ../../src/local/BLELocalService.cpp
  ../../src/remote/BLERemoteAttribute.cpp
  ../../src/remote/BLERemoteAdvertisingData.cpp
  ../../src/remote/BLERemoteCharacteristic.cpp
  ../../src/remote/BLERemoteDescriptor.cpp
  ../../src/remote/BLERemoteDevice.cpp
  ../../src/remote/BLERemoteService.cpp
  ../../src/BLEStringCharacteristic.cpp
  ../../src/BLETypedCharacteristics.cpp
  # Fake classes 
  src/util/HCIFakeTransport.cpp
  src/test_discovered_device/FakeGAP.cpp
)

##########################################################################

set(CMAKE_C_FLAGS   ${CMAKE_C_FLAGS}   "--coverage")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "--coverage")

##########################################################################

add_executable(TEST_TARGET_UUID ${TEST_TARGET_UUID_SRCS})
add_executable(TEST_TARGET_DISC_DEVICE ${TEST_TARGET_DISC_DEVICE_SRCS})

##########################################################################

# Build unit tests as a post build step
add_custom_command(TARGET TEST_TARGET_UUID POST_BUILD
  COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/TEST_TARGET_UUID
)
add_custom_command(TARGET TEST_TARGET_DISC_DEVICE POST_BUILD
  COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/TEST_TARGET_DISC_DEVICE
)
