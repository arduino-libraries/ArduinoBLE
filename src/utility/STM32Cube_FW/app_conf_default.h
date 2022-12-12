/**
  ******************************************************************************
  * @file    app_conf_default.h
  * @author  MCD Application Team
  * @brief   Default application configuration file for STM32WPAN Middleware.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
  #define BLE_SHARED_MEM_BYTE_ORDER  MSBFIRST
#endif
#define BLE_MODULE_SHARED_MEM_BUFFER_SIZE 128

/**
 * Define Tx Power
 */
#define CFG_TX_POWER                      (0x18) /* -0.15dBm */

/******************************************************************************
 * BLE Stack
 ******************************************************************************/
/**
 * Maximum number of simultaneous connections that the device will support.
 * Valid values are from 1 to 8
 */
#define CFG_BLE_NUM_LINK            8

/**
 * Maximum number of Services that can be stored in the GATT database.
 * Note that the GAP and GATT services are automatically added so this parameter should be 2 plus the number of user services
 */
#define CFG_BLE_NUM_GATT_SERVICES   8

/**
 * Maximum number of Attributes
 * (i.e. the number of characteristic + the number of characteristic values + the number of descriptors, excluding the services)
 * that can be stored in the GATT database.
 * Note that certain characteristics and relative descriptors are added automatically during device initialization
 * so this parameters should be 9 plus the number of user Attributes
 */
#define CFG_BLE_NUM_GATT_ATTRIBUTES 68

/**
 * Maximum supported ATT_MTU size
 * This parameter is ignored by the CPU2 when CFG_BLE_OPTIONS has SHCI_C2_BLE_INIT_OPTIONS_LL_ONLY flag set
 */
#define CFG_BLE_MAX_ATT_MTU             (156)

/**
 * Size of the storage area for Attribute values
 *  This value depends on the number of attributes used by application. In particular the sum of the following quantities (in octets) should be made for each attribute:
 *  - attribute value length
 *  - 5, if UUID is 16 bit; 19, if UUID is 128 bit
 *  - 2, if server configuration descriptor is used
 *  - 2*DTM_NUM_LINK, if client configuration descriptor is used
 *  - 2, if extended properties is used
 *  The total amount of memory needed is the sum of the above quantities for each attribute.
 * This parameter is ignored by the CPU2 when CFG_BLE_OPTIONS has SHCI_C2_BLE_INIT_OPTIONS_LL_ONLY flag set
 */
#define CFG_BLE_ATT_VALUE_ARRAY_SIZE    (1344)

/**
 * Prepare Write List size in terms of number of packet
 * This parameter is ignored by the CPU2 when CFG_BLE_OPTIONS has SHCI_C2_BLE_INIT_OPTIONS_LL_ONLY flag set
 */
// #define CFG_BLE_PREPARE_WRITE_LIST_SIZE         BLE_PREP_WRITE_X_ATT(CFG_BLE_MAX_ATT_MTU)
#define CFG_BLE_PREPARE_WRITE_LIST_SIZE         (0x3A)

/**
 * Number of allocated memory blocks
 * This parameter is overwritten by the CPU2 with an hardcoded optimal value when the parameter CFG_BLE_OPTIONS has SHCI_C2_BLE_INIT_OPTIONS_LL_ONLY flag set
 */
// #define CFG_BLE_MBLOCK_COUNT            (BLE_MBLOCKS_CALC(CFG_BLE_PREPARE_WRITE_LIST_SIZE, CFG_BLE_MAX_ATT_MTU, CFG_BLE_NUM_LINK))
#define CFG_BLE_MBLOCK_COUNT              (0x79)

/**
 * Enable or disable the Extended Packet length feature. Valid values are 0 or 1.
 */
#define CFG_BLE_DATA_LENGTH_EXTENSION   1

/**
 * Sleep clock accuracy in Slave mode (ppm value)
 */
