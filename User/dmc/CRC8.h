/* =================================================================================

 File name:		CRC8.h             
 Originator:	BLJ
 Description: 	CRC8校验，x^8 + x^2 + x + 1,初始值为0，适用于TI BQ769x0系列AFE芯片I2C通信校验
 
=====================================================================================
 History:
-------------------------------------------------------------------------------------
 12-20-2015		Version 1.0.0		正式版本，测试通过
-----------------------------------------------------------------------------------*/


#include "stm32f0xx.h"


// 计算CRC校验值并返回
// Parameter：
// pMsg:待校验数据起始地址
// wDataLen：待校验数据长度，单位字节
u8 GetCRC8(u8* pMsg, u16 wDataLen);


// End of CRC8.h

