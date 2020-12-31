/*----------------------------------------------------------------------------
 *      R T L  -  C A N   D r i v e r
 *----------------------------------------------------------------------------
 *      Name:    RTX_CAN.h
 *      Purpose: Header for CAN Generic Layer Driver
 *      Rev.:    V3.40
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2008 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <RTL.h>                      /* RTX kernel functions & defines      */
#include "CAN_Cfg.h"                  /* CAN Configuration                   */


#ifndef _RTX_CAN_H
#define _RTX_CAN_H

/* Definition of memory pool type, for CAN messages                          */
#define CAN_msgpool_declare(name,cnt)     U32 name[cnt*(sizeof(CAN_msg)/4) + 3]

/* Definition of mailbox array, for used controllers                         */
#define mbx_arr_declare(name,arr_num,cnt) U32 name[arr_num][4 + cnt]

/* CAN message object structure                                              */
typedef struct  {
  U32 id;                 /* 29 bit identifier                               */
  U8  data[8];            /* Data field                                      */
  U8  len;                /* Length of data field in bytes                   */
  U8  ch;                 /* Object channel                                  */
  U8  format;             /* 0 - STANDARD, 1- EXTENDED IDENTIFIER            */
  U8  type;               /* 0 - DATA FRAME, 1 - REMOTE FRAME                */
}  CAN_msg;

/* Externaly declared memory pool for CAN messages, both transmit and receive*/
extern U32 CAN_mpool[CAN_CTRL_MAX_NUM*(CAN_No_SendObjects+CAN_No_ReceiveObjects)*(sizeof(CAN_msg)/4) + 3];

/* Externaly declared mailbox, for CAN transmit messages                     */
extern U32 MBX_tx_ctrl[CAN_CTRL_MAX_NUM][4 + CAN_No_SendObjects];

/* Externaly declared mailbox, for CAN receive messages                      */
extern U32 MBX_rx_ctrl[CAN_CTRL_MAX_NUM][4 + CAN_No_ReceiveObjects];

/* Semaphores used for protecting writing to CAN hardware                    */
extern OS_SEM wr_sem[CAN_CTRL_MAX_NUM];

/* Symbolic names for formats of CAN message                                 */
typedef enum {STANDARD_FORMAT = 0, EXTENDED_FORMAT} CAN_FORMAT;

/* Symbolic names for type of CAN message                                    */
typedef enum {DATA_FRAME = 0, REMOTE_FRAME}         CAN_FRAME;

/* Functions defined in module CAN.c                                         */
CAN_ERROR CAN_init      (U32 ctrl, U32 baudrate);
CAN_ERROR CAN_start     (U32 ctrl);
CAN_ERROR CAN_send      (U32 ctrl, CAN_msg *msg, U16 timeout);
CAN_ERROR CAN_request   (U32 ctrl, CAN_msg *msg, U16 timeout);
CAN_ERROR CAN_set       (U32 ctrl, CAN_msg *msg, U16 timeout);
CAN_ERROR CAN_receive   (U32 ctrl, CAN_msg *msg, U16 timeout);
CAN_ERROR CAN_rx_object (U32 ctrl, U32 ch, U32 id, CAN_FORMAT filter_type);
CAN_ERROR CAN_rx_object_mask (U32 ctrl, U32 ch, U32 id,U32 mask, CAN_FORMAT format);
CAN_ERROR CAN_rx_object_mask_idx (U32 ctrl, U32 ch, U32 id, U32 mask, U8 idx, U8 enable, CAN_FORMAT format);
CAN_ERROR CAN_tx_object (U32 ctrl, U32 ch, CAN_FORMAT filter_type);

CAN_ERROR CAN_init_ex		(U32 ctrl, U32 baudrate, U8 bArt);
CAN_ERROR CAN_reinit		(U32 ctrl, U32 baudrate);
CAN_ERROR CAN_reinit_ex	(U32 ctrl, U32 baudrate, U8 bArt);
CAN_ERROR CAN_start_ex	(U32 ctrl, U8 bABOM);

//Ðì³É
#if CAN_MUT_Enable
	#define CAN_MUT_Init(mut) os_mut_init (mut);
	#define CAN_LOCK() os_mut_wait (mut_can, 0xffff);
	#define CAN_UNLOCK() os_mut_release (mut_can);
#else
	#define CAN_MUT_Init(mut)
	#define CAN_LOCK()
	#define CAN_UNLOCK()
#endif

#endif /* _RTX_CAN_H */


/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

