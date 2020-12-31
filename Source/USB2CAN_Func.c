
#include "USB2CAN_Func.h"
#include "CAN_Hw.h"
#include "RTX_CAN.h"
#include "FLASH.h"
#include "USART.h"

u32			USB2CAN_Ver = CAN_INIT_VER;
bool  		USB2CAN_First = TRUE;				//是否是第一次启动
//检查FLASH中是否有数据
bool CheckFLASH()
{
	u32 start;
	FLASH_read(&start, StartAddr, 4);
	
	if(start == CAN_PARM_START)
	{
		return TRUE;
	}
	else
		return FALSE;
}

//从FLASH中初始化USART
void USARTInitFromFLASH(u8 ctrl_remap)
{
	u32 parm[CAN_PARM_NUM];
	FLASH_read(parm, StartAddr, sizeof(parm));

	if(parm[USART_ADDR_BAUD] == 0xFFFFFFFF)
	{
		parm[USART_ADDR_BAUD] = 115200;
		FLASH_write(StartAddr, parm, sizeof(parm));
	}
	USART_Initial(ctrl_remap, parm[USART_ADDR_BAUD]);	
}

//擦除FLASH
bool CleanFlash(CAN_msg msg)
{
	return CANInitFLASH();
}

//如果是第一次配置，FLASH中也没数据，则初始化FLASH和全局波特率
bool CANInitFLASH(void)
{
	u32 parm[CAN_PARM_NUM - 1];	   //不能配置USART的波特率
	u8 i;
	CAN_msg msg;
	msg.id = CAN_INIT_ID;
	msg.len = 8;
	msg.ch = CAN_CONFIG_CHANNEL;
	msg.format = EXTENDED_FORMAT;
	
	parm[CAN_ADDR_START] = CAN_PARM_START;
	parm[CAN_ADDR_BAUD] = CAN_INIT_BAUD;

	//使能一个标准帧过滤器
	parm[0 + CAN_ADDR_FILTER] = 0x01;
	parm[1 + CAN_ADDR_FILTER] = 0;
	parm[2 + CAN_ADDR_FILTER] = 0;

	//使能一个扩展帧过滤器
	parm[3 + CAN_ADDR_FILTER] = 0x03;
	parm[4 + CAN_ADDR_FILTER] = 0;
	parm[5 + CAN_ADDR_FILTER] = 0;

	for(i=2; i<14; i++)
	{
		parm[i*3 + CAN_ADDR_FILTER] = CAN_INIT_ENA;
		parm[i*3 + CAN_ADDR_FILTER + 1] = CAN_INIT_MASK_ID;
		parm[i*3 + CAN_ADDR_FILTER + 2] = CAN_INIT_MASK;
	}
	
	parm[CAN_ADDR_ABOM] = CAN_INIT_ABOM;
	parm[CAN_ADDR_ART] = CAN_INIT_ART;
	parm[CAN_ADDR_VER] = CAN_INIT_VER;
	
	FLASH_write(StartAddr, parm, sizeof(parm));
	
	return CANInitFromUSART(parm, sizeof(parm), msg);
}

//从USART配置CAN参数
bool CANInitFromUSART(u32 *parm, u32 size, CAN_msg msg)
{
	u32 i;
	u32 Mask_Id;
	u32 Mask;
	bool fEna;
	CAN_FORMAT format;
	u32 temp;  
	
	
	//重新初始化
	if(USB2CAN_First)
	{
		CAN_init_ex(1, parm[CAN_ADDR_BAUD], parm[CAN_ADDR_ART]);
	}
	else
	{
		CAN_reinit_ex(1, parm[CAN_ADDR_BAUD], parm[CAN_ADDR_ART]);
	}
	
	for(i=0; i<14; i++)
	{
		temp = parm[i*3+CAN_ADDR_FILTER];
		fEna = (bool)(temp & 1);
		format = (CAN_FORMAT)(temp & 2);

		Mask_Id = parm[i*3+CAN_ADDR_FILTER+1];
		Mask = parm[i*3+CAN_ADDR_FILTER+2];

		CAN_rx_object_mask_idx(1, 0, Mask_Id, Mask, i, fEna, format);
	}
	
	CAN_start_ex(1, parm[CAN_ADDR_ABOM]);
	USB2CAN_First = FALSE;
	
	//返回正确的应答消息
	msg.type = REMOTE_FRAME;
	return USART_SendPackage(USART_2, -1, (u8 *)&msg, sizeof(msg), 0xffff);
}

