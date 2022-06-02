/* ==================================================================================

 File name:     system_parameter.h
 Originator:    BLJ
 Description:   系统参数结构体头文件，系统参数，储存在EEPROM中

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 01-28-2015     Version 0.0.1           测试功能
-----------------------------------------------------------------------------------*/

#include "stm32f0xx.h"                      // STM32器件寄存器定义头文件

// 全局系统状态量，表示系统状态
struct SYSTEM_PARAMETER_Information {
	u16	DeviceType;							// 设备类型编号
	u16	SerialNumber1;						// 序列号1
	u16	SerialNumber2;						// 序列号2	
	u16	rsvd[13];
};

struct SYSTEM_PARAMETER_Display {
	u16	SpeedScaler;						// 车速比例，基值1000
	u16	ControllerSelect;					// 匹配控制器选择
	u16	rsvd[6];
};

// 系统状态结构体
typedef struct {
	struct SYSTEM_PARAMETER_Information	Information;
	struct SYSTEM_PARAMETER_Display		Display;
//	u16		Size;									// 系统状态量结构体，长度，单位2BYTE
} SYSTEM_PARAMETER_structDef;

// End of system_parameter.h

