/*----------------------------------------------------------------------------
 *      R T L  -  C A N   D r i v e r
 *----------------------------------------------------------------------------
 *      Name:    RTX_CAN.c
 *      Purpose: CAN Generic Layer Driver
 *      Rev.:    V3.40
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2008 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <RTL.h>                      /* RTX kernel functions & defines      */
#include "CAN_Cfg.h"                  /* CAN Configuration                   */
#include "RTX_CAN.h"                  /* CAN Generic functions & defines     */
#include "CAN_Hw.h"                   /* CAN hw specific functions & defines */

#pragma diag_suppress 550


/* Declare memory pool for CAN messages, both transmit and receive           */
CAN_msgpool_declare(CAN_mpool,CAN_CTRL_MAX_NUM*(CAN_No_SendObjects+CAN_No_ReceiveObjects));

/* Declare mailbox, for CAN transmit messages                                */
mbx_arr_declare(MBX_tx_ctrl,CAN_CTRL_MAX_NUM,CAN_No_SendObjects);

/* Declare mailbox, for CAN receive messages                                 */
mbx_arr_declare(MBX_rx_ctrl,CAN_CTRL_MAX_NUM,CAN_No_ReceiveObjects);

/* Semaphores used for protecting writing to CAN hardware                    */
OS_SEM wr_sem[CAN_CTRL_MAX_NUM];


/*----------------------------------------------------------------------------
 *      CAN RTX Generic Driver Functions
 *----------------------------------------------------------------------------
 *  Functions implemented in this module:
 *           CAN_ERROR CAN_mem_init  (void);
 *           CAN_ERROR CAN_setup     (void)
 *           CAN_ERROR CAN_init      (U32 ctrl, U32 baudrate)
 *           CAN_ERROR CAN_start     (U32 ctrl)
 *    static CAN_ERROR CAN_push      (U32 ctrl, CAN_msg *msg, U16 timeout)
 *           CAN_ERROR CAN_send      (U32 ctrl, CAN_msg *msg, U16 timeout)
 *           CAN_ERROR CAN_request   (U32 ctrl, CAN_msg *msg, U16 timeout)
 *           CAN_ERROR CAN_set       (U32 ctrl, CAN_msg *msg, U16 timeout)
 *    static CAN_ERROR CAN_pull      (U32 ctrl, CAN_msg *msg, U16 timeout)
 *           CAN_ERROR CAN_receive   (U32 ctrl, CAN_msg *msg, U16 timeout)
 *           CAN_ERROR CAN_rx_object (U32 ctrl, U32 ch, U32 id, CAN_FORMAT format)
 *           CAN_ERROR CAN_tx_object (U32 ctrl, U32 ch, CAN_FORMAT format)
 *---------------------------------------------------------------------------*/


/*--------------------------- CAN_init --------------------------------------
 *
 *  The first time this function is called initialize the memory pool for 
 *  CAN messages and setup CAN controllers hardware
 *
 *  Initialize mailboxes for CAN messages and initialize CAN controller
 *
 *  Parameter:  ctrl:       Index of the hardware CAN controller (1 .. x)
 *              baudrate:   Baudrate
 *
 *  Return:     CAN_ERROR:  Error code
 *---------------------------------------------------------------------------*/
#if CAN_MUT_Enable
static OS_MUT mut_can;
#endif

CAN_ERROR CAN_init (U32 ctrl, U32 baudrate)  {
  static U8 first_run_flag = 0;
  CAN_ERROR error_code;
  U32 ctrl0 = ctrl-1;                 /* Controller index 0 .. x-1           */

  /* Initialize can mutex£¨Ðì³É£© */
  CAN_MUT_Init(mut_can);

  /* Initialize the Semaphore before the first use */
  os_sem_init (wr_sem[ctrl0], 1);

  /* When function is called for the first time it will initialize and setup 
     all of the resources that are common to CAN functionality               */
  if (first_run_flag == 0)  {
    first_run_flag = 1;
    if (_init_box (CAN_mpool, sizeof(CAN_mpool), sizeof(CAN_msg)) == 1)
      return CAN_MEM_POOL_INIT_ERROR;
  }

  os_mbx_init (MBX_tx_ctrl[ctrl0], sizeof(MBX_tx_ctrl[ctrl0]));
  os_mbx_init (MBX_rx_ctrl[ctrl0], sizeof(MBX_rx_ctrl[ctrl0]));

  error_code = CAN_hw_setup (ctrl);
  if (error_code != CAN_OK) 
    return error_code;
  
  return (CAN_hw_init (ctrl, baudrate));
}

