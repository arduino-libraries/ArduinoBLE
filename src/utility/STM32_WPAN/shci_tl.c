/**
 ******************************************************************************
 * @file    shci.c
 * @author  MCD Application Team
 * @brief   System HCI command implementation
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


#if defined(STM32WBxx)
/* Includes ------------------------------------------------------------------*/
#include "stm32_wpan_common.h"

#include "stm_list.h"
#include "shci_tl.h"
#include "stm32_def.h"
#include "wiring_time.h"

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
  SHCI_TL_CMD_RESP_RELEASE,
  SHCI_TL_CMD_RESP_WAIT,
} SHCI_TL_CmdRespStatus_t;

/* Private defines -----------------------------------------------------------*/
/**
 * The default System HCI layer timeout is set to 33s
 */
#define SHCI_TL_DEFAULT_TIMEOUT (33000)

/* Private macros ------------------------------------------------------------*/
/* Public variables ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/**
 * START of Section SYSTEM_DRIVER_CONTEXT
 */
PLACE_IN_SECTION("SYSTEM_DRIVER_CONTEXT") static tListNode SHciAsynchEventQueue;
PLACE_IN_SECTION("SYSTEM_DRIVER_CONTEXT") static volatile SHCI_TL_CmdStatus_t SHCICmdStatus;
PLACE_IN_SECTION("SYSTEM_DRIVER_CONTEXT") static TL_CmdPacket_t *pCmdBuffer;
PLACE_IN_SECTION("SYSTEM_DRIVER_CONTEXT") SHCI_TL_UserEventFlowStatus_t SHCI_TL_UserEventFlow;
/**
 * END of Section SYSTEM_DRIVER_CONTEXT
 */

static tSHciContext shciContext;
static void (* StatusNotCallBackFunction) (SHCI_TL_CmdStatus_t status);

static volatile SHCI_TL_CmdRespStatus_t CmdRspStatusFlag;

/* Private function prototypes -----------------------------------------------*/
static void Cmd_SetStatus(SHCI_TL_CmdStatus_t shcicmdstatus);
static void TlCmdEvtReceived(TL_EvtPacket_t *shcievt);
static void TlUserEvtReceived(TL_EvtPacket_t *shcievt);
static void TlInit( TL_CmdPacket_t * p_cmdbuffer );

/* Interface ------- ---------------------------------------------------------*/
void shci_init(void(* UserEvtRx)(void* pData), void* pConf)
{
  StatusNotCallBackFunction = ((SHCI_TL_HciInitConf_t *)pConf)->StatusNotCallBack;
  shciContext.UserEvtRx = UserEvtRx;

  shci_register_io_bus (&shciContext.io);

  TlInit((TL_CmdPacket_t *)(((SHCI_TL_HciInitConf_t *)pConf)->p_cmdbuffer));

  return;
}

void shci_user_evt_proc(void)
{
  TL_EvtPacket_t *phcievtbuffer;
  tSHCI_UserEvtRxParam UserEvtRxParam;

  /**
   * Up to release version v1.2.0, a while loop was implemented to read out events from the queue as long as
   * it is not empty. However, in a bare metal implementation, this leads to calling in a "blocking" mode
   * shci_user_evt_proc() as long as events are received without giving the opportunity to run other tasks
   * in the background.
   * From now, the events are reported one by one. When it is checked there is still an event pending in the queue,
   * a request to the user is made to call again shci_user_evt_proc().
   * This gives the opportunity to the application to run other background tasks between each event.
   */

  /**
   * It is more secure to use LST_remove_head()/LST_insert_head() compare to LST_get_next_node()/LST_remove_node()
   * in case the user overwrite the header where the next/prev pointers are located
   */
  if((LST_is_empty(&SHciAsynchEventQueue) == FALSE) && (SHCI_TL_UserEventFlow != SHCI_TL_UserEventFlow_Disable))
  {
    LST_remove_head ( &SHciAsynchEventQueue, (tListNode **)&phcievtbuffer );

    if (shciContext.UserEvtRx != NULL)
    {
      UserEvtRxParam.pckt = phcievtbuffer;
      UserEvtRxParam.status = SHCI_TL_UserEventFlow_Enable;
      shciContext.UserEvtRx((void *)&UserEvtRxParam);
      SHCI_TL_UserEventFlow = UserEvtRxParam.status;
    }
    else
    {
      SHCI_TL_UserEventFlow = SHCI_TL_UserEventFlow_Enable;
    }

    if(SHCI_TL_UserEventFlow != SHCI_TL_UserEventFlow_Disable)
    {
      TL_MM_EvtDone( phcievtbuffer );
    }
    else
    {
      /**
       * put back the event in the queue
       */
      LST_insert_head ( &SHciAsynchEventQueue, (tListNode *)phcievtbuffer );
    }
  }

  if((LST_is_empty(&SHciAsynchEventQueue) == FALSE) && (SHCI_TL_UserEventFlow != SHCI_TL_UserEventFlow_Disable))
  {
    shci_notify_asynch_evt((void*) &SHciAsynchEventQueue);
  }


  return;
}

