#ifndef __FLASH_H
#define __FLASH_H

#include "stm32f10x_lib.h"

u32 FLASH_write(vu32 startAddr, u32 *buf, u32 buf_size);
bool FLASH_read(u32 *buf, vu32 startAddr, u32 buf_size);

#endif
