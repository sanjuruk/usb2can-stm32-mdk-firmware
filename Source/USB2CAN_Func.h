#ifndef USB2CAN_FUNC_H
#define USB2CAN_FUNC_H

#include <stm32f10x_lib.h>
#include "RTX_CAN.h"

/*---------------内存分布--------------
			----------------------
 0x08008000:|    内存起始标识    |
			----------------------
			|                    |
			----------------------
			|                    |
			----------------------
--------------------------------------*/

/*-------------------Definition-------------------*/
/*   初始化值   */
#define CAN_INIT_ENA		0x02
#define CAN_INIT_MASK		0x1fffffff
#define CAN_INIT_MASK_ID	0x1fffffff
#define CAN_INIT_BAUD		500000
#define CAN_INIT_INDEX		0
#define	CAN_INIT_ABOM		FALSE
#define CAN_INIT_ART		FALSE
#define CAN_INIT_VER		1

/* CHANNEL */
#define CAN_CONFIG_CHANNEL	0xff					//CAN配置使用0xff通道

/*  ID  */
#define CAN_CPU_INFO0_ID		0x01fffff0				//ID: CPU信息_前两个信息
#define CAN_CPU_INFO1_ID		(CAN_CPU_INFO0_ID + 1)	//ID: CPU信息_后一个信息
#define CAN_VERSION_ID			0x01ffffe0				//ID: 获取版本信息
#define CAN_INIT_ID				0x01fffeff				//ID: CAN成功初始化
#define CAN_FILTER_BASE_ID		0x01fffee0				//ID: 获取FILTER信息（基本ID）
#define CAN_BAUDRATE_ID 		0x01fffed0				//ID: 获取波特率
#define CAN_RESET_ID			0x01fffec0				//ID: 重启CAN
#define CAN_ABOM_ID				0x01fffeb0				//ID: 设置ABOM（自动离线管理）
#define CAN_ART_ID				0x01fffea0				//ID: 设置ART（自动重传）
#define USART_BAUDRATE_ID		0x01fffe90				//ID: 设置USART波特率			

/* FILTER相关宏 */
#define CAN_FILTER_IDX_MASK	0x0000000f				//从FILTER的ID中获取INDEX
#define CAN_FILTER_EN		0x80000000				//FILTER的使能标识
#define CAN_FILTER_DIS		0						//FILTER的失能标识
#define CAN_FORMAT_EXT		0x40000000				//FORMAT是EXTENDED
#define CAN_FORMAT_STD		0						//FORMAT是STANDARD

/* FLASH相关宏 */
#define CAN_PARM_START		0xaaaaaaaa				//FLASH中标识已经存有数据
#define StartAddr 	 		((vu32)0x08008000)		//FLASH的首地址
#define EndAddr    			((vu32)0x0800C000)		//FLASH的尾地址
#define CAN_PARM_NUM		48						//FLASH中存的u32数据个数
#define CAN_ADDR_START		0						//START存储在数组中第一个元素
#define CAN_ADDR_BAUD		1						//波特率存储在数组中第二个元素
#define CAN_ADDR_FILTER		2  //-43  共42个		//FILTER存在数组中第2到第42个元素
#define CAN_ADDR_ABOM		44						//ABOM配置信息位置
#define CAN_ADDR_ART		45						//ART位置
#define CAN_ADDR_VER		46						//VERSION位置
#define USART_ADDR_BAUD		47



/*---------------Global Function--------------*/

//初始化和重新初始化使用的函数
bool CheckFLASH(void);
bool CANInitFLASH(void);
bool CANInitFromUSART(u32 *parm, u32 size, CAN_msg msg);
bool CANInitFromFLASH(void);
void USARTInitFromFLASH(u8 ctrl_remap);
void CAN_Operator(CAN_msg configMsg);

//获取和设置FILTER
bool ReturnFilterInfo(u32 msgID);
bool SetFilterInfo(CAN_msg msg);

//获取和设置波特率
bool ReturnBaudrate(void);
bool SetBaudrate(CAN_msg msg);

//获取和设置ART
bool ReturnART(void);
bool SetART(CAN_msg msg);

//获取和设置ABOM
bool ReturnABOM(void);
bool SetABOM(CAN_msg msg);

//获取CPU信息
bool ReturnCPU(void);

//获取版本信息
bool ReturnVersion(void);

//重启CAN
bool RemoteResetCan(CAN_msg msg);

//擦除FLASH
bool CleanFlash(CAN_msg msg);

#endif

