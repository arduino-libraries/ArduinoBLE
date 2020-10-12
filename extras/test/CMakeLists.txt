##########################################################################

set(CMAKE_VERBOSE_MAKEFILE ON)
cmake_minimum_required(VERSION 2.8)

##########################################################################

project(testArduinoBLE)

##########################################################################

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

##########################################################################

set(COMMON_TEST_SRCS
  src/test_main.cpp
  src/Arduino.cpp
  src/util/itoa.c
  src/util/TestUtil.cpp
  src/util/String.cpp
  src/util/Common.cpp
)

set(DUT_SRCS
  ../../src/utility/BLEUuid.cpp
  ../../src/BLEDevice.cpp
  ../../src/BLECharacteristic.cpp
  ../../src/BLEDescriptor.cpp
  ../../src/BLEService.cpp
  ../../src/BLEAdvertisingData.cpp
  ../../src/utility/ATT.cpp
  ../../src/utility/GAP.cpp
  ../../src/utility/HCI.cpp
  ../../src/utility/GATT.cpp
  ../../src/utility/L2CAPSignaling.cpp
  ../../src/local/BLELocalAttribute.cpp
  ../../src/local/BLELocalCharacteristic.cpp
  ../../src/local/BLELocalDescriptor.cpp
  ../../src/local/BLELocalDevice.cpp
  ../../src/local/BLELocalService.cpp
  ../../src/remote/BLERemoteAttribute.cpp
  ../../src/remote/BLERemoteCharacteristic.cpp
  ../../src/remote/BLERemoteDescriptor.cpp
  ../../src/remote/BLERemoteDevice.cpp
  ../../src/remote/BLERemoteService.cpp
  ../../src/BLEStringCharacteristic.cpp
  ../../src/BLETypedCharacteristics.cpp
)

set(TEST_TARGET_UUID_SRCS
  # Test files
  ${COMMON_TEST_SRCS}
  src/test_uuid/test_uuid.cpp
  # DUT files
  #${DUT_SRCS}
  ../../src/utility/BLEUuid.cpp
)

set(TEST_TARGET_DISC_DEVICE_SRCS
  # Test files
  ${COMMON_TEST_SRCS}
  src/test_discovered_device/test_discovered_device.cpp
  # DUT files
  ${DUT_SRCS}
  # Fake classes files
  src/util/HCIFakeTransport.cpp
  src/test_discovered_device/FakeGAP.cpp
)

set(TEST_TARGET_ADVERTISING_DATA_SRCS
  # Test files
  ${COMMON_TEST_SRCS}
  src/test_advertising_data/test_advertising_data.cpp
  src/test_advertising_data/test_service.cpp
  src/test_advertising_data/test_local_name.cpp
  src/test_advertising_data/test_manufacturer.cpp
  # DUT files
  ${DUT_SRCS}
  # Fake classes files
  src/util/HCIFakeTransport.cpp
  src/test_advertising_data/FakeBLELocalDevice.cpp
)

##########################################################################

set(CMAKE_C_FLAGS   ${CMAKE_C_FLAGS}   "--coverage")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "--coverage")

##########################################################################

add_executable(TEST_TARGET_UUID ${TEST_TARGET_UUID_SRCS})
add_executable(TEST_TARGET_DISC_DEVICE ${TEST_TARGET_DISC_DEVICE_SRCS})
add_executable(TEST_TARGET_ADVERTISING_DATA ${TEST_TARGET_ADVERTISING_DATA_SRCS})

##########################################################################

include_directories(include)
include_directories(include/util)
include_directories(../../src)
include_directories(../../src/local)
include_directories(../../src/remote)
include_directories(../../src/utility)
include_directories(external/catch/v2.12.1/include)

target_include_directories(TEST_TARGET_DISC_DEVICE PUBLIC include/test_discovered_device)
target_include_directories(TEST_TARGET_ADVERTISING_DATA PUBLIC include/test_advertising_data)

##########################################################################

target_compile_definitions(TEST_TARGET_DISC_DEVICE PUBLIC FAKE_GAP)
target_compile_definitions(TEST_TARGET_ADVERTISING_DATA PUBLIC FAKE_BLELOCALDEVICE)

##########################################################################

# Build unit tests as a post build step
add_custom_command(TARGET TEST_TARGET_UUID POST_BUILD
  COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/TEST_TARGET_UUID
)
add_custom_command(TARGET TEST_TARGET_DISC_DEVICE POST_BUILD
  COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/TEST_TARGET_DISC_DEVICE
)
add_custom_command(TARGET TEST_TARGET_ADVERTISING_DATA POST_BUILD
  COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/TEST_TARGET_ADVERTISING_DATA
)
