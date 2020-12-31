/*----------------------------------------------------------------------------
 *      R T L  -  C A N   D r i v e r
 *----------------------------------------------------------------------------
 *      Name:    CAN_Hw.h
 *      Purpose: Header for CAN Hardware specific module
 *      Rev.:    V3.40
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2008 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include "stm32f10x_lib.h"            /* STM32F10x Library Definitions       */
#include "RTX_CAN.h"                  /* CAN Generic functions & defines     */


#ifndef _CAN_HW_H_
#define _CAN_HW_H_

/* register RCC------------------------------------------------------*/    
//#define RCC_APB1ENR_CANEN     ((unsigned long)0x02000000)
//#define RCC_APB2ENR_AFIOEN    ((unsigned long)0x00000001)
//#define RCC_APB2ENR_IOPAEN    ((unsigned long)0x00000004)
//#define RCC_APB2ENR_IOPBEN    ((unsigned long)0x00000008)  

/* register CAN_MCR ----------------------------------------------------------*/
//#define CAN_MCR_INRQ          ((unsigned long)0x00000001)
//#define CAN_MCR_NART          ((unsigned long)0x00000010)
//Ðì³É
//#define CAN_MCR_ABOM 		  ((unsigned long)0x00000040)

/* register CAN_FMR ----------------------------------------------------------*/
//#define CAN_FMR_FINIT         ((unsigned long)0x00000001)

/* register CAN_TSR ----------------------------------------------------------*/
//#define CAN_TSR_RQCP0         ((unsigned long)0x00000001)
//#define CAN_TSR_TME0          ((unsigned long)0x04000000)

/* register CAN_RF0R ---------------------------------------------------------*/
//#define CAN_RF0R_FMP0         ((unsigned long)0x00000003)
//#define CAN_RF0R_RFOM0        ((unsigned long)0x00000020)

/* register CAN_IER ----------------------------------------------------------*/
//#define CAN_IER_TMEIE         ((unsigned long)0x00000001)
//#define CAN_IER_FMPIE0        ((unsigned long)0x00000002)

/* register CAN_BTR ----------------------------------------------------------*/
//#define CAN_BTR_SILM          ((unsigned long)0x80000000)
//#define CAN_BTR_LBKM          ((unsigned long)0x40000000)

/* register CAN_TIxR ---------------------------------------------------------*/
#define CAN_TIxR_TXRQ         ((unsigned long)0x00000001)

/* register CAN_TDTxR --------------------------------------------------------*/
#define CAN_TDTxR_DLC         ((unsigned long)0x0000000F)



/* Functions defined in module CAN_Hw.c                                      */
CAN_ERROR CAN_hw_setup      (U32 ctrl);
CAN_ERROR CAN_hw_init       (U32 ctrl, U32 baudrate);
CAN_ERROR CAN_hw_init_ex	(U32 ctrl, U32 baudrate, U8 bArt);
CAN_ERROR CAN_hw_reinit		(U32 ctrl, U32 baudrate);
CAN_ERROR CAN_hw_reinit_ex	(U32 ctrl, U32 baudrate, U8 bArt);
CAN_ERROR CAN_hw_start      (U32 ctrl);
CAN_ERROR CAN_hw_start_ex	(U32 ctrl, U8 bABOM);
CAN_ERROR CAN_hw_tx_empty   (U32 ctrl);
CAN_ERROR CAN_hw_wr         (U32 ctrl, CAN_msg *msg);
CAN_ERROR CAN_hw_set        (U32 ctrl, CAN_msg *msg);
CAN_ERROR CAN_hw_rx_object  (U32 ctrl, U32 ch, U32 id, CAN_FORMAT filter_type);
CAN_ERROR CAN_hw_rx_object_mask (U32 ctrl, U32 ch, U32 id, U32 mask, CAN_FORMAT format);
CAN_ERROR CAN_hw_rx_object_mask_idx (U32 ctrl, U32 ch, U32 id, U32 mask, U8 idx, U8 enable, CAN_FORMAT format); 
CAN_ERROR CAN_hw_tx_object  (U32 ctrl, U32 ch, CAN_FORMAT filter_type);

CAN_ERROR CAN_hw_testmode   (U32 ctrl, U32 testmode);

//Ðì³É
typedef void (*CANERRHANDLER)(CAN_HW_ERR hwErr, u32 it);

void CAN_err_config(CANERRHANDLER ceh);
#endif /* _CAN_HW_H_ */

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

