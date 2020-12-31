#include "USART.h"
#include "stdlib.h"
#include <rtl.h>

//声明发送缓冲区的空间
#ifdef __USART_1
	u32 USART_1_TPOOL[USART_SENDOBJ_NUM*((MAX_PACK_SIZE+3)/4) + 3];
#endif
#ifdef __USART_2
	u32 USART_2_TPOOL[USART_SENDOBJ_NUM*((MAX_PACK_SIZE+3)/4) + 3];
#endif
#ifdef __USART_3
	u32 USART_3_TPOOL[USART_SENDOBJ_NUM*((MAX_PACK_SIZE+3)/4) + 3];
#endif

//声明缓冲区
USART_msgpool_ptr_declare(USART_mTpool, USART_NUM);						//发送缓冲区
USART_msgpool_declare(USART_mRpool, USART_NUM*(USART_RECVOBJ_NUM));		//接收缓冲区

//声明发送邮箱
USART_mbx_arr_declare(USART_MBX_TX,USART_NUM,USART_SENDOBJ_NUM);

//声明接收邮箱
USART_mbx_arr_declare(USART_MBX_RX,USART_NUM,USART_RECVOBJ_NUM);

#if USART_MUT_Enable
	static OS_MUT mut_usart[USART_NUM];
#endif

bool UASRT_Signal[USART_NUM];
static u32 UASRT_RecvDataLen[USART_NUM];
static bool UASRT_UsePack[USART_NUM];
static OS_TID tx_task[USART_NUM];

//接收部分的变量
static u8 	*ptrRxMsg_Buf[USART_NUM] = { 0, 0 , 0};
static u32	USART_RecvOffset_Buf[USART_NUM] = { 0, 0 , 0};
static bool bFirstIn_Buf[USART_NUM] = {TRUE, TRUE, TRUE};

