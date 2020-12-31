#include "FLASH.h"
#include "string.h"

#define FLASH_PAGE_SIZE    ((u16)0x400)
typedef enum {FAILED = 0, PASSED = !FAILED} FLASH_Test_Status;

u32 FLASH_write(vu32 startAddr, u32 *buf, u32 buf_size)
{
	volatile FLASH_Status FLASHStatus;
	u32 *bufStart;
	u32 EraseCounter = 0x00, Address = 0x00;
	vu32 NbrOfPage = 0x00;
	
	bufStart = buf;
	FLASHStatus = FLASH_COMPLETE;

	FLASH_Unlock();

	NbrOfPage = (buf_size + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	
	
	for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
	{
		FLASHStatus = FLASH_ErasePage(startAddr + (FLASH_PAGE_SIZE * EraseCounter));
	}
	
	Address = startAddr;
	while((Address < (startAddr + buf_size)) && (FLASHStatus == FLASH_COMPLETE))
	{
		FLASHStatus = FLASH_ProgramWord(Address, *buf++);
		Address += 4;
	}
	
	//检查写入数据是否正确
	Address = startAddr;
	while((Address < (startAddr + buf_size)))
	{
		if((*(vu32*) Address) != *bufStart++)
		{
			return FALSE;
		}
		Address += 4;
	}
	
	return TRUE;
}

bool FLASH_read(u32 *buf, vu32 startAddr, u32 buf_size)
{
	memcpy(buf, (u32 *)startAddr, buf_size);
	return TRUE;
}