//从FLASH配置CAN参数
bool CANInitFromFLASH(void)
{
	u8 i;
	u32 parm[CAN_PARM_NUM];
	u32 Mask_Id;
	u32 Mask;
	bool fEna;
	CAN_FORMAT format;
	u32 temp;  

	FLASH_read(parm, StartAddr, sizeof(parm));
		
	USB2CAN_Ver = parm[CAN_ADDR_VER];
	CAN_init_ex(1, parm[CAN_ADDR_BAUD], (bool)parm[CAN_ADDR_ART]);
	for(i=0; i<14; i++)
	{		 
		temp = parm[i*3+CAN_ADDR_FILTER];
		fEna = (bool)(temp & 1);
		format = (CAN_FORMAT)(temp & 2);
		Mask_Id = parm[i*3+CAN_ADDR_FILTER+1];
		Mask = parm[i*3+CAN_ADDR_FILTER+2];
		
		CAN_rx_object_mask_idx(1, 0, Mask_Id, Mask, i, fEna, format);	//format:0,standard;1,extended
	}

	CAN_start_ex(1, parm[CAN_ADDR_ABOM]);
		
	USB2CAN_First = FALSE;
	return TRUE;
}


//获取FILTER的参数
bool ReturnFilterInfo(u32 msgID)
{
	u32 fInfo[3];
	CAN_msg msg;
	u8 idx;
	
	idx = msgID & CAN_FILTER_IDX_MASK;
	FLASH_read(fInfo, StartAddr + sizeof(u32)*(idx*3+2), sizeof(fInfo));
	
	msg.id = msgID;
	msg.len = 8;
	msg.ch = CAN_CONFIG_CHANNEL;
	msg.format = EXTENDED_FORMAT;
	msg.type = DATA_FRAME;
	
	if(fInfo[0] & 1)
	{
		fInfo[1] += CAN_FILTER_EN;
	}
	else
	{
		fInfo[1] += CAN_FILTER_DIS;
	}
	
	if(fInfo[0] & 2)
	{
		fInfo[1] += CAN_FORMAT_EXT;
	}
	else
	{
		fInfo[1] += CAN_FORMAT_STD;
		
	}

	memcpy(msg.data, &fInfo[1], sizeof(u32));	
	memcpy(msg.data+4, &fInfo[2], sizeof(u32));
	
	return USART_SendPackage(USART_2, -1, (u8 *)&msg, sizeof(msg), 0xffff);
}

bool SetFilterInfo(CAN_msg msg)
{
	u32 parm[CAN_PARM_NUM];
	u32 Mask_Id;
	u32 Mask;
	bool fEna;
	u32 format;
	u32 fidx;
	
	fidx = msg.id & CAN_FILTER_IDX_MASK;
	fEna = (bool)(((*((u32 *)msg.data)) & CAN_FILTER_EN) >> 31);	
	format = ((*((u32 *)(msg.data+4))) & CAN_FORMAT_EXT) >> 30;

	if(format == EXTENDED_FORMAT)
	{
		Mask_Id = (*((u32 *)msg.data)) & 0x1FFFFFFF;
		Mask = (*((u32 *)(msg.data+4))) & 0x1FFFFFFF;
	}
	else
	{
		Mask_Id = (*((u32 *)msg.data)) & 0x7FF;
		Mask = (*((u32 *)(msg.data+4))) & 0x7FF;
	}
	
	FLASH_read(parm, StartAddr, sizeof(parm));
	parm[CAN_ADDR_START] = CAN_PARM_START;
	parm[fidx*3 + CAN_ADDR_FILTER] = (format<<1) + fEna;
	parm[fidx*3 + CAN_ADDR_FILTER + 1] = Mask_Id;
	parm[fidx*3 + CAN_ADDR_FILTER + 2] = Mask;
	
		//写入FLASH
	if(!(bool)FLASH_write(StartAddr, parm, sizeof(parm)))
	{
		return FALSE;
	}
	
	return CANInitFromUSART(parm, sizeof(parm), msg);
}

bool ReturnBaudrate()
{
	CAN_msg msg;
	u32 parm[CAN_PARM_NUM];
	FLASH_read(parm, StartAddr, sizeof(parm));
	
	msg.id = CAN_BAUDRATE_ID;
	msg.len = 8;
	msg.format = EXTENDED_FORMAT;
	msg.type = DATA_FRAME;
	msg.ch = CAN_CONFIG_CHANNEL;
	memcpy(msg.data, &parm[CAN_ADDR_BAUD], sizeof(u32));

	return USART_SendPackage(USART_2, -1, (u8 *)&msg, sizeof(msg), 0xffff);
}

