/*
  This file is part of the STM32duinoBLE library.
  Copyright (c) 2019 STMicroelectronics. All rights reserved.

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
#if defined(STM32WBxx)

#include "HCISharedMemTransport.h"
#include "STM32_WPAN/hw.h"
#include "otp.h"

/* Private variables ---------------------------------------------------------*/
PLACE_IN_SECTION("MB_MEM1") ALIGN(4) static TL_CmdPacket_t BleCmdBuffer;

PLACE_IN_SECTION("MB_MEM1") ALIGN(4) static uint8_t EvtPool[POOL_SIZE];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t SystemCmdBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t
SystemSpareEvtBuffer[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t
BleSpareEvtBuffer[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t
HciAclDataBuffer[sizeof(TL_PacketHeader_t) + 5 + 251];


/* global var used as semaphore to control incoming events */
volatile bool sys_event; /* true : M0 core is already up and running */
volatile bool acl_data_on; /* true : sending ACL data in progress, false : send is possible */
volatile bool data_overflow;

/* buffer to store the received packets */
volatile uint8_t _rxbuff[BLE_MODULE_SHARED_MEM_BUFFER_SIZE];
volatile uint16_t _read_index; /* fifo position when reading */
volatile uint16_t _write_index; /* fifo position when receiving */

/* var of different device steps during init and receiving  */
volatile bool phase_bd_addr;
volatile bool phase_tx_power;
volatile bool phase_reset;
volatile bool phase_running;


/** Bluetooth Device Address */
static uint8_t bd_addr_udn[CONFIG_DATA_PUBADDR_LEN];

/* Private functions ---------------------------------------------------------*/
/**
 * TL Mailbox synchronisation means
 */

/*  returns true if sys_event was received, false otherwise */
static bool sysevt_wait(void)
{
  /* sys_event remains false until event is received */
  for (unsigned long start = millis(); (millis() - start) < BLE_IPCC_TIMEOUT;) {
    /* Wait for 10sec max - if not return an error */
    if (sys_event) {
      break;
    }
  }

  if (!sys_event) {
#if defined(PRINT_IPCC_INFO)
    printf("ERROR: sys_evt timeout\r\n");
#endif /*(PRINT_IPCC_INFO)*/
    /*  no event received, timeout occurs */
    return false;
  }
  /*  release immediately, now that M0 runs */
  return true;
}

/*  WEAK callbacks from the BLE TL driver - will be called under Interrupt */
static void sysevt_received(void *pdata)
{
  UNUSED(pdata);
  /* For now only READY event is received, so we know this is it */
  __disable_irq();
  sys_event = true;
  __enable_irq();
  /* But later on ... we'll have to parse the answer */
}

/*  returns true if sysevt was already received, which means M0 core is
 *  already up and running */
static bool sysevt_check(void)
{
  /*  Check if system is UP and running already */
  for (unsigned long start = millis(); (millis() - start) < 10;) {
    /* Wait for 10ms max - if not return an error */
    if (sys_event) {
      break;
    }
  }
  if (sys_event) {
    /*  release immediately as M0 already runs */
    return true;
  }
  return false;
}

static void acl_data_ack(void)
{
  /**
   * The current implementation assumes the taskGUI will not send a new HCI ACL DATA packet before this ack is received
   * ( which means the CPU2 has handled the previous packet )
   * In order to implement a secure mechanism, it is required either
   * - a flow control with the stack
   * - a local pool of buffer to store packets received from the stack
   */
  __disable_irq();
  acl_data_on = false;
  __enable_irq();
}

static bool acl_data_wait(void)
{
  /* Wait 10 sec for previous ACL command to be ack'ed by Low Layers
   * before sending the next one */
  for (unsigned long start = millis(); (millis() - start) < BLE_IPCC_TIMEOUT;) {
    /* Wait for 10sec max - if not return an error */
    if (!acl_data_on) {
      break;
    }
  }
  if (acl_data_on) {
    /*  no event received, timeout occurs */
#if defined(PRINT_IPCC_INFO)
    printf("ERROR: acl data timeout\r\n");
#endif /*(PRINT_IPCC_INFO)*/
    return false;
  }
  /*  release immediately, now that M0 runs */
  __disable_irq();
  acl_data_on = false;
  __enable_irq();
  return true;
}

static void syscmd_status_not(SHCI_TL_CmdStatus_t status)
{
#if defined(PRINT_IPCC_INFO)
  printf("syscmd_status_not, status:%d\r\n", status);
#else
  UNUSED(status);
#endif /*(PRINT_IPCC_INFO)*/
}

/* to received BLE packet from the SharedMem */
void evt_received(TL_EvtPacket_t *hcievt)
{
  uint16_t len = 0;

  /* We need to memcpy the data before passing to higher layers.
   * The received packet is copied in the _rxbuff
   * but it must not exceed the BLE_MODULE_SHARED_MEM_BUFFER_SIZE
   */
  switch (hcievt->evtserial.type) {
    case TL_BLEEVT_PKT_TYPE: {
        /* before starting the running_phase', a RESET command (0x0C03) is sent
         * by the HCI. Then this evt packet is temporarily kept in the _rxbuff
         * the set_bd_address (0xFC0C) and the set_tw_power (0xFC0F) commands are sent.
         * Only when both evt are received (not store in the _rxbuffer),
         * the Reset packet is handled at HCI layer : the running_phase begins
         */
        if (phase_running == false) {
          /* check the Rx event of complete the previous bd_addr or random address opcode 0xFC0C */
          if ((hcievt->evtserial.evt.evtcode == TL_BLEEVT_CC_OPCODE) &&
              (hcievt->evtserial.evt.payload[0] == 0x01) &&
              (hcievt->evtserial.evt.payload[1] == 0x0C) &&
              (hcievt->evtserial.evt.payload[2] == 0xFC)) {
            /* First setting must be global address
             */
            phase_bd_addr = true;

            if (hcievt->evtserial.evt.payload[3] != 0) {
#if defined(PRINT_IPCC_INFO)
              printf("Error: wrong BD Addr\r\n");
#endif /*(PRINT_IPCC_INFO)*/
            }
            /* rx data is no more useful : not stored in the _rxbuff */
            break;
          }
          /* check the Rx event of complete the previous tx power opcode 0xFC0F */
          if ((hcievt->evtserial.evt.evtcode == TL_BLEEVT_CC_OPCODE) &&
              (hcievt->evtserial.evt.payload[0] == 0x01) &&
              (hcievt->evtserial.evt.payload[1] == 0x0F) &&
              (hcievt->evtserial.evt.payload[2] == 0xFC)) {
            phase_tx_power = true;
            if (hcievt->evtserial.evt.payload[3] != 0) {
#if defined(PRINT_IPCC_INFO)
              printf("Error: wrong Tx power\r\n");
#endif /*(PRINT_IPCC_INFO)*/
            }
            /* rx data is no more useful : not stored in the _rxbuff */
            break;
          }

          /* check if the reset phase is in progress (opcode is 0x0C03) */
          if ((hcievt->evtserial.evt.evtcode == TL_BLEEVT_CC_OPCODE) &&
              (hcievt->evtserial.evt.payload[0] == 0x01) &&
              (hcievt->evtserial.evt.payload[1] == 0x03) &&
              (hcievt->evtserial.evt.payload[2] == 0x0C)) {
            phase_reset = true;
#if defined(PRINT_IPCC_INFO)
            if (hcievt->evtserial.evt.payload[3] != 0) {
              printf("Error: wrong reset\r\n");
            }
#endif /*(PRINT_IPCC_INFO)*/
          }
        }
        __disable_irq();
        /* store received data in the _rxbuff buffer */
        len = hcievt->evtserial.evt.plen + TL_EVT_HDR_SIZE;
        if (len < BLE_MODULE_SHARED_MEM_BUFFER_SIZE - _write_index) {
          /* at the position of the _write_index */
          memcpy((uint8_t *)&_rxbuff[_write_index], (uint8_t *)&hcievt->evtserial, len);
          /* move index */
          _write_index += len;
        } else {
          data_overflow = true;
        }
        __enable_irq();
      }
      break;
    case TL_ACL_DATA_PKT_TYPE: {
        TL_AclDataSerial_t *acl = &(((TL_AclDataPacket_t *)hcievt)->AclDataSerial);
        __disable_irq();
        len = acl->length + 5;
        if (len < BLE_MODULE_SHARED_MEM_BUFFER_SIZE - _write_index) {
          memcpy((uint8_t *)&_rxbuff[_write_index], (uint8_t *)acl, len);
          /* move index */
          _write_index += len;
        } else {
          data_overflow = true;
        }
        __enable_irq();
      }
      break;
    default:
      /* should not happen */
#if defined(PRINT_IPCC_INFO)
      printf("BLE TL evt_received, wrong type:%d\r\n", hcievt->evtserial.type);
      while (1); /* let's block to check */
#endif /*(PRINT_IPCC_INFO)*/
      break;
  }
#if defined(PRINT_IPCC_INFO)
  if (data_overflow) {
    printf("Error: data read overflow\r\n");
  }
#endif /*(PRINT_IPCC_INFO)*/

  /*  In case Event belongs to the Evt Pool we need to inform  */
  if (((uint8_t *)hcievt >= EvtPool) && ((uint8_t *)hcievt < (EvtPool + POOL_SIZE))) {
    /*  Free the message from shared memory */
    TL_MM_EvtDone(hcievt);
  }
}

/* to send BLE packet to the SharedMem : return nb of bytes actually written */
uint16_t mbox_write(uint8_t type, uint16_t len, const uint8_t *pData)
{
  TL_CmdPacket_t *bleCmdBuf = &BleCmdBuffer;
  // Note: Until enum is available
  // type 01 Command
  // type 02 ACL DATA
  // type 03 SCO Voice (not supported)
  // type 04 event - uplink (not supported)
  switch (type) {
    case 1: { //BLE command
        bleCmdBuf->cmdserial.type = type; // for now this param is overwritten in TL_BLE_SendCmd
        bleCmdBuf->cmdserial.cmd.plen = len;
        memcpy((void *) &bleCmdBuf->cmdserial.cmd, pData, len);
        /* We're tracing here the command, after copy in shared mem but before M0 trigger. */
        TL_BLE_SendCmd(NULL, 0); // unused parameters for now
      }
      break;
    case 2: { //ACL DATA
        if (!acl_data_wait()) {
#if defined(PRINT_IPCC_INFO)
          printf("ERROR: previous ACL message not ACK'd\r\n");
#endif /*(PRINT_IPCC_INFO)*/
          /*  return number of bytes sent, 0 in this error case */
          return 0;
        }
        TL_AclDataSerial_t *aclDataSerial = (TL_AclDataSerial_t *)(HciAclDataBuffer + sizeof(TL_PacketHeader_t));
        aclDataSerial->type = type; // for now this param is overwritten in TL_BLE_SendCmd
        memcpy(HciAclDataBuffer + sizeof(TL_PacketHeader_t) + sizeof(type), pData, len);
        TL_BLE_SendAclData(NULL, 0); // unused parameters for now
        __disable_irq();
        acl_data_on = true; /* data being send */
        __enable_irq();
      }
      break;
    default:
#if defined(PRINT_IPCC_INFO)
      printf("ERROR: not supported type\r\n");
#endif /*(PRINT_IPCC_INFO)*/
      /*  return number of bytes sent, 0 in this error case */
      len = 0;
      break;
  }
  return len;
}

/**
 * Few utilities functions
 */
/* This function fills in a BD address table */
static bool get_bd_address(uint8_t *bd_addr)
{
  uint32_t udn;
  uint32_t company_id;
  uint32_t device_id;
  bool bd_found;

  udn = LL_FLASH_GetUDN();

  if (udn != 0xFFFFFFFF) {
    /* "Found Unique Device Number: %#06x", udn) */

    company_id = LL_FLASH_GetSTCompanyID();
    device_id = LL_FLASH_GetDeviceID();

    bd_addr[0] = (uint8_t)(udn & 0x000000FF);
    bd_addr[1] = (uint8_t)((udn & 0x0000FF00) >> 8);
    bd_addr[2] = (uint8_t)device_id;
    bd_addr[3] = (uint8_t)(company_id & 0x000000FF);
    bd_addr[4] = (uint8_t)((company_id & 0x0000FF00) >> 8);
    bd_addr[5] = (uint8_t)((company_id & 0x00FF0000) >> 16);

    bd_found = true;
  } else {
    OTP_BT_t *p_otp = (OTP_BT_t *)OTP_Read(0);
    if (p_otp) {
      memcpy(bd_addr, p_otp->bd_address, CONFIG_DATA_PUBADDR_LEN);
      bd_found = true;
    } else {
      bd_found = false;
    }
  }

  return bd_found;
}

static void init_debug(void)
{
  /* In case of debug profile, configure debugger support */

#if defined(CONFIG_DEBUG)
#if defined(PRINT_IPCC_INFO)
  printf("init_debug ENABLED\r\n");
#endif /*(PRINT_IPCC_INFO)*/
  /**
   * Keep debugger enabled while in any low power mode
   */
  HAL_DBGMCU_EnableDBGSleepMode();
  HAL_DBGMCU_EnableDBGStopMode();
  HAL_DBGMCU_EnableDBGStandbyMode();

  /* Enable debugger: Debug power up request wakeup EXTI line 48 */
  LL_EXTI_EnableIT_32_63(LL_EXTI_LINE_48);
  LL_C2_EXTI_EnableIT_32_63(LL_EXTI_LINE_48);

#endif /* CONFIG_DEBUG */
}

/* Class definition ----------------------------------------------------------*/

HCISharedMemTransportClass::HCISharedMemTransportClass()
{
  _read_index = 0; /* fifo position when reading */
  _write_index = 0; /* fifo position when receiving */

  memset((void *)_rxbuff, 0, sizeof(_rxbuff));

  sys_event = false;
  acl_data_on = false;

  data_overflow = false;

  phase_bd_addr = false;
  phase_tx_power = false;
  phase_reset = false;
  phase_running = false;
}

HCISharedMemTransportClass::~HCISharedMemTransportClass()
{
}

int HCISharedMemTransportClass::begin()
{
  int status = 1;
  /* clean data Rx variables */
  _read_index = 0;
  _write_index = 0;

  memset((void *)_rxbuff, 0, sizeof(_rxbuff));

  /*  Check whether M0 sub-system was started already by
   *  checking if the system event was already received
   *  before. If it was not, then go thru all init. */
  if (!sysevt_check()) {
    start_ble_rf();
    init_debug();
    /* Take BLE out of reset */
    stm32wb_reset();
    /* "C2 unlocking" */
    transport_init();
    /*  At this stage, we got the ready event,
     *  passed thru TL_SYS_EvtReceived */

    WirelessFwInfo_t wireless_info_instance;
    WirelessFwInfo_t *p_wireless_info = &wireless_info_instance;
    SHCI_GetWirelessFwInfo(p_wireless_info);
#if defined(PRINT_IPCC_INFO)
    printf("WB copro FW version = %d.%d.%d\r\n", p_wireless_info->VersionMajor, p_wireless_info->VersionMinor, p_wireless_info->VersionSub);
#endif /*(PRINT_IPCC_INFO)*/

    /* Now start BLE service on firmware side, using Vendor specific
     * command on the System Channel
     */
    status = stm32wb_start_ble();

    /* Once reset complete event is received we will need
     * to send a few more commands:
     * set bd addr with bt_ipm_set_addr();
     * during the HCI rest command */
  }
  /* IPM Channel is now open */
#if defined(PRINT_IPCC_INFO)
  printf("IPM Channel Open Completed\r\n");
#endif /*(PRINT_IPCC_INFO)*/

  return status;
}

void HCISharedMemTransportClass::end()
{
  /* M0 sub-system is already on (sys_event) */
  acl_data_on = false;
  data_overflow = false;

  /* the HCI RESET command ready to be processed again */
  phase_bd_addr = false;
  phase_tx_power = false;
  phase_reset = false;
  phase_running = false;
}

void HCISharedMemTransportClass::wait(unsigned long timeout)
{
  for (unsigned long start = millis(); (millis() - start) < timeout;) {
    if (available()) {
      break;
    }
  }
}

int HCISharedMemTransportClass::available()
{
  /* assuming the reset is already achieved,
   * the LL-only mode is already configured. */

  if (_read_index != _write_index) {
    return 1;
  } else if (data_overflow) {
    __disable_irq();
    data_overflow = false;
    __enable_irq();
    if (_read_index != _write_index) {
      return 1;
    }
  }

  return 0;
}

int HCISharedMemTransportClass::peek()
{
  int peek_val = -1;
  __disable_irq();
  if (_read_index != _write_index) {
    peek_val = _rxbuff[_read_index];
  }
  __enable_irq();
  return peek_val;
}

int HCISharedMemTransportClass::read()
{
  int read_val = -1;
  __disable_irq();
  if (_read_index != _write_index) {
    read_val = _rxbuff[_read_index];
    _read_index++;
    if (_read_index == _write_index) {
      /* Reset buffer index */
      _read_index = 0;
      _write_index = 0;
    }
  }
  __enable_irq();
  return read_val;
}

size_t HCISharedMemTransportClass::write(const uint8_t *data, size_t length)
{
  const uint8_t *msg_data;
  msg_data = &data[1];

  /* capture the HCI reset send command opcode = 0x0C03
   * After HCI reset event complete in the evt_received(),
   * the bd_addr and tx_power must be sent
   * before the phase_running begins.
   */
  if (phase_running) {
    return mbox_write(data[0], (length - 1), msg_data);;
  }
  if ((data[1] == 0x03) && (data[2] == 0x0C)) {
    phase_reset = false;

    if (mbox_write(data[0], (length - 1), msg_data) != (length - 1)) {
      /* Error: no data are written */
      return 0;
    }
    /* capture event after HCI_RESET */
    while (!phase_reset);

    /* set the bd add */
    if (!bt_ipm_set_addr()) {
      /* in case of error, no data are written */
      return 0;
    }
    /* wait for the Rx complete */
    while (!phase_bd_addr);
    /* this sequence is now complete */

    /* set the Tx power */
    bt_ipm_set_power();
    /* wait for the Rx complete */
    while (!phase_tx_power);

    /* this sequence is now complete */
    phase_running = true;

    return (length - 1); /* mbox_size of the HCI reset command */
  }
  return 0; /* Error: no data written */
}

//private:
void HCISharedMemTransportClass::start_ble_rf(void)
{
  /* Set the DBP bit in the Power control register 1 (PWR_CR1) */
  LL_PWR_EnableBkUpAccess();

  /* LSE belongs to the back-up domain, enable access.*/
  while (!LL_PWR_IsEnabledBkUpAccess()) {
  /* Wait for Backup domain access */
  }
  LL_RCC_ForceBackupDomainReset();
  LL_RCC_ReleaseBackupDomainReset();

  /* Enable LSE Oscillator (32.768 kHz) */
  LL_RCC_LSE_Enable();
  while (!LL_RCC_LSE_IsReady()) {
    /* Wait for LSE ready */
  }

  LL_PWR_DisableBkUpAccess();

  /* Switch OFF LSI as LSE is the source clock */
  LL_RCC_LSI2_Disable();
}

void HCISharedMemTransportClass::stm32wb_reset(void)
{
  // Reset IPCC
  LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_IPCC);

  LL_C1_IPCC_ClearFlag_CHx(
    IPCC,
    LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4
    | LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

  LL_C2_IPCC_ClearFlag_CHx(
    IPCC,
    LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4
    | LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

  LL_C1_IPCC_DisableTransmitChannel(
    IPCC,
    LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4
    | LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

  LL_C2_IPCC_DisableTransmitChannel(
    IPCC,
    LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4
    | LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

  LL_C1_IPCC_DisableReceiveChannel(
    IPCC,
    LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4
    | LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

  LL_C2_IPCC_DisableReceiveChannel(
    IPCC,
    LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4
    | LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

  /* IPCC default IRQ handlers: IPCC_C1_TX_IRQHandler & IPCC_C1_RX_IRQHandler
   * are mapped in the flash mem area, so that NVIC does not need to SetVector
   */
}

int HCISharedMemTransportClass::stm32wb_start_ble(void)
{
  SHCI_C2_Ble_Init_Cmd_Packet_t ble_init_cmd_packet = {
    0, 0, 0,                            /**< Header unused */
    0,                                  /** pBleBufferAddress not used */
    0,                                  /** BleBufferSize not used */
    CFG_BLE_NUM_GATT_ATTRIBUTES,
    CFG_BLE_NUM_GATT_SERVICES,
    CFG_BLE_ATT_VALUE_ARRAY_SIZE,
    CFG_BLE_NUM_LINK,
    CFG_BLE_DATA_LENGTH_EXTENSION,
    CFG_BLE_PREPARE_WRITE_LIST_SIZE,
    CFG_BLE_MBLOCK_COUNT,
    CFG_BLE_MAX_ATT_MTU,
    CFG_BLE_PERIPHERAL_SCA,
    CFG_BLE_CENTRAL_SCA,
    CFG_BLE_LS_SOURCE,
    CFG_BLE_MAX_CONN_EVENT_LENGTH,
    CFG_BLE_HSE_STARTUP_TIME,
    CFG_BLE_VITERBI_MODE,
    CFG_BLE_OPTIONS,
    0,                                  /** TODO Should be read from HW */
    CFG_BLE_MAX_COC_INITIATOR_NBR,
    CFG_BLE_MIN_TX_POWER,
    CFG_BLE_MAX_TX_POWER,
    CFG_BLE_RX_MODEL_CONFIG,
    CFG_BLE_MAX_ADV_SET_NBR,
    CFG_BLE_MAX_ADV_DATA_LEN,
    CFG_BLE_TX_PATH_COMPENS,
    CFG_BLE_RX_PATH_COMPENS,
    CFG_BLE_CORE_VERSION,
    CFG_BLE_OPTIONS_EXT
  };
  /**
   * Starts the BLE Stack on CPU2
   */
  if (SHCI_C2_BLE_Init(&ble_init_cmd_packet) == SHCI_Success) {
    return 1;
  }
  return 0;
}

void HCISharedMemTransportClass::transport_init(void)
{
  TL_MM_Config_t tl_mm_config;
  TL_BLE_InitConf_t tl_ble_config;
  /* STM32WB offers a System Channel HCI interface for
     offering system services, with proprietary commands.
     System Channel must be used as well for starting up
     BLE service so we need to initialize it. */
  SHCI_TL_HciInitConf_t shci_init_config;

  /**< Reference table initialization */
  TL_Init();

  /**< System channel initialization */
  shci_init_config.p_cmdbuffer = (uint8_t *)&SystemCmdBuffer;
  shci_init_config.StatusNotCallBack = syscmd_status_not;
  shci_init(sysevt_received, (void *) &shci_init_config);

  /**< Memory Manager channel initialization */
  tl_mm_config.p_BleSpareEvtBuffer = BleSpareEvtBuffer;
  tl_mm_config.p_SystemSpareEvtBuffer = SystemSpareEvtBuffer;
  tl_mm_config.p_AsynchEvtPool = EvtPool;
  tl_mm_config.AsynchEvtPoolSize = POOL_SIZE;
  TL_MM_Init(&tl_mm_config);

  TL_Enable();

  /*  At this stage, we'll need to wait for ready event,
   *  passed thru TL_SYS_EvtReceived */
  if (!sysevt_wait()) {
#if defined(PRINT_IPCC_INFO)
    printf("ERROR booting WB controller\r\n");
#endif /*(PRINT_IPCC_INFO)*/
  } else {
    /**< BLE channel initialization */
    tl_ble_config.p_cmdbuffer = (uint8_t *)&BleCmdBuffer;
    tl_ble_config.p_AclDataBuffer = HciAclDataBuffer;
    tl_ble_config.IoBusEvtCallBack = evt_received;
    tl_ble_config.IoBusAclDataTxAck = acl_data_ack;
    TL_BLE_Init((void *)&tl_ble_config);
  }
}

int HCISharedMemTransportClass::bt_ipm_set_addr(void)
{
  /* the specific table for set addr is 8 bytes:
   * one byte for config_offset
   * one byte for length
   * 6 bytes for payload */
  uint8_t data[4 + 8];

  phase_bd_addr = false;

  if (get_bd_address(bd_addr_udn)) {
    /* create ACI_HAL_WRITE_CONFIG_DATA */

    data[0] = BT_BUF_CMD;
    data[1] = uint8_t(ACI_WRITE_CONFIG_DATA_OPCODE & 0x000000FF); /* OCF */
    data[2] = uint8_t((ACI_WRITE_CONFIG_DATA_OPCODE & 0x0000FF00) >> 8); /* OGF */
    data[3] = 8; /* length of parameters */
    /* fill the ACI_HAL_WRITE_CONFIG_DATA with the addr*/
    data[4] = CONFIG_DATA_PUBADDR_OFFSET; /* the offset */
    data[5] = 6; /* is the length of the bd_addr table */
    memcpy(data + 6, bd_addr_udn, 6);
    /* send the ACI_HAL_WRITE_CONFIG_DATA */
    if (mbox_write(data[0], 11, &data[1]) != 11) {
      /* Error: no data are written */
      return 0;
    }
    /* now wait for the corresponding Rx event */
    return 1; /* success */
  }
  return 0; /* Error */
}


int HCISharedMemTransportClass::bt_ipm_set_power(void)
{
  /* the specific table for power is 2 bytes:
   * En_High_Power byte, PA_level byte */
  uint8_t data[4 + 2];

  phase_tx_power = false;

  data[0] = BT_BUF_CMD; /* the type */
  data[1] = (uint8_t)(ACI_HAL_SET_TX_POWER_LEVEL & 0x000000FF); /* the OPCODE */
  data[2] = (uint8_t)((ACI_HAL_SET_TX_POWER_LEVEL & 0x0000FF00) >> 8);
  data[3] = 2; /* the length */
  /* fill the SET_POWER */
  data[4] = 0x01; /* En_High_Power */
  data[5] = CFG_TX_POWER; /* PA_level */

  /* send the SET_POWER */
  if (mbox_write(data[0], 5, &data[1]) != 5) {
    /* Error: no data are written */
    return 0;
  }
  /* now wait for the corresponding Rx event */
  return 1; /* success */
}

#endif /* STM32WBxx */