bool USART_Initial(u8 ctrl_remap, u32 baudrate)
{
	static U8 firstRun = 0;
	
	u32 ctrl0 = (u32)(ctrl_remap & 0x0f) - 1;

    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
	
	u32 RCC_APBxPeriph_GPIOx;
	u32 RCC_APBxPeriph_USARTx;
	u16 GPIO_Pin_Tx;
	u16 GPIO_Pin_Rx;
	GPIO_TypeDef *GPIOx;
	USART_TypeDef *UsartBase;
	u8 USARTx_IRQChannel;
	
	//初始化互斥量
	USART_MUT_Init(ctrl0);

	//初始化邮箱
	if (firstRun == 0)
	{
	    firstRun = 1;
		
#ifdef __USART_1
		USART_mTpool[0] = USART_1_TPOOL;
		if (_init_box (USART_mTpool[0], sizeof(USART_1_TPOOL), MAX_PACK_SIZE) == 1)
			return FALSE;
#endif
#ifdef __USART_2
		USART_mTpool[1] = USART_2_TPOOL;
		if (_init_box (USART_mTpool[1], sizeof(USART_2_TPOOL), MAX_PACK_SIZE) == 1)
			return FALSE;
#endif
#ifdef __USART_3
		USART_mTpool[2] = USART_3_TPOOL;
		if (_init_box (USART_mTpool[2], sizeof(USART_3_TPOOL), MAX_PACK_SIZE) == 1)
			return FALSE;
#endif

	    if ( _init_box (USART_mRpool, sizeof(USART_mRpool), MAX_PACK_SIZE) == 1)
	      return FALSE;
	}

	os_mbx_init (USART_MBX_TX[ctrl0], sizeof(USART_MBX_TX[ctrl0]));
	os_mbx_init (USART_MBX_RX[ctrl0], sizeof(USART_MBX_RX[ctrl0]));
	
	UASRT_RecvDataLen[ctrl0] = 0;

	//设置要开启的中断等
	switch(ctrl_remap)	
	{
	case (USART_1 | NO_REMAP): 
		{
			RCC_APBxPeriph_GPIOx = RCC_APB2Periph_GPIOA;
			RCC_APBxPeriph_USARTx = RCC_APB2Periph_USART1;
			GPIO_Pin_Tx = GPIO_Pin_9;
			GPIO_Pin_Rx = GPIO_Pin_10;
			GPIOx = GPIOA;
			UsartBase = USART1;
			USARTx_IRQChannel = USART1_IRQChannel;
			break;
		}
	case (USART_1 | PARTIAL_REMAP): 
		{
			RCC_APBxPeriph_GPIOx = RCC_APB2Periph_GPIOB;
			RCC_APBxPeriph_USARTx = RCC_APB2Periph_USART1;
			GPIO_Pin_Tx = GPIO_Pin_6;
			GPIO_Pin_Rx = GPIO_Pin_7;
			GPIOx = GPIOB;
			UsartBase = USART1;
			USARTx_IRQChannel = USART1_IRQChannel;
			break;
		}
	case (USART_2 | NO_REMAP): 
		{
			RCC_APBxPeriph_GPIOx = RCC_APB2Periph_GPIOA;
			RCC_APBxPeriph_USARTx = RCC_APB1Periph_USART2;
			GPIO_Pin_Tx = GPIO_Pin_2;
			GPIO_Pin_Rx = GPIO_Pin_3;
			GPIOx = GPIOA;
			UsartBase = USART2;		
			USARTx_IRQChannel = USART2_IRQChannel;
			break;
		}
	case (USART_2 | FULL_REMAP): 
		{
			RCC_APBxPeriph_GPIOx = RCC_APB2Periph_GPIOD;
			RCC_APBxPeriph_USARTx = RCC_APB1Periph_USART2;
			GPIO_Pin_Tx = GPIO_Pin_5;
			GPIO_Pin_Rx = GPIO_Pin_6;
			GPIOx = GPIOD;
			UsartBase = USART2;		
			USARTx_IRQChannel = USART2_IRQChannel;
			break;
		}
	case (USART_3 | NO_REMAP):
		{
			RCC_APBxPeriph_GPIOx = RCC_APB2Periph_GPIOB;
			RCC_APBxPeriph_USARTx = RCC_APB1Periph_USART3;
			GPIO_Pin_Tx = GPIO_Pin_10;
			GPIO_Pin_Rx = GPIO_Pin_11;
			GPIOx = GPIOB;
			UsartBase = USART3;
			USARTx_IRQChannel = USART3_IRQChannel;
			break;
		}
	case (USART_3 | PARTIAL_REMAP):
		{
			RCC_APBxPeriph_GPIOx = RCC_APB2Periph_GPIOC;
			RCC_APBxPeriph_USARTx = RCC_APB1Periph_USART3;
			GPIO_Pin_Tx = GPIO_Pin_10;
			GPIO_Pin_Rx = GPIO_Pin_11;
			GPIOx = GPIOC;
			UsartBase = USART3;
			USARTx_IRQChannel = USART3_IRQChannel;
			break;
		}
	case (USART_3 | FULL_REMAP):
		{
			RCC_APBxPeriph_GPIOx = RCC_APB2Periph_GPIOD;
			RCC_APBxPeriph_USARTx = RCC_APB1Periph_USART3;
			GPIO_Pin_Tx = GPIO_Pin_8;
			GPIO_Pin_Rx = GPIO_Pin_9;
			GPIOx = GPIOD;
			UsartBase = USART3;
			USARTx_IRQChannel = USART3_IRQChannel;
			break;
		}
	default: return FALSE;
	}
	
	RCC_APB2PeriphClockCmd(RCC_APBxPeriph_GPIOx , ENABLE);
	
	if(UsartBase == USART1)
    	RCC_APB2PeriphClockCmd(RCC_APBxPeriph_USARTx, ENABLE);
	else
    	RCC_APB1PeriphClockCmd(RCC_APBxPeriph_USARTx, ENABLE);

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_Tx ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOx, &GPIO_InitStructure);  
    
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_Rx ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOx, &GPIO_InitStructure);       

    NVIC_InitStructure.NVIC_IRQChannel = USARTx_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  
    
    USART_Init(UsartBase, &USART_InitStructure);
	
    USART_Cmd(UsartBase, ENABLE);   
	
	return TRUE;
}