bool SetBaudrate(CAN_msg msg)
{
	u32 parm[CAN_PARM_NUM];
	u32 baud;
	
	baud = *((u32 *)msg.data);
	
	FLASH_read(parm, StartAddr, sizeof(parm));
	parm[CAN_ADDR_START] = CAN_PARM_START;
	parm[CAN_ADDR_BAUD] = baud;
	
	//写入FLASH
	if(!(bool)FLASH_write(StartAddr, parm, sizeof(parm)))
	{
		return FALSE;
	}
	
	return CANInitFromUSART(parm, sizeof(parm), msg);
}

//获取CPU信息
bool ReturnCPU0()
{
	u32 SysInfo[2];
	CAN_msg msg;
	
	SysInfo[0] = *(u32*)(0x1FFFF7E8);
	SysInfo[1] = *(u32*)(0x1FFFF7EC);
	
	msg.id = CAN_CPU_INFO0_ID;
	msg.len = 8;
	msg.format = EXTENDED_FORMAT;
	msg.type = DATA_FRAME;
	msg.ch = CAN_CONFIG_CHANNEL;
	memcpy(msg.data, &SysInfo[0], sizeof(u32));
	memcpy(msg.data+4, &SysInfo[1], sizeof(u32));
	
	return USART_SendPackage(USART_2, -1, (u8 *)&msg, sizeof(msg), 0xffff);
}

bool ReturnCPU1()
{
	u32 SysInfo;
	CAN_msg msg;
	
	SysInfo = *(u32*)(0x1FFFF7F0);

	msg.id = CAN_CPU_INFO1_ID;
	msg.len = 8;
	msg.format = EXTENDED_FORMAT;
	msg.type = DATA_FRAME;
	msg.ch = CAN_CONFIG_CHANNEL;
	memcpy(msg.data, &SysInfo, sizeof(u32));
	
	return USART_SendPackage(USART_2, -1, (u8 *)&msg, sizeof(msg), 0xffff);
}

bool ReturnVersion(void)
{
	CAN_msg msg;
	
	msg.id = CAN_VERSION_ID;
	msg.len = 8;
	msg.format = EXTENDED_FORMAT;
	msg.type = DATA_FRAME;
	msg.ch = CAN_CONFIG_CHANNEL;
	memcpy(msg.data, &USB2CAN_Ver, sizeof(USB2CAN_Ver));

	return USART_SendPackage(USART_2, -1, (u8 *)&msg, sizeof(msg), 0xffff);
}

//获取和设置ART
bool ReturnART(void)
{
	CAN_msg msg;
	u32 parm[CAN_PARM_NUM];
	FLASH_read(parm, StartAddr, sizeof(parm));
	
	msg.id = CAN_ART_ID;
	msg.len = 8;
	msg.format = EXTENDED_FORMAT;
	msg.type = DATA_FRAME;
	msg.ch = CAN_CONFIG_CHANNEL;
	msg.data[0] = (u8)parm[CAN_ADDR_ART];

	return USART_SendPackage(USART_2, -1, (u8 *)&msg, sizeof(msg), 0xffff);
}

bool SetART(CAN_msg msg)
{
	u32 parm[CAN_PARM_NUM];
		
	FLASH_read(parm, StartAddr, sizeof(parm));
	parm[CAN_ADDR_START] = CAN_PARM_START;
	parm[CAN_ADDR_ART] = (bool)msg.data[0];;
	
	//写入FLASH
	if(!(bool)FLASH_write(StartAddr, parm, sizeof(parm)))
	{
		return FALSE;
	}
	return CANInitFromUSART(parm, sizeof(parm), msg);
}

//获取和设置ABOM
bool ReturnABOM(void)
{
	CAN_msg msg;
	u32 parm[CAN_PARM_NUM];
	FLASH_read(parm, StartAddr, sizeof(parm));
	
	msg.id = CAN_ABOM_ID;
	msg.len = 8;
	msg.format = EXTENDED_FORMAT;
	msg.type = DATA_FRAME;
	msg.ch = CAN_CONFIG_CHANNEL;
	msg.data[0] = (u8)parm[CAN_ADDR_ABOM];

	return USART_SendPackage(USART_2, -1, (u8 *)&msg, sizeof(msg), 0xffff);
}

bool SetABOM(CAN_msg msg)
{
	u32 parm[CAN_PARM_NUM];
	
	FLASH_read(parm, StartAddr, sizeof(parm));
	parm[CAN_ADDR_START] = CAN_PARM_START;
	parm[CAN_ADDR_ABOM] = (bool)msg.data[0];

	//写入FLASH
	if(!(bool)FLASH_write(StartAddr, parm, sizeof(parm)))
	{
		return FALSE;
	}
	
	return CANInitFromUSART(parm, sizeof(parm), msg);
}

