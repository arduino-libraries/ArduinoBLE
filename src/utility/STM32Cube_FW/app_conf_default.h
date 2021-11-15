/**
 ******************************************************************************
 * File Name          : app_conf_default.h
 * Description        : Default application configuration file for STM32WPAN Middleware.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef APP_CONF_DEFAULT_H
#define APP_CONF_DEFAULT_H

/******************************************************************************
 * Application Config
 ******************************************************************************/

/**< generic parameters ******************************************************/
/* HCI related defines */

#define ACI_HAL_SET_TX_POWER_LEVEL 0xFC0F
#define ACI_WRITE_CONFIG_DATA_OPCODE 0xFC0C
#define ACI_READ_CONFIG_DATA_OPCODE 0xFC0D
#define MAX_HCI_ACL_PACKET_SIZE (sizeof(TL_PacketHeader_t) + 5 + 251)
#define HCI_RESET 0x0C03

#ifndef BLE_SHARED_MEM_BYTE_ORDER
#define BLE_SHARED_MEM_BYTE_ORDER MSBFIRST
#endif
#define BLE_MODULE_SHARED_MEM_BUFFER_SIZE 128

/**
 * Define Tx Power
 */
#ifndef CFG_TX_POWER
#define CFG_TX_POWER (0x18) /* -0.15dBm */
#endif

/******************************************************************************
 * BLE Stack
 ******************************************************************************/
/**
 * Maximum number of simultaneous connections that the device will support.
 * Valid values are from 1 to 8
 */
#ifndef CFG_BLE_NUM_LINK
#define CFG_BLE_NUM_LINK 8
#endif

/**
 * Maximum number of Services that can be stored in the GATT database.
 * Note that the GAP and GATT services are automatically added so this parameter should be 2 plus the number of user
 * services
 */
#ifndef CFG_BLE_NUM_GATT_SERVICES
#define CFG_BLE_NUM_GATT_SERVICES 8
#endif

/**
 * Maximum number of Attributes
 * (i.e. the number of characteristic + the number of characteristic values + the number of descriptors, excluding the
 * services) that can be stored in the GATT database. Note that certain characteristics and relative descriptors are
 * added automatically during device initialization so this parameters should be 9 plus the number of user Attributes
 */
#ifndef CFG_BLE_NUM_GATT_ATTRIBUTES
#define CFG_BLE_NUM_GATT_ATTRIBUTES 68
#endif

/**
 * Maximum supported ATT_MTU size
 */
#ifndef CFG_BLE_MAX_ATT_MTU
#define CFG_BLE_MAX_ATT_MTU (156)
#endif

/**
 * Size of the storage area for Attribute values
 *  This value depends on the number of attributes used by application. In particular the sum of the following
 * quantities (in octets) should be made for each attribute:
 *  - attribute value length
 *  - 5, if UUID is 16 bit; 19, if UUID is 128 bit
 *  - 2, if server configuration descriptor is used
 *  - 2*DTM_NUM_LINK, if client configuration descriptor is used
 *  - 2, if extended properties is used
 *  The total amount of memory needed is the sum of the above quantities for each attribute.
 */
#ifndef CFG_BLE_ATT_VALUE_ARRAY_SIZE
#define CFG_BLE_ATT_VALUE_ARRAY_SIZE (1344)
#endif

/**
 * Prepare Write List size in terms of number of packet
 */
//#define CFG_BLE_PREPARE_WRITE_LIST_SIZE         BLE_PREP_WRITE_X_ATT(CFG_BLE_MAX_ATT_MTU)
#ifndef CFG_BLE_PREPARE_WRITE_LIST_SIZE
#define CFG_BLE_PREPARE_WRITE_LIST_SIZE (0x3A)
#endif

/**
 * Number of allocated memory blocks
 */
//#define CFG_BLE_MBLOCK_COUNT            (BLE_MBLOCKS_CALC(CFG_BLE_PREPARE_WRITE_LIST_SIZE, CFG_BLE_MAX_ATT_MTU,
// CFG_BLE_NUM_LINK))
#ifndef CFG_BLE_MBLOCK_COUNT
#define CFG_BLE_MBLOCK_COUNT (0x79)
#endif

/**
 * Enable or disable the Extended Packet length feature. Valid values are 0 or 1.
 */
#ifndef CFG_BLE_DATA_LENGTH_EXTENSION
#define CFG_BLE_DATA_LENGTH_EXTENSION 1
#endif

/**
 * Sleep clock accuracy in Slave mode (ppm value)
 */
#ifndef CFG_BLE_SLAVE_SCA
#define CFG_BLE_SLAVE_SCA 500
#endif

/**
 * Sleep clock accuracy in Master mode
 * 0 : 251 ppm to 500 ppm
 * 1 : 151 ppm to 250 ppm
 * 2 : 101 ppm to 150 ppm
 * 3 : 76 ppm to 100 ppm
 * 4 : 51 ppm to 75 ppm
 * 5 : 31 ppm to 50 ppm
 * 6 : 21 ppm to 30 ppm
 * 7 : 0 ppm to 20 ppm
 */
#ifndef CFG_BLE_MASTER_SCA
#define CFG_BLE_MASTER_SCA 0
#endif

/**
 *  Source for the 32 kHz slow speed clock
 *  1 : internal RO
 *  0 : external crystal ( no calibration )
 */
#ifndef CFG_BLE_LSE_SOURCE
#define CFG_BLE_LSE_SOURCE 0
#endif

/**
 * Start up time of the high speed (16 or 32 MHz) crystal oscillator in units of 625/256 us (~2.44 us)
 */
#ifndef CFG_BLE_HSE_STARTUP_TIME
#define CFG_BLE_HSE_STARTUP_TIME 0x148
#endif

/**
 * Maximum duration of the connection event when the device is in Slave mode in units of 625/256 us (~2.44 us)
 */
#ifndef CFG_BLE_MAX_CONN_EVENT_LENGTH
#define CFG_BLE_MAX_CONN_EVENT_LENGTH (0xFFFFFFFF)
#endif

/**
 * Viterbi Mode
 * 1 : enabled
 * 0 : disabled
 */
#define CFG_BLE_VITERBI_MODE 1

/**
 *  LL Only Mode
 *  1 : LL Only
 *  0 : LL + Host
 */
#define CFG_BLE_LL_ONLY 1

#endif /* APP_CONF_DEFAULT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
