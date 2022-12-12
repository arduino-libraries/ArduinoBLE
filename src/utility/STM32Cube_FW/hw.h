/**
  ******************************************************************************
  * @file    hw.h
  * @author  MCD Application Team
  * @brief   Hardware
  ******************************************************************************
   * @attention
  *
  * Copyright (c) 2018-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HW_H
#define __HW_H

#ifdef __cplusplus
extern "C" {
#endif

  /* Includes ------------------------------------------------------------------*/
#include "stm32_def.h"
#include "stm32wbxx_ll_bus.h"
#include "stm32wbxx_ll_exti.h"
#include "stm32wbxx_ll_system.h"
#include "stm32wbxx_ll_rcc.h"
#include "stm32wbxx_ll_ipcc.h"
#include "stm32wbxx_ll_cortex.h"
#include "stm32wbxx_ll_utils.h"
#include "stm32wbxx_ll_pwr.h"

  /******************************************************************************
   * HW IPCC
   ******************************************************************************/
  void HW_IPCC_Enable( void );
  void HW_IPCC_Init( void );

  void HW_IPCC_BLE_Init( void );
  void HW_IPCC_BLE_SendCmd( void );
  void HW_IPCC_MM_SendFreeBuf( void (*cb)( void ) );
  void HW_IPCC_BLE_RxEvtNot( void );
  void HW_IPCC_BLE_SendAclData( void );
  void HW_IPCC_BLE_AclDataAckNot( void );

  void HW_IPCC_SYS_Init( void );
  void HW_IPCC_SYS_SendCmd( void );
  void HW_IPCC_SYS_CmdEvtNot( void );
  void HW_IPCC_SYS_EvtNot( void );

  void HW_IPCC_THREAD_Init( void );
  void HW_IPCC_OT_SendCmd( void );
  void HW_IPCC_CLI_SendCmd( void );
  void HW_IPCC_THREAD_SendAck( void );
  void HW_IPCC_OT_CmdEvtNot( void );
  void HW_IPCC_CLI_CmdEvtNot( void );
  void HW_IPCC_THREAD_EvtNot( void );
  void HW_IPCC_THREAD_CliSendAck( void );
  void HW_IPCC_THREAD_CliEvtNot( void );


  void HW_IPCC_LLDTESTS_Init( void );
  void HW_IPCC_LLDTESTS_SendCliCmd( void );
  void HW_IPCC_LLDTESTS_ReceiveCliRsp( void );
  void HW_IPCC_LLDTESTS_SendCliRspAck( void );
  void HW_IPCC_LLDTESTS_ReceiveM0Cmd( void );
  void HW_IPCC_LLDTESTS_SendM0CmdAck( void );


  void HW_IPCC_BLE_LLD_Init( void );
  void HW_IPCC_BLE_LLD_SendCliCmd( void );
  void HW_IPCC_BLE_LLD_ReceiveCliRsp( void );
  void HW_IPCC_BLE_LLD_SendCliRspAck( void );
  void HW_IPCC_BLE_LLD_ReceiveM0Cmd( void );
  void HW_IPCC_BLE_LLD_SendM0CmdAck( void );
  void HW_IPCC_BLE_LLD_SendCmd( void );
  void HW_IPCC_BLE_LLD_ReceiveRsp( void );
  void HW_IPCC_BLE_LLD_SendRspAck( void );

  
  void HW_IPCC_TRACES_Init( void );
  void HW_IPCC_TRACES_EvtNot( void );

#ifdef __cplusplus
}
#endif

#endif /*__HW_H */

