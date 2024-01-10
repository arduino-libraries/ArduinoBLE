/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * File Name          : tl_dbg_conf.h
  * Description        : Debug configuration file for stm32wpan transport layer interface.
  *
 ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef TL_DBG_CONF_H
#define TL_DBG_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* USER CODE BEGIN Tl_Conf */

/* Includes ------------------------------------------------------------------*/
#include "core_debug.h"

/**
 * Enable or Disable traces
 * The raw data output is the hci binary packet format as specified by the BT specification *
 */
#ifndef TL_SHCI_CMD_DBG_EN
#define TL_SHCI_CMD_DBG_EN      1   /* Reports System commands sent to CPU2 and the command response */
#endif

#ifndef TL_SHCI_EVT_DBG_EN
#define TL_SHCI_EVT_DBG_EN      1   /* Reports System Asynchronous Events received from CPU2 */
#endif

#ifndef TL_HCI_CMD_DBG_EN
#define TL_HCI_CMD_DBG_EN       1   /* Reports BLE command sent to CPU2 and the command response */
#endif

#ifndef TL_HCI_EVT_DBG_EN
#define TL_HCI_EVT_DBG_EN       1   /* Reports BLE Asynchronous Events received from CPU2 */
#endif

#ifndef TL_MM_DBG_EN
#define TL_MM_DBG_EN            1   /* Reports the information of the buffer released to CPU2 */
#endif

/**
 * Macro definition
 */

/**
 * System Transport Layer
 */
#if (TL_SHCI_CMD_DBG_EN != 0)
#define TL_SHCI_CMD_DBG_MSG             core_debug
#define TL_SHCI_CMD_DBG_BUF             PRINT_LOG_BUFF_DBG
#else
#define TL_SHCI_CMD_DBG_MSG(...)
#define TL_SHCI_CMD_DBG_BUF(...)
#endif

#define TL_SHCI_CMD_DBG_RAW(...)

#if (TL_SHCI_EVT_DBG_EN != 0)
#define TL_SHCI_EVT_DBG_MSG             core_debug
#define TL_SHCI_EVT_DBG_BUF             PRINT_LOG_BUFF_DBG
#else
#define TL_SHCI_EVT_DBG_MSG(...)
#define TL_SHCI_EVT_DBG_BUF(...)
#endif

#define TL_SHCI_EVT_DBG_RAW(...)

/**
 * BLE Transport Layer
 */
#if (TL_HCI_CMD_DBG_EN != 0)
#define TL_HCI_CMD_DBG_MSG             core_debug
#define TL_HCI_CMD_DBG_BUF             PRINT_LOG_BUFF_DBG
#else
#define TL_HCI_CMD_DBG_MSG(...)
#define TL_HCI_CMD_DBG_BUF(...)
#endif

#define TL_HCI_CMD_DBG_RAW(...)

#if (TL_HCI_EVT_DBG_EN != 0)
#define TL_HCI_EVT_DBG_MSG             core_debug
#define TL_HCI_EVT_DBG_BUF             PRINT_LOG_BUFF_DBG
#else
#define TL_HCI_EVT_DBG_MSG(...)
#define TL_HCI_EVT_DBG_BUF(...)
#endif

#define TL_HCI_EVT_DBG_RAW(...)

/**
 * Memory Manager - Released buffer tracing
 */
#if (TL_MM_DBG_EN != 0)
#define TL_MM_DBG_MSG             core_debug
#else
#define TL_MM_DBG_MSG(...)
#endif


#define PRINT_LOG_BUFF_DBG(...) DbgTraceBuffer(__VA_ARGS__)

void DbgTraceBuffer(const void *pBuffer, uint32_t u32Length, const char *strFormat, ...)
{
  va_list vaArgs;
  uint32_t u32Index;
  va_start(vaArgs, strFormat);
  vprintf(strFormat, vaArgs);
  va_end(vaArgs);
  for (u32Index = 0; u32Index < u32Length; u32Index ++)
  {
    core_debug(" %02X", ((const uint8_t *) pBuffer)[u32Index]);
  }
}

/* USER CODE END Tl_Conf */

#ifdef __cplusplus
}
#endif

#endif /* TL_DBG_CONF_H */