#define CFG_BLE_SLAVE_SCA   500

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
#define CFG_BLE_MASTER_SCA   0

/**
 * LsSource
 * Some information for Low speed clock mapped in bits field
 * - bit 0:   1: Calibration for the RF system wakeup clock source   0: No calibration for the RF system wakeup clock source
 * - bit 1:   1: STM32WB5M Module device                             0: Other devices as STM32WBxx SOC, STM32WB1M module
 * - bit 2:   1: HSE/1024 Clock config                               0: LSE Clock config
 */
#if defined(STM32WB5Mxx)
  #define CFG_BLE_LS_SOURCE  (SHCI_C2_BLE_INIT_CFG_BLE_LS_NOCALIB | SHCI_C2_BLE_INIT_CFG_BLE_LS_MOD5MM_DEV | SHCI_C2_BLE_INIT_CFG_BLE_LS_CLK_LSE)
#else
  #define CFG_BLE_LS_SOURCE  (SHCI_C2_BLE_INIT_CFG_BLE_LS_NOCALIB | SHCI_C2_BLE_INIT_CFG_BLE_LS_OTHER_DEV | SHCI_C2_BLE_INIT_CFG_BLE_LS_CLK_LSE)
#endif

/**
 * Start up time of the high speed (16 or 32 MHz) crystal oscillator in units of 625/256 us (~2.44 us)
 */
#define CFG_BLE_HSE_STARTUP_TIME  0x148

/**
 * Maximum duration of the connection event when the device is in Slave mode in units of 625/256 us (~2.44 us)
 */
#define CFG_BLE_MAX_CONN_EVENT_LENGTH  (0xFFFFFFFF)

/**
 * Viterbi Mode
 * 1 : enabled
 * 0 : disabled
 */
#define CFG_BLE_VITERBI_MODE  1

/**
 * BLE stack Options flags to be configured with:
 * - SHCI_C2_BLE_INIT_OPTIONS_LL_ONLY
 * - SHCI_C2_BLE_INIT_OPTIONS_LL_HOST
 * - SHCI_C2_BLE_INIT_OPTIONS_NO_SVC_CHANGE_DESC
 * - SHCI_C2_BLE_INIT_OPTIONS_WITH_SVC_CHANGE_DESC
 * - SHCI_C2_BLE_INIT_OPTIONS_DEVICE_NAME_RO
 * - SHCI_C2_BLE_INIT_OPTIONS_DEVICE_NAME_RW
 * - SHCI_C2_BLE_INIT_OPTIONS_EXT_ADV
 * - SHCI_C2_BLE_INIT_OPTIONS_NO_EXT_ADV
 * - SHCI_C2_BLE_INIT_OPTIONS_CS_ALGO2
 * - SHCI_C2_BLE_INIT_OPTIONS_NO_CS_ALGO2
 * - SHCI_C2_BLE_INIT_OPTIONS_REDUC_GATTDB_NVM
 * - SHCI_C2_BLE_INIT_OPTIONS_FULL_GATTDB_NVM
 * - SHCI_C2_BLE_INIT_OPTIONS_GATT_CACHING_USED
 * - SHCI_C2_BLE_INIT_OPTIONS_GATT_CACHING_NOTUSED
 * - SHCI_C2_BLE_INIT_OPTIONS_POWER_CLASS_1
 * - SHCI_C2_BLE_INIT_OPTIONS_POWER_CLASS_2_3
 * - SHCI_C2_BLE_INIT_OPTIONS_APPEARANCE_WRITABLE
 * - SHCI_C2_BLE_INIT_OPTIONS_APPEARANCE_READONLY
 * - SHCI_C2_BLE_INIT_OPTIONS_ENHANCED_ATT_SUPPORTED
 * - SHCI_C2_BLE_INIT_OPTIONS_ENHANCED_ATT_NOTSUPPORTED
 * which are used to set following configuration bits:
 * (bit 0): 1: LL only
 *          0: LL + host
 * (bit 1): 1: no service change desc.
 *          0: with service change desc.
 * (bit 2): 1: device name Read-Only
 *          0: device name R/W
 * (bit 3): 1: extended advertizing supported
 *          0: extended advertizing not supported
 * (bit 4): 1: CS Algo #2 supported
 *          0: CS Algo #2 not supported
 * (bit 5): 1: Reduced GATT database in NVM
 *          0: Full GATT database in NVM
 * (bit 6): 1: GATT caching is used
 *          0: GATT caching is not used
 * (bit 7): 1: LE Power Class 1
 *          0: LE Power Class 2-3
 * (bit 8): 1: appearance Writable
 *          0: appearance Read-Only
 * (bit 9): 1: Enhanced ATT supported
 *          0: Enhanced ATT not supported
 * other bits: reserved (shall be set to 0)
 */