CAN_ERROR CAN_reinit(U32 ctrl, U32 baudrate)
{
	return CAN_hw_reinit(ctrl, baudrate);
}

CAN_ERROR CAN_init_ex	(U32 ctrl, U32 baudrate, U8 bArt)
{ 
	static U8 first_run_flag = 0;
	CAN_ERROR error_code;
	U32 ctrl0 = ctrl-1;                 /* Controller index 0 .. x-1           */
	
	/* Initialize can mutex£¨Ðì³É£© */
	CAN_MUT_Init(mut_can);
	
	/* Initialize the Semaphore before the first use */
	os_sem_init (wr_sem[ctrl0], 1);
	
	/* When function is called for the first time it will initialize and setup 
	 all of the resources that are common to CAN functionality               */
	if (first_run_flag == 0)  {
	first_run_flag = 1;
	if (_init_box (CAN_mpool, sizeof(CAN_mpool), sizeof(CAN_msg)) == 1)
	  return CAN_MEM_POOL_INIT_ERROR;
	}
	
	os_mbx_init (MBX_tx_ctrl[ctrl0], sizeof(MBX_tx_ctrl[ctrl0]));
	os_mbx_init (MBX_rx_ctrl[ctrl0], sizeof(MBX_rx_ctrl[ctrl0]));
	
	error_code = CAN_hw_setup (ctrl);
	if (error_code != CAN_OK) 
		return error_code;
		
	return (CAN_hw_init_ex (ctrl, baudrate, bArt));
}

CAN_ERROR CAN_reinit_ex	(U32 ctrl, U32 baudrate, U8 bArt)
{
	return (CAN_hw_reinit_ex (ctrl, baudrate, bArt));
}


/*--------------------------- CAN_start -------------------------------------
 *
 *  Start CAN controller (enable it to participate on CAN network)
 *
 *  Parameter:  ctrl:       Index of the hardware CAN controller (1 .. x)
 *
 *  Return:     CAN_ERROR:  Error code
 *---------------------------------------------------------------------------*/

CAN_ERROR CAN_start (U32 ctrl)  {
  CAN_ERROR err;
  CAN_LOCK();
  	err = CAN_hw_start (ctrl);
  CAN_UNLOCK();
  return err;
}

CAN_ERROR CAN_start_ex	(U32 ctrl, U8 bABOM)
{
  CAN_ERROR err;
  CAN_LOCK();
	err = (CAN_hw_start_ex (ctrl, bABOM));
  CAN_UNLOCK();
  return err;
}


/*--------------------------- CAN_push --------------------------------------
 *
 *  Send CAN_msg if hardware is free for sending, otherwise push message to 
 *  message queue to be sent when hardware becomes free
 *
 *  Parameter:  ctrl:       Index of the hardware CAN controller (1 .. x)
 *              msg:        Pointer to CAN message to be sent
 *              timeout:    Timeout value for message sending
 *
 *  Return:     CAN_ERROR:  Error code
 *---------------------------------------------------------------------------*/