bool USART_Pull(u32 ctrl, u8 *msg, bool bRxBuf, u32 *len, U16 timeout)
{
	u8 *ptrMsg;
	USART_TypeDef *USARTx;
	u32 ctrl0 = ctrl-1;
	USARTx = GetUSARTType(ctrl);
	
	if (os_mbx_wait (USART_MBX_RX[ctrl0], (void **)&ptrMsg, timeout) == OS_R_TMO)
	{
		if(bRxBuf)
		{
			if(!bFirstIn_Buf[ctrl0] && USART_RecvOffset_Buf[ctrl0] && len != NULL)
			{
				//关中断，否则可能出问题
				USART_ITConfig(USARTx, USART_IT_RXNE, DISABLE);
				
				//再检查一次邮箱，如果有数据则执行取邮箱数据操作
				if( os_mbx_check(USART_MBX_RX[ctrl0]) < USART_RECVOBJ_NUM)
				{
					os_mbx_wait (USART_MBX_RX[ctrl0], (void **)&ptrMsg, 1);
					memcpy(msg, ptrMsg, UASRT_RecvDataLen[ctrl0]);

					if (_free_box (USART_mRpool, ptrMsg) == 1)
					{
						return FALSE;
					}

					return TRUE;
				}
				else
				{
					//如果没有数据，则执行直接取数据操作
					memcpy(msg, ptrRxMsg_Buf[ctrl0], USART_RecvOffset_Buf[ctrl0]);
					
					if(_free_box (USART_mRpool, ptrRxMsg_Buf[ctrl0]) == 1)
					{
						return FALSE;
					}
				}					
				
				*len = USART_RecvOffset_Buf[ctrl0];
				bFirstIn_Buf[ctrl0] = TRUE;
				UASRT_Signal[ctrl0] = TRUE; 
				USART_RecvOffset_Buf[ctrl0] = 0;
				
				//开中断		
				USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
			}
		}
		return FALSE;
	}
	
	memcpy(msg, ptrMsg, UASRT_RecvDataLen[ctrl0]);
	
	if (_free_box (USART_mRpool, ptrMsg) == 1)
	{
		return FALSE;
	}

	return TRUE;
}


bool CheckCANMsgAvailable(CAN_msg *msg)
{
	//检查格式
	if(msg->format != 0 && msg->format != 1)
		return FALSE;
	
	//检查类型
	if(msg->type != 0 && msg->type != 1)
		return FALSE;
		
	//检查数据长度
	if((msg->len) > 8)
		msg->len = 8;
	
	return TRUE;
}


bool USART_AnalyzePackage(u32 ctrl, u8 *buf, u8 data)
{
	static u8 *USART_BufBegin[USART_NUM];
	static u8 *USART_BufStart[USART_NUM];
	static u8 USART_LastByte[USART_NUM] = {0};
	static bool USART_BeginFlag[USART_NUM] = {FALSE};
	static bool USART_CtrlFlag[USART_NUM] = {FALSE};
    static u8 CheckSum[USART_NUM] = {0};
	static u32 USART_RevOffset[USART_NUM] = {0};
	
	u32 ctrl0 = ctrl-1;

	if(USART_RevOffset[ctrl0] > (sizeof(CAN_msg) + 2))
	{
		USART_RevOffset[ctrl0] = (sizeof(CAN_msg) + 5);
	}

	if( (data==USART_FRAMEHEAD)&&(USART_LastByte[ctrl0]==USART_FRAMEHEAD) || (USART_RevOffset[ctrl0] > (sizeof(CAN_msg) + 2)) )
	{
		//RESET
		CheckSum[ctrl0] = 0;
		USART_RevOffset[ctrl0] = 0;
		USART_BufBegin[ctrl0] = buf;
		USART_BufStart[ctrl0] = buf;
		USART_BeginFlag[ctrl0] = TRUE;
		USART_LastByte[ctrl0] = data ;
		return FALSE;
	}
	if( (data==USART_FRAMETAIL)&&(USART_LastByte[ctrl0]==USART_FRAMETAIL)&&USART_BeginFlag[ctrl0] )
	{
		//Signal
		UASRT_RecvDataLen[ctrl0] = USART_BufBegin[ctrl0] - USART_BufStart[ctrl0] - 1;	//真实数据长度应该去掉最后一个55
        CheckSum[ctrl0] -= USART_FRAMETAIL;
        CheckSum[ctrl0] -= USART_BufStart[ctrl0][UASRT_RecvDataLen[ctrl0] -1];			//减去校验和
        USART_LastByte[ctrl0] = data;
        USART_BeginFlag[ctrl0] = FALSE;
        if((u8)CheckSum[ctrl0] == USART_BufStart[ctrl0][UASRT_RecvDataLen[ctrl0] -1])
        {                          		   		
    		CheckSum[ctrl0] = 0;
            UASRT_RecvDataLen[ctrl0] -- ;
            UASRT_Signal[ctrl0] = TRUE;  
			
			if(UASRT_RecvDataLen[ctrl0] != sizeof(CAN_msg))
			{
				return FALSE;
			}
			 
    	 	return TRUE;
        }
        CheckSum[ctrl0] = 0;                    
        return FALSE;
	}
	USART_LastByte[ctrl0] = data ;
	if(USART_BeginFlag[ctrl0])
	{
		if(USART_CtrlFlag[ctrl0])
		{
		    *(USART_BufBegin[ctrl0]++) = data;
			USART_RevOffset[ctrl0] ++;
            CheckSum[ctrl0] += data;
			USART_CtrlFlag[ctrl0] = FALSE;
			USART_LastByte[ctrl0] = USART_FRAMECTRL;
		}
		else if(data == USART_FRAMECTRL)
		{
		    USART_CtrlFlag[ctrl0] = TRUE;
		}
		else
		{
		    *(USART_BufBegin[ctrl0]++) = data;
            CheckSum[ctrl0] += data;
			USART_RevOffset[ctrl0] ++;
		}
	}

	return FALSE;
}