#define CFG_BLE_OPTIONS  (SHCI_C2_BLE_INIT_OPTIONS_LL_ONLY)

#define CFG_BLE_MAX_COC_INITIATOR_NBR   (32)

#define CFG_BLE_MIN_TX_POWER            (-40)

#define CFG_BLE_MAX_TX_POWER            (6)

/**
 * BLE Rx model configuration flags to be configured with:
 * - SHCI_C2_BLE_INIT_RX_MODEL_AGC_RSSI_LEGACY
 * - SHCI_C2_BLE_INIT_RX_MODEL_AGC_RSSI_BLOCKER
 * which are used to set following configuration bits:
 * (bit 0): 1: agc_rssi model improved vs RF blockers
 *          0: Legacy agc_rssi model
 * other bits: reserved (shall be set to 0)
 */

#define CFG_BLE_RX_MODEL_CONFIG         (SHCI_C2_BLE_INIT_RX_MODEL_AGC_RSSI_LEGACY)

/* Maximum number of advertising sets.
 * Range: 1 .. 8 with limitation:
 * This parameter is linked to CFG_BLE_MAX_ADV_DATA_LEN such as both compliant with allocated Total memory computed with BLE_EXT_ADV_BUFFER_SIZE based
 * on Max Extended advertising configuration supported.
 * This parameter is considered by the CPU2 when CFG_BLE_OPTIONS has SHCI_C2_BLE_INIT_OPTIONS_EXT_ADV flag set
 */

#define CFG_BLE_MAX_ADV_SET_NBR     (8)

 /* Maximum advertising data length (in bytes)
 * Range: 31 .. 1650 with limitation:
 * This parameter is linked to CFG_BLE_MAX_ADV_SET_NBR such as both compliant with allocated Total memory computed with BLE_EXT_ADV_BUFFER_SIZE based
 * on Max Extended advertising configuration supported.
 * This parameter is considered by the CPU2 when CFG_BLE_OPTIONS has SHCI_C2_BLE_INIT_OPTIONS_EXT_ADV flag set
 */

#define CFG_BLE_MAX_ADV_DATA_LEN    (207)

 /* RF TX Path Compensation Value (16-bit signed integer). Units: 0.1 dB.
  * Range: -1280 .. 1280
  */

#define CFG_BLE_TX_PATH_COMPENS    (0)

 /* RF RX Path Compensation Value (16-bit signed integer). Units: 0.1 dB.
  * Range: -1280 .. 1280
  */

#define CFG_BLE_RX_PATH_COMPENS    (0)

  /* BLE core version (16-bit signed integer).
   * - SHCI_C2_BLE_INIT_BLE_CORE_5_2
   * - SHCI_C2_BLE_INIT_BLE_CORE_5_3
   * which are used to set: 11(5.2), 12(5.3).
   */

#define CFG_BLE_CORE_VERSION   (SHCI_C2_BLE_INIT_BLE_CORE_5_3)

#endif /* APP_CONF_DEFAULT_H */