//重启CAN
bool RemoteResetCan(CAN_msg msg)
{
	if(!USB2CAN_First)
	{	
		u32 parm[CAN_PARM_NUM];
		
		FLASH_read(parm, StartAddr, sizeof(parm));
		return CANInitFromUSART(parm, sizeof(parm), msg);	
	}
	else
	{
		return FALSE;
	}
}

//获取USART的波特率
bool ReturnUSARTBaud(void)
{
	CAN_msg msg;
	u32 parm[CAN_PARM_NUM];
	FLASH_read(parm, StartAddr, sizeof(parm));
	
	msg.id = USART_BAUDRATE_ID;
	msg.len = 8;
	msg.format = EXTENDED_FORMAT;
	msg.type = DATA_FRAME;
	msg.ch = CAN_CONFIG_CHANNEL;
	memcpy(msg.data, &parm[USART_ADDR_BAUD], sizeof(u32));

	return USART_SendPackage(USART_2, -1, (u8 *)&msg, sizeof(msg), 0xffff);
}

//设置USART的波特率
bool SetUSARTBaud(CAN_msg msg)
{
	u32 parm[CAN_PARM_NUM];
	u32 baud;
	
	baud = *((u32 *)msg.data);
	
	FLASH_read(parm, StartAddr, sizeof(parm));
	parm[CAN_ADDR_START] = CAN_PARM_START;
	parm[USART_ADDR_BAUD] = baud;
	
	//写入FLASH
	if(!(bool)FLASH_write(StartAddr, parm, sizeof(parm)))
	{
		return FALSE;
	}
	
	msg.type = REMOTE_FRAME;
	return USART_SendPackage(USART_2, -1, (u8 *)&msg, sizeof(msg), 0xffff);;
}

void CAN_Operator(CAN_msg configMsg)
{
	if(configMsg.ch == CAN_CONFIG_CHANNEL)
	{
		//操作FILTER
		if(((configMsg.id << 4) >> 8) == (CAN_FILTER_BASE_ID >> 4))
		{
			//判断是远程帧还是数据帧，如果是远程帧就返回数据，如果是数据帧就设置
			if(configMsg.type == REMOTE_FRAME)
			{
				ReturnFilterInfo(configMsg.id);
			}
			else
			{
				SetFilterInfo(configMsg);
			}
		}
		else
		{
			switch(configMsg.id)
			{
			case CAN_BAUDRATE_ID:		//操作波特率
				if(configMsg.type == REMOTE_FRAME)
				{
					ReturnBaudrate();
				}
				else
				{
					SetBaudrate(configMsg);
				}
				break;
				
			case CAN_CPU_INFO0_ID:		//操作CPU
				if(configMsg.type == REMOTE_FRAME)
				{
					ReturnCPU0();
				}
				break;
				
			case CAN_CPU_INFO1_ID:		//操作CPU
				if(configMsg.type == REMOTE_FRAME)
				{
					ReturnCPU1();
				}
				break;
				
			case CAN_VERSION_ID:		//操作Version
				if(configMsg.type == REMOTE_FRAME)
				{
					ReturnVersion();
				}
				break;
				
			case CAN_ABOM_ID:			//操作ABOM
				if(configMsg.type == REMOTE_FRAME)
				{
					ReturnABOM();
				}
				else
				{
					SetABOM(configMsg);
				}
				break;
			case CAN_ART_ID:			//操作ART
				if(configMsg.type == REMOTE_FRAME)
				{
					ReturnART();
				}
				else
				{
					SetART(configMsg);
				}
				break;
			case CAN_RESET_ID:			//重启CAN
				if(configMsg.type == DATA_FRAME)
				{
					RemoteResetCan(configMsg);
				}
				break;
			case CAN_INIT_ID:	//恢复出厂设置
				if(configMsg.type == DATA_FRAME)
				{
					CleanFlash(configMsg);
				}
				break;
			case USART_BAUDRATE_ID:
				if(configMsg.type == REMOTE_FRAME)
				{
					ReturnUSARTBaud();
				}
				else
				{
					SetUSARTBaud(configMsg);
				}
				break;
			default:break;
			}
		}
	}
	else
	{
		if(!USB2CAN_First){
			if(configMsg.type == 0)
				CAN_send(1, &configMsg, 0xffff);
			else
				CAN_request(1, &configMsg, 0xffff);
		}
	}
}