//接收中断调用的函数
bool USART_RxCallBack(u32 ctrl, u8 data)
{
	u32 ctrl0;
	static u8 *ptrMsg[USART_NUM];
	static bool bFirstIn[USART_NUM] = {TRUE, TRUE, TRUE};
	ctrl0 = ctrl - 1;
	
	if(UASRT_UsePack[ctrl0] == TRUE)
	{
		if(bFirstIn[ctrl0])
		{
			ptrMsg[ctrl0] = _alloc_box(USART_mRpool);
			
			if(ptrMsg[ctrl0] == NULL)
			{
				bFirstIn[ctrl0] = FALSE;
				return FALSE;
			}
				
			bFirstIn[ctrl0] = FALSE;
			return USART_AnalyzePackage(ctrl, ptrMsg[ctrl0], data);
		}
		else
		{
			if(USART_AnalyzePackage(ctrl, ptrMsg[ctrl0], data))
			{
				if(isr_mbx_check (USART_MBX_RX[ctrl0]) != 0)
				{
					isr_mbx_send (USART_MBX_RX[ctrl0], ptrMsg[ctrl0]);
					bFirstIn[ctrl0] = TRUE;
					return TRUE;
				}
			}
			return FALSE;
		}
	}
	else
	{
		return USART_FillRxBuf(ctrl, data);
	}
}

//填充Buf，用于接收Buf
bool USART_FillRxBuf(u32 ctrl, u8 data)
{
	u32 ctrl0;
	ctrl0 = ctrl - 1;

	if(bFirstIn_Buf[ctrl0])
	{
		ptrRxMsg_Buf[ctrl0] = _alloc_box(USART_mRpool);
		
		if(ptrRxMsg_Buf[ctrl0] == NULL)
		{
			bFirstIn_Buf[ctrl0] = TRUE;
			return FALSE;
		}
		
		bFirstIn_Buf[ctrl0] = FALSE;
	}
	
	ptrRxMsg_Buf[ctrl0][USART_RecvOffset_Buf[ctrl0]++] = data;
	
	if(USART_RecvOffset_Buf[ctrl0] >= UASRT_RecvDataLen[ctrl0])
	{
		USART_RecvOffset_Buf[ctrl0] = 0;
		UASRT_Signal[ctrl0] = TRUE; 
		bFirstIn_Buf[ctrl0] = TRUE;
		
		if(isr_mbx_check (USART_MBX_RX[ctrl0]) != 0)
		{
			isr_mbx_send (USART_MBX_RX[ctrl0], ptrRxMsg_Buf[ctrl0]);

			return TRUE;
		}
	}

    return FALSE;
}

