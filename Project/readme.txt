����ǰ����������

�轫D:\����������Keil\ARM\INC\ST\STM32F10x
�ļ����µ�stm32f10x_it.h�ļ���
��
void NMI_Handler(void)        __attribute__ ((alias("NMIException")));
void HardFault_Handler(void)  __attribute__ ((alias("HardFaultException")));
void MemManage_Handler(void)  __attribute__ ((alias("MemManageException")));
void BusFault_Handler(void)   __attribute__ ((alias("BusFaultException")));
void UsageFault_Handler(void) __attribute__ ((alias("UsageFaultException")));
void DebugMon_Handler(void)   __attribute__ ((alias("DebugMonitor")));
void SVC_Handler(void)        __attribute__ ((alias("SVCHandler")));
void PendSV_Handler(void)     __attribute__ ((alias("PendSVC")));
void SysTick_Handler(void)    __attribute__ ((alias("SysTickHandler")));
��һ�����ע�͵���������������
