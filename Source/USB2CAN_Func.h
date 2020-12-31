#ifndef USB2CAN_FUNC_H
#define USB2CAN_FUNC_H

#include <stm32f10x_lib.h>
#include "RTX_CAN.h"

/*---------------�ڴ�ֲ�--------------
			----------------------
 0x08008000:|    �ڴ���ʼ��ʶ    |
			----------------------
			|                    |
			----------------------
			|                    |
			----------------------
--------------------------------------*/

/*-------------------Definition-------------------*/
/*   ��ʼ��ֵ   */
#define CAN_INIT_ENA		0x02
#define CAN_INIT_MASK		0x1fffffff
#define CAN_INIT_MASK_ID	0x1fffffff
#define CAN_INIT_BAUD		500000
#define CAN_INIT_INDEX		0
#define	CAN_INIT_ABOM		FALSE
#define CAN_INIT_ART		FALSE
#define CAN_INIT_VER		1

/* CHANNEL */
#define CAN_CONFIG_CHANNEL	0xff					//CAN����ʹ��0xffͨ��

/*  ID  */
#define CAN_CPU_INFO0_ID		0x01fffff0				//ID: CPU��Ϣ_ǰ������Ϣ
#define CAN_CPU_INFO1_ID		(CAN_CPU_INFO0_ID + 1)	//ID: CPU��Ϣ_��һ����Ϣ
#define CAN_VERSION_ID			0x01ffffe0				//ID: ��ȡ�汾��Ϣ
#define CAN_INIT_ID				0x01fffeff				//ID: CAN�ɹ���ʼ��
#define CAN_FILTER_BASE_ID		0x01fffee0				//ID: ��ȡFILTER��Ϣ������ID��
#define CAN_BAUDRATE_ID 		0x01fffed0				//ID: ��ȡ������
#define CAN_RESET_ID			0x01fffec0				//ID: ����CAN
#define CAN_ABOM_ID				0x01fffeb0				//ID: ����ABOM���Զ����߹���
#define CAN_ART_ID				0x01fffea0				//ID: ����ART���Զ��ش���
#define USART_BAUDRATE_ID		0x01fffe90				//ID: ����USART������			

/* FILTER��غ� */
#define CAN_FILTER_IDX_MASK	0x0000000f				//��FILTER��ID�л�ȡINDEX
#define CAN_FILTER_EN		0x80000000				//FILTER��ʹ�ܱ�ʶ
#define CAN_FILTER_DIS		0						//FILTER��ʧ�ܱ�ʶ
#define CAN_FORMAT_EXT		0x40000000				//FORMAT��EXTENDED
#define CAN_FORMAT_STD		0						//FORMAT��STANDARD

/* FLASH��غ� */
#define CAN_PARM_START		0xaaaaaaaa				//FLASH�б�ʶ�Ѿ���������
#define StartAddr 	 		((vu32)0x08008000)		//FLASH���׵�ַ
#define EndAddr    			((vu32)0x0800C000)		//FLASH��β��ַ
#define CAN_PARM_NUM		48						//FLASH�д��u32���ݸ���
#define CAN_ADDR_START		0						//START�洢�������е�һ��Ԫ��
#define CAN_ADDR_BAUD		1						//�����ʴ洢�������еڶ���Ԫ��
#define CAN_ADDR_FILTER		2  //-43  ��42��		//FILTER���������е�2����42��Ԫ��
#define CAN_ADDR_ABOM		44						//ABOM������Ϣλ��
#define CAN_ADDR_ART		45						//ARTλ��
#define CAN_ADDR_VER		46						//VERSIONλ��
#define USART_ADDR_BAUD		47



/*---------------Global Function--------------*/

//��ʼ�������³�ʼ��ʹ�õĺ���
bool CheckFLASH(void);
bool CANInitFLASH(void);
bool CANInitFromUSART(u32 *parm, u32 size, CAN_msg msg);
bool CANInitFromFLASH(void);
void USARTInitFromFLASH(u8 ctrl_remap);
void CAN_Operator(CAN_msg configMsg);

//��ȡ������FILTER
bool ReturnFilterInfo(u32 msgID);
bool SetFilterInfo(CAN_msg msg);

//��ȡ�����ò�����
bool ReturnBaudrate(void);
bool SetBaudrate(CAN_msg msg);

//��ȡ������ART
bool ReturnART(void);
bool SetART(CAN_msg msg);

//��ȡ������ABOM
bool ReturnABOM(void);
bool SetABOM(CAN_msg msg);

//��ȡCPU��Ϣ
bool ReturnCPU(void);

//��ȡ�汾��Ϣ
bool ReturnVersion(void);

//����CAN
bool RemoteResetCan(CAN_msg msg);

//����FLASH
bool CleanFlash(CAN_msg msg);

#endif