static CAN_ERROR CAN_push (U32 ctrl, CAN_msg *msg, U16 timeout)  {
  CAN_msg *ptrmsg;
  U32 ctrl0 = ctrl-1;                 /* Controller index 0 .. x-1           */                                   
  
  if (CAN_hw_tx_empty (ctrl) == CAN_OK) { /* Transmit hardware free for send */
    CAN_hw_wr (ctrl, msg);            /* Send message                        */
  }
  else {                              /* If hardware for sending is busy     */
    /* Write the message to send mailbox if there is room for it             */
    ptrmsg = _alloc_box (CAN_mpool);
    if (ptrmsg != NULL) {
      *ptrmsg = *msg;
      /* If message hasn't been sent but timeout expired, deallocate memory  */
      if (os_mbx_send (MBX_tx_ctrl[ctrl0], ptrmsg, timeout) == OS_R_TMO) {
        if (_free_box (CAN_mpool, ptrmsg) == 1)
		{
          return CAN_DEALLOC_MEM_ERROR;
		}
        return CAN_TIMEOUT_ERROR;
      }
    } else
	{
      return CAN_ALLOC_MEM_ERROR;
	}
  }
  return CAN_OK;
}


/*--------------------------- CAN_send --------------------------------------
 *
 *  Send DATA FRAME message, see CAN_push function comment
 *
 *  Parameter:  ctrl:       Index of the hardware CAN controller (1 .. x)
 *              msg:        Pointer to CAN message to be sent
 *              timeout:    Timeout value for message sending
 *
 *  Return:     CAN_ERROR:  Error code
 *---------------------------------------------------------------------------*/

CAN_ERROR CAN_send (U32 ctrl, CAN_msg *msg, U16 timeout)  {
  CAN_ERROR err;
  CAN_LOCK();
    msg->type = DATA_FRAME;
    err = CAN_push (ctrl, msg, timeout);
	//os_dly_wait(1000);
  CAN_UNLOCK();
  return err;
}


/*--------------------------- CAN_request -----------------------------------
 *
 *  Send REMOTE FRAME message, see CAN_push function comment
 *
 *  Parameter:  ctrl:       Index of the hardware CAN controller (1 .. x)
 *              msg:        Pointer to CAN message to be sent
 *              timeout:    Timeout value for message sending
 *
 *  Return:     CAN_ERROR:  Error code
 *---------------------------------------------------------------------------*/

CAN_ERROR CAN_request (U32 ctrl, CAN_msg *msg, U16 timeout)  {

  CAN_ERROR err;
  CAN_LOCK();

  msg->type = REMOTE_FRAME;
  err = CAN_push (ctrl, msg, timeout);
  
  CAN_UNLOCK();
  return err;
}


/*--------------------------- CAN_set ---------------------------------------
 *
 *  Set a message that will automatically be sent as an answer to REMOTE
 *  FRAME message
 *
 *  Parameter:  ctrl:       Index of the hardware CAN controller (1 .. x)
 *              msg:        Pointer to CAN message to be set
 *              timeout:    Timeout value for message to be set
 *
 *  Return:     CAN_ERROR:  Error code
 *---------------------------------------------------------------------------*/

CAN_ERROR CAN_set (U32 ctrl, CAN_msg *msg, U16 timeout)  {
  
  S32 i = timeout;
  U32 result = 0;

  CAN_LOCK();
  do {
    if (CAN_hw_tx_empty (ctrl) == CAN_OK)  {  /* Transmit hardware free      */
      result = CAN_hw_set (ctrl, msg);  /* Set message                       */
      os_sem_send (wr_sem[ctrl-1]);     /* Return a token back to semaphore  */
	  CAN_UNLOCK();
      return CAN_OK;
    }
    if (timeout == 0xffff)              /* Indefinite wait                   */
      i++;
    i--;
    os_dly_wait (1);                    /* Wait 1 timer tick                 */
  }  while (i >= 0);

  CAN_UNLOCK();
  return CAN_TIMEOUT_ERROR;             /* CAN message not set               */
}


/*--------------------------- CAN_pull --------------------------------------
 *
 *  Pull first received and unread CAN_msg from receiving message queue
 *
 *  Parameter:  ctrl:       Index of the hardware CAN controller (1 .. x)
 *              msg:        Pointer where CAN message will be read
 *              timeout:    Timeout value for message receiving
 *
 *  Return:     CAN_ERROR:  Error code
 *---------------------------------------------------------------------------*/


