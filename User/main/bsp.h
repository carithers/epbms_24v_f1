/* ==================================================================================

 File name:     bsp.h
 Originator:    BLJ
 Description:   DSP硬件初始化模块

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 10-07-2014     Version 0.0.1           测试功能
-----------------------------------------------------------------------------------*/
#define APP_START_ADDRESS   (uint32_t)(0x08002000)   //偏移后的起始地址
// -------------------- 模块外部调用函数 -------------------------
void System_Init(void);                     // DSP硬件初始化函数
