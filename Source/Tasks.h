
#ifndef __TASKS_H__
#define __TASKS_H__
#include <stm32f10x_lib.h>
                          
#define KEY_EVENT BIT(0)
#define ROCKER_CHANGE_EVENT BIT(1)
#define ROCKER_CHANGE_THRESHOLD_EVENT BIT(2)
#define ROCKER_EVENT BIT(3)

#define __DEBUG__
//#define __ERR_PROCESS__
#define BIT(x) (1 << (x))


__task void InitTask (void);

extern u8 BeepType;



#endif