void shci_resume_flow( void )
{
  SHCI_TL_UserEventFlow = SHCI_TL_UserEventFlow_Enable;

  /**
   * It is better to go through the background process as it is not sure from which context this API may
   * be called
   */
  shci_notify_asynch_evt((void*) &SHciAsynchEventQueue);

  return;
}

void shci_send( uint16_t cmd_code, uint8_t len_cmd_payload, uint8_t * p_cmd_payload, TL_EvtPacket_t * p_rsp )
{
  Cmd_SetStatus(SHCI_TL_CmdBusy);

  pCmdBuffer->cmdserial.cmd.cmdcode = cmd_code;
  pCmdBuffer->cmdserial.cmd.plen = len_cmd_payload;

  memcpy(pCmdBuffer->cmdserial.cmd.payload, p_cmd_payload, len_cmd_payload );
  CmdRspStatusFlag = SHCI_TL_CMD_RESP_WAIT;
  shciContext.io.Send(0,0);

  shci_cmd_resp_wait(SHCI_TL_DEFAULT_TIMEOUT);

  /**
   * The command complete of a system command does not have the header
   * It starts immediately with the evtserial field
   */
  memcpy( &(p_rsp->evtserial), pCmdBuffer, ((TL_EvtSerial_t*)pCmdBuffer)->evt.plen + TL_EVT_HDR_SIZE );

  Cmd_SetStatus(SHCI_TL_CmdAvailable);

  return;
}

void shci_notify_asynch_evt(void *pdata)
{
  UNUSED(pdata);
  /* Need to parse data in future version */
  shci_user_evt_proc();
}

void shci_register_io_bus(tSHciIO *fops)
{
  /* Register IO bus services */
  fops->Init    = TL_SYS_Init;
  fops->Send    = TL_SYS_SendCmd;
}

/* Private functions ---------------------------------------------------------*/
static void TlInit( TL_CmdPacket_t * p_cmdbuffer )
{
  TL_SYS_InitConf_t Conf;

  pCmdBuffer = p_cmdbuffer;

  LST_init_head (&SHciAsynchEventQueue);

  Cmd_SetStatus(SHCI_TL_CmdAvailable);

  SHCI_TL_UserEventFlow = SHCI_TL_UserEventFlow_Enable;

  /* Initialize low level driver */
  if (shciContext.io.Init)
  {

    Conf.p_cmdbuffer = (uint8_t *)p_cmdbuffer;
    Conf.IoBusCallBackCmdEvt = TlCmdEvtReceived;
    Conf.IoBusCallBackUserEvt = TlUserEvtReceived;
    shciContext.io.Init(&Conf);
  }

  return;
}

static void Cmd_SetStatus(SHCI_TL_CmdStatus_t shcicmdstatus)
{
  if(shcicmdstatus == SHCI_TL_CmdBusy)
  {
    if(StatusNotCallBackFunction != 0)
    {
      StatusNotCallBackFunction( SHCI_TL_CmdBusy );
    }
    SHCICmdStatus = SHCI_TL_CmdBusy;
  }
  else
  {
    SHCICmdStatus = SHCI_TL_CmdAvailable;
    if(StatusNotCallBackFunction != 0)
    {
      StatusNotCallBackFunction( SHCI_TL_CmdAvailable );
    }
  }

  return;
}

static void TlCmdEvtReceived(TL_EvtPacket_t *shcievt)
{
  (void)(shcievt);
  shci_cmd_resp_release(0); /**< Notify the application the Cmd response has been received */

  return;
}

static void TlUserEvtReceived(TL_EvtPacket_t *shcievt)
{
  LST_insert_tail(&SHciAsynchEventQueue, (tListNode *)shcievt);
  shci_notify_asynch_evt((void*) &SHciAsynchEventQueue); /**< Notify the application a full HCI event has been received */

  return;
}

/* Weak implementation ----------------------------------------------------------------*/
__WEAK void shci_cmd_resp_wait(uint32_t timeout)
{
  for (unsigned long start = millis(); (millis() - start) < timeout;) {
    if (CmdRspStatusFlag == SHCI_TL_CMD_RESP_RELEASE) {
      break;
    }
  }
  return;
}

__WEAK void shci_cmd_resp_release(uint32_t flag)
{
  (void)flag;

  CmdRspStatusFlag = SHCI_TL_CMD_RESP_RELEASE;

  return;
}
#endif /* STM32WBxx */
