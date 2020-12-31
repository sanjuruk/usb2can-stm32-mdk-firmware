下载前的特殊设置

需将D:\（或其他）Keil\ARM\INC\ST\STM32F10x
文件夹下的stm32f10x_it.h文件打开
将
void NMI_Handler(void)        __attribute__ ((alias("NMIException")));
void HardFault_Handler(void)  __attribute__ ((alias("HardFaultException")));
void MemManage_Handler(void)  __attribute__ ((alias("MemManageException")));
void BusFault_Handler(void)   __attribute__ ((alias("BusFaultException")));
void UsageFault_Handler(void) __attribute__ ((alias("UsageFaultException")));
void DebugMon_Handler(void)   __attribute__ ((alias("DebugMonitor")));
void SVC_Handler(void)        __attribute__ ((alias("SVCHandler")));
void PendSV_Handler(void)     __attribute__ ((alias("PendSVC")));
void SysTick_Handler(void)    __attribute__ ((alias("SysTickHandler")));
这一段语句注释掉，才能正常下载