//发送中断调用的函数
void USART_TxCallBack(u32 ctrl)
{
	u32 ctrl0;
	static u16 TxCnt[USART_NUM] = {2, 2, 2};
	static bool bFirstIn[USART_NUM]  = {TRUE, TRUE, TRUE};
	static u8 *ptrMsg[USART_NUM];
	static u16 TxTotal[USART_NUM];
	
	USART_TypeDef *USARTx = GetUSARTType(ctrl);
	ctrl0 = ctrl-1;
	
	if(bFirstIn[ctrl0] == TRUE)
	{
		//初始化ptrMsg
		if(isr_mbx_receive (USART_MBX_TX[ctrl0], (void **)&ptrMsg[ctrl0]) != OS_R_OK)
		{
			//初始化需要发送的数据数量
			TxCnt[ctrl0] = 2;
			TxTotal[ctrl0] = *((u16*)ptrMsg[ctrl0]);
			
			USART_SendData(USARTx, ptrMsg[ctrl0][TxCnt[ctrl0]++]);
			
			bFirstIn[ctrl0] = FALSE;
		}
		else
		{
			//关闭发送中断
	        USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);
			return;
		}
	}
	else
	{
		if(TxCnt[ctrl0] >= TxTotal[ctrl0])
		{
			_free_box(USART_mTpool[ctrl0], ptrMsg[ctrl0]);
			TxCnt[ctrl0] = 2;
			TxTotal[ctrl0] = 0;
			bFirstIn[ctrl0] = TRUE;
			
			isr_evt_set (TX_EVENT(ctrl), tx_task[ctrl0]);
			return ;
		}			
		
		USART_SendData(USARTx, ptrMsg[ctrl0][TxCnt[ctrl0]++]);
	}
}

/////////////////////////////////////////////////////////////////////////
//User Interface
/////////////////////////////////////////////////////////////////////////
bool USART_SendPackage(u32 ctrl, s16 packid, u8 *data, u16 n, u16 timeout)
{
	s16 i;
	u8 *pBuf;
	u8 *pBufBase;
	u16 *pSendNum;
    u8 CheckSum = 0;
	USART_TypeDef *USARTx;
	u32 ctrl0 = ctrl - 1;
	
	USART_LOCK(ctrl0);
	
	USARTx = GetUSARTType(ctrl);
	
	tx_task[ctrl0] = os_tsk_self ();
	
	//阻塞在这里等待邮箱
	pBuf = _alloc_box(USART_mTpool[ctrl0]);
	while((pBuf) == NULL)
	{
		os_evt_clr (TX_EVENT(ctrl), tx_task[ctrl0]);
		if( os_evt_wait_or(TX_EVENT(ctrl), timeout ) == OS_R_EVT )
			pBuf = _alloc_box(USART_mTpool[ctrl0]);
		else
		{
			USART_UNLOCK(ctrl0);
			return FALSE;
		}
	}

	pBufBase = pBuf;
	pSendNum = (u16*)pBuf;
	
	pBuf += 2;
    *pBuf++ = USART_FRAMEHEAD;
	*pBuf++ = USART_FRAMEHEAD;
	(*pSendNum) = n;
	(*pSendNum) += 4;			//两个USART_FRAMEHEAD和自身16字节数据
	
    if(packid >= 0)
    {
        //Set ID
        if (packid==USART_FRAMECTRL||packid==USART_FRAMEHEAD||packid==USART_FRAMETAIL)
    	{
    		*pBuf++ = USART_FRAMECTRL;
			(*pSendNum) ++;
    	}
        *pBuf++ = packid;
        CheckSum += packid;
    }

	for (i=0;i<n;i++)
	{
		if (data[i]==USART_FRAMECTRL||data[i]==USART_FRAMEHEAD||data[i]==USART_FRAMETAIL)
		{
			*pBuf++ = USART_FRAMECTRL;
			(*pSendNum) ++;
		}
		*pBuf++ = data[i];
        CheckSum += data[i];
	}

    //校验和
    if (CheckSum==USART_FRAMECTRL||CheckSum==USART_FRAMEHEAD||CheckSum==USART_FRAMETAIL)
	{
		*pBuf++ = USART_FRAMECTRL;
		(*pSendNum)++;
	}
    *pBuf++ = CheckSum;
	(*pSendNum) ++;

	//Send Tail USART_FRAMETAIL USART_FRAMETAIL
	*pBuf++ = USART_FRAMETAIL;
	*pBuf++ = USART_FRAMETAIL;
	(*pSendNum) += 2;
	
	//发送至邮箱
	if (os_mbx_send (USART_MBX_TX[ctrl0], pBufBase, timeout) == OS_R_TMO) 
	{
		_free_box (USART_mTpool[ctrl0], pBufBase);
		USART_UNLOCK(ctrl0);
		return FALSE;
	}
	
	//使能发送中断
	USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
	USART_UNLOCK(ctrl0);
	
    return TRUE;
}


