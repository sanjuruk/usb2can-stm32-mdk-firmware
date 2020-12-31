#include <RTL.h>

#include "Tasks.h"
#include "RTX_CAN.h"
#include "CAN_Hw.h"
#include "USB2CAN_Func.h"
#include "USART.h"
#include "Flash.h"

extern bool USB2CAN_First;
extern u32 	USB2CAN_Baudrate;
extern u32 	USB2CAN_Mask;
extern u32 	USB2CAN_Mask_Id;
extern u32 	USB2CAN_FilterIndex;
extern bool USB2CAN_FilterEnable;
extern u32	USB2CAN_FlashCleaned;

static bool bDataFlag = FALSE;


/*--------------------------------
		根据接收信息配置CAN
线程1：接收USART传过来的数据，如果是配置包，则配置CAN；反之，直接转发
线程2：CAN的接收线程
---------------------------------*/
OS_TID task1, task2, task3;

__task void CAN_RecvTask(void)
{
	CAN_msg msg;
	while(1)
	{
		CAN_receive(1, &msg, 0xffff);
		USART_SendPackage(USART_2, -1, (u8 *)&msg, sizeof(msg) , 0xffff);
		bDataFlag = TRUE;
	}
}

u32 t = 0;
__task void CANConfigAndSendTask(void)
{
	CAN_msg configMsg;
	u8 buf[40];
	bool bIDAndMaskInFlash = FALSE;
	
	//取FLASH中的数据
	bIDAndMaskInFlash = CheckFLASH();
	while(1)
	{
		if(bIDAndMaskInFlash && USB2CAN_First)
		{
			CANInitFromFLASH();
		}
		else
		{
			if(USB2CAN_First)
			{
				CANInitFLASH();
			}
			
			if(USART_RecvPackage(USART_2, buf, 0xffff) != 0)
			{
				if(CheckCANMsgAvailable((CAN_msg*)buf))
				{
					memcpy((u8 *)&configMsg, buf, sizeof(configMsg));
					CAN_Operator(configMsg);
					t++;
					bDataFlag = TRUE;
				}
			}
		}
	}
}

__task void LightTask(void)
{
	while(1)
	{
		if(bDataFlag)
		{
			GPIO_SetBits(GPIOA, GPIO_Pin_0);
			os_dly_wait(100);
			GPIO_ResetBits(GPIOA, GPIO_Pin_0);			   
			os_dly_wait(100);
			
			bDataFlag = FALSE;
		}
	}
}

__task void InitTask (void) 
{  
	os_tsk_prio_self(100);

	GPIO_ResetBits(GPIOA, GPIO_Pin_0);

	//CANInitFLASH();

	//串口初始化
	USARTInitFromFLASH(USART_2|NO_REMAP);
	
	//通过USB配置CAN
	task3 = os_tsk_create(LightTask, 2);
	task2 = os_tsk_create(CAN_RecvTask, 4);
	task1 = os_tsk_create(CANConfigAndSendTask, 6);
	
	
	os_tsk_delete_self ();
}







