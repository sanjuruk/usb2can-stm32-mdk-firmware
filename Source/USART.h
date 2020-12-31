/******************** (C) COPYRIGHT 2009 Robotell ********************
* File Name          : USART.h
* Author             : 郭盖华
* Version            : V2.0.1
* Date               : 2009-4-23
* Description        : 串口通讯主文件
**********************************************************************/

//----------------------------------------------------------------------------
// Define to prevent recursive inclusion
//----------------------------------------------------------------------------
#ifndef __USART_H
#define __USART_H

//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------
#include "stm32f10x_lib.h"
#include "stm32f10x_it.h"
#include "string.h"
#include "RTL.h"
#include "RTX_CAN.h"


//在这里定义会使用哪些串口，如果不用就注释掉，节省空间
#define __USART_1
#define __USART_2
#define __USART_3

//----------------------------------------------------------------------------
// macros
//----------------------------------------------------------------------------
#define USART_FRAMECTRL 0xA5                                                  
#define USART_FRAMEHEAD 0xAA
#define USART_FRAMETAIL 0x55

#define RX_BUFFER_LEN 128
#define TX_BUFFER_LEN 128

#define TX_EVENT(i) (1 << i)
#define RX_EVENT(i) (1 << (i+3))

#define USART_MUT_Enable 0				//使能互斥量

#ifdef __USART_1
	#define USART_1 0x01					//USART1
#else
	#define USART_1 0x0A
#endif

#ifdef __USART_2
	#define USART_2 0x02					//USART2
#else
	#define USART_2 0x0B
#endif

#ifdef __USART_3
	#define USART_3 0x03					//USART3
#else
	#define USART_3 0x0C
#endif

#define NO_REMAP 0x00					//无重映射
#define PARTIAL_REMAP 0x10				//部分重映射
#define FULL_REMAP 0x20					//全部重映射

#define USART_NUM 3						//USART个数
#define USART_SENDOBJ_NUM  20			//发送缓冲的长度
#define USART_RECVOBJ_NUM  20			//接收缓冲的长度
#define MAX_PACK_SIZE 32				//最大包长

/* Definition of memory pool type, for CAN messages                          */
#define USART_msgpool_declare(name,cnt)  u32 name[((MAX_PACK_SIZE+3)/4)*(cnt) + 3]
#define USART_msgpool_ptr_declare(name,arr_num)  u32 *name[arr_num]

/* Definition of mailbox array, for used controllers */
#define USART_mbx_arr_declare(name,arr_num,cnt)  u32 name[arr_num][4 + cnt]

//徐成
#if USART_MUT_Enable
	#define USART_MUT_Init(i) os_mut_init (mut_usart[i]);
	#define USART_LOCK(i) os_mut_wait (mut_usart[i], 0xffff);
	#define USART_UNLOCK(i) os_mut_release (mut_usart[i]);
#else
	#define USART_MUT_Init(i)
	#define USART_LOCK(i)
	#define USART_UNLOCK(i)
#endif

#define USART_IRQTx_ENABLE(USARTx) USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
#define USART_IRQRx_ENABLE(USARTx) USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE); 
#define USART_IRQTx_DISABLE(USARTx) USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);
#define USART_IRQRx_DISABLE(USARTx) USART_ITConfig(USARTx, USART_IT_RXNE, DISABLE);

//----------------------------------------------------------------------------
// types
//----------------------------------------------------------------------------
typedef struct USART_QUEUE_ELEM
{
	u8 *pElem;
	u16 len;
}UQElem;

//----------------------------------------------------------------------------
// Exported variables
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Exported functions
//----------------------------------------------------------------------------
bool USART_Initial(u8 ctrl_remap, u32 baudrate);
u16 USART_RecvPackage(u32 ctrl, u8 *data , u16 timeout);
bool USART_SendPackage(u32 ctrl, s16 packid, u8 *data, u16 n, u16 timeout);

u16 USART_RecvBuf(u32 ctrl, u8 *buf, u16 n, u16 timeout);
bool USART_SendBuf(u32 ctrl, u8 *buf, u16 n, u16 timeout);
bool USART_FillRxBuf(u32 ctrl, u8 data);

bool USART_RxCallBack(u32 ctrl, u8 data);
void USART_TxCallBack(u32 ctrl);

USART_TypeDef* GetUSARTType(u32 USART_ID);
bool CheckCANMsgAvailable(CAN_msg *msg);

#endif