void USART_CreatePackage(u8 *pBuf, u8 *data, u16 n)
{
	s16 i;
	u16 *pSendNum;
    u8 	CheckSum = 0;
	
	pSendNum = (u16*)pBuf;
	pBuf += 2;
    *pBuf++ = USART_FRAMEHEAD;
	*pBuf++ = USART_FRAMEHEAD;
	(*pSendNum) = n;
	(*pSendNum) += 4;			//两个USART_FRAMEHEAD和自身16字节数据
	
	for (i=0;i<n;i++)
	{
		if (data[i]==USART_FRAMECTRL||data[i]==USART_FRAMEHEAD||data[i]==USART_FRAMETAIL)
		{
			*pBuf++ = USART_FRAMECTRL;
			(*pSendNum) ++;
		}
		*pBuf++ = data[i];
        CheckSum += data[i];
	}

    //校验和
    if (CheckSum==USART_FRAMECTRL||CheckSum==USART_FRAMEHEAD||CheckSum==USART_FRAMETAIL)
	{
		*pBuf++ = USART_FRAMECTRL;
		(*pSendNum)++;
	}
    *pBuf++ = CheckSum;
	(*pSendNum) ++;

	//Send Tail USART_FRAMETAIL USART_FRAMETAIL
	*pBuf++ = USART_FRAMETAIL;
	*pBuf++ = USART_FRAMETAIL;
	(*pSendNum) += 2;
}

u16 USART_RecvPackage(u32 ctrl, u8 *data, u16 timeout)
{
	USART_TypeDef *USARTx;
	u32 ctrl0 = ctrl-1;
	USARTx = GetUSARTType(ctrl);
	
	USART_LOCK(ctrl0);
	UASRT_UsePack[ctrl0] = TRUE;
	
	USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
	if(!USART_Pull(ctrl, data, FALSE, NULL, timeout))
	{
		USART_UNLOCK(ctrl0);
		return 0;
	}
	
	USART_UNLOCK(ctrl0);
	
	return UASRT_RecvDataLen[ctrl0];
}

u16 USART_RecvBuf(u32 ctrl, u8 *buf, u16 n, u16 timeout)
{
	u32 bufNum, i, ctrl0, bufLen, left;
	USART_TypeDef *USARTx;
	ctrl0 = ctrl-1;	
	bufNum = n / MAX_PACK_SIZE;
	left = n - bufNum * MAX_PACK_SIZE;
	
	USART_LOCK(ctrl0);
	USARTx = GetUSARTType(ctrl);
	UASRT_UsePack[ctrl0] = FALSE;
	
	for(i=0; i<bufNum; i++)
	{
		//USART_ITConfig(USARTx, USART_IT_RXNE, DISABLE);
		UASRT_RecvDataLen[ctrl0] = MAX_PACK_SIZE;
		USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
		bufLen = 0;
		if(!USART_Pull(ctrl, buf+i*MAX_PACK_SIZE, TRUE, &bufLen, timeout))
		{
			USART_UNLOCK(ctrl0);
			return i*MAX_PACK_SIZE + bufLen;
		}
	}
	
	if(left)
	{
		//USART_ITConfig(USARTx, USART_IT_RXNE, DISABLE);
		UASRT_RecvDataLen[ctrl0] = left;
		USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
		bufLen = 0;
		if(!USART_Pull(ctrl, buf + bufNum*MAX_PACK_SIZE, TRUE, &bufLen, timeout))
		{
			USART_UNLOCK(ctrl0);
			return i*MAX_PACK_SIZE + bufLen;
		}
		//USART_ITConfig(USARTx, USART_IT_RXNE, DISABLE);
		if(n > MAX_PACK_SIZE)
			UASRT_RecvDataLen[ctrl0] = MAX_PACK_SIZE;
		USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
	}
	
	USART_UNLOCK(ctrl0);
	return n;
}