static CAN_ERROR CAN_pull (U32 ctrl, CAN_msg *msg, U16 timeout)  {
  
  CAN_msg *ptrmsg;
  U32 ctrl0 = ctrl-1;                 /* Controller index 0 .. x-1           */
  
  /* Wait for received message in mailbox                                    */
  if (os_mbx_wait (MBX_rx_ctrl[ctrl0], (void **)&ptrmsg, timeout) == OS_R_TMO){
    return CAN_TIMEOUT_ERROR;
  }

  /* Copy received message from mailbox to address given in function parameter msg */
  *msg = *ptrmsg;

  /* Free box where message was kept                                         */
  if (_free_box (CAN_mpool, ptrmsg) == 1)
  {
    return CAN_DEALLOC_MEM_ERROR;
  }
  
  return CAN_OK;
}


/*--------------------------- CAN_receive -----------------------------------
 *
 *  Read received message, see CAN_pull function comment
 *
 *  Parameter:  ctrl:       Index of the hardware CAN controller (1 .. x)
 *              msg:        Pointer where CAN message will be read
 *              timeout:    Timeout value for message receiving
 *
 *  Return:     CAN_ERROR:  Error code
 *---------------------------------------------------------------------------*/

CAN_ERROR CAN_receive (U32 ctrl, CAN_msg *msg, U16 timeout)  {
  
  CAN_ERROR err;
 // CAN_LOCK();
  	err = CAN_pull (ctrl, msg, timeout);
 // CAN_UNLOCK();
  
  return err;
}


/*--------------------------- CAN_rx_object ---------------------------------
 *
 *  Enable reception of messages on specified controller and channel with 
 *  specified identifier
 *
 *  Parameter:  ctrl:       Index of the hardware CAN controller (1 .. x)
 *              ch:         Channel for the message transmission
 *              id:         CAN message identifier
 *              CAN_FORMAT: Format of CAN identifier (standard or extended)
 *
 *  Return:     CAN_ERROR:  Error code
 *---------------------------------------------------------------------------*/

CAN_ERROR CAN_rx_object (U32 ctrl, U32 ch, U32 id, CAN_FORMAT format)  {

  CAN_ERROR err;
  CAN_LOCK();
    err = CAN_hw_rx_object (ctrl, ch, id, format);
  CAN_UNLOCK();
  return err;
}

CAN_ERROR CAN_rx_object_mask (U32 ctrl, U32 ch, U32 id,U32 mask, CAN_FORMAT format)  {

  CAN_ERROR err;
  CAN_LOCK();
    err = CAN_hw_rx_object_mask (ctrl, ch, id, mask, format);
  CAN_UNLOCK();

  return err;
}

CAN_ERROR CAN_rx_object_mask_idx (U32 ctrl, U32 ch, U32 id, U32 mask, u8 idx, u8 enable, CAN_FORMAT format)
{
	CAN_ERROR err;
  	CAN_LOCK();
    	err = CAN_hw_rx_object_mask_idx (ctrl, ch, id, mask, idx, enable, format);
  	CAN_UNLOCK();

  	return err;
}


/*--------------------------- CAN_tx_object ---------------------------------
 *
 *  Enable transmission of messages on specified controller and channel with 
 *  specified identifier
 *
 *  Parameter:  ctrl:       Index of the hardware CAN controller (1 .. x)
 *              ch:         Channel for the message transmission
 *              CAN_FORMAT: Format of CAN identifier (standard or extended)
 *
 *  Return:     CAN_ERROR:  Error code
 *---------------------------------------------------------------------------*/

CAN_ERROR CAN_tx_object (U32 ctrl, U32 ch, CAN_FORMAT format)  {
  CAN_ERROR err;
  CAN_LOCK();
  
    err = CAN_hw_tx_object (ctrl, ch, format);
  
  CAN_UNLOCK();
  return err;
}


/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