bool USART_SendBuf(u32 ctrl, u8 *buf, u16 n, u16 timeout)
{
	u32 realPackSize;
	u32 bufNum;
	u32 i;
	u32 ctrl0;
	u32 leftsize;
	u8 *pbufdata;
	USART_TypeDef *USARTx;
	
	realPackSize = MAX_PACK_SIZE-2;
	bufNum = n / realPackSize;
	ctrl0 = ctrl - 1;
	USARTx = GetUSARTType(ctrl);
	leftsize  = n - bufNum * realPackSize;
	
	USART_LOCK(ctrl0);
	
	tx_task[ctrl0] = os_tsk_self();
	pbufdata = buf;
	for(i=0; i<bufNum; i++)
	{
		u8 *ptrMsg;
		u16 *pSendNum;
		
		//阻塞在这里等待邮箱
		ptrMsg = _alloc_box(USART_mTpool[ctrl0]);
		while((ptrMsg) == NULL)
		{
			os_evt_clr (TX_EVENT(ctrl), tx_task[ctrl0]);
			if( os_evt_wait_or(TX_EVENT(ctrl), timeout ) == OS_R_EVT )
				ptrMsg = _alloc_box(USART_mTpool[ctrl0]);
			else
			{
				USART_UNLOCK(ctrl0);
				return FALSE;
			}
		}
		
		pSendNum = (u16*)ptrMsg;
		*pSendNum = realPackSize + 2;
		
		memcpy(ptrMsg+2, pbufdata, realPackSize);
		pbufdata += realPackSize;
		
		if (os_mbx_send (USART_MBX_TX[ctrl0], ptrMsg, timeout) == OS_R_TMO) 
		{
			_free_box (USART_mTpool[ctrl0], ptrMsg);
			USART_UNLOCK(ctrl0);
			return FALSE;
		}
		
		USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
	}

	if(leftsize > 0)
	{
		u8 *ptrMsgLeft;
		u16 *pSendNum;
		
		//阻塞在这里等待邮箱
		ptrMsgLeft = _alloc_box(USART_mTpool[ctrl0]);
		while(ptrMsgLeft == NULL)
		{
			os_evt_clr (TX_EVENT(ctrl), tx_task[ctrl0]);
			if( os_evt_wait_or(TX_EVENT(ctrl), timeout ) == OS_R_EVT )
			{
				ptrMsgLeft = _alloc_box(USART_mTpool[ctrl0]);
			}
			else
			{
				USART_UNLOCK(ctrl0);
				return FALSE;
			}
		}

		pSendNum = (u16*)ptrMsgLeft;
		*pSendNum = leftsize + 2;
			
		memcpy(ptrMsgLeft+2, pbufdata, leftsize);
		
		if (os_mbx_send (USART_MBX_TX[ctrl0], ptrMsgLeft, timeout) == OS_R_TMO) 
		{
			_free_box (USART_mTpool[ctrl0], ptrMsgLeft);
			USART_UNLOCK(ctrl0);
			return FALSE;
		}
		
		USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
	}
	
	USART_UNLOCK(ctrl0);
	
	return TRUE;
}

void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        USART_RxCallBack(1, USART_ReceiveData(USART1));
    }
        
    if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
    {   
        USART_TxCallBack(1);
    }
}

u32 RxCnt = 0;
void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        USART_RxCallBack(2, USART_ReceiveData(USART2));
		RxCnt ++;
    }
        
    if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
    {   
        USART_TxCallBack(2);   
    }
}

void USART3_IRQHandler(void)
{
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        USART_RxCallBack(3, USART_ReceiveData(USART3));
    }
        
    if(USART_GetITStatus(USART3, USART_IT_TXE) != RESET)
    {
        USART_TxCallBack(3);   
    }
}

USART_TypeDef* GetUSARTType(u32 USART_ID)
{
	switch(USART_ID)
	{
	case 1: return USART1;
	case 2: return USART2;
	case 3: return USART3;
	default: return NULL;
	}
}

