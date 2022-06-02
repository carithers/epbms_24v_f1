/* ==================================================================================

 File name:     system_record.h
 Originator:    BLJ
 Description:   运行记录数据结构体，默认长度为14byte，2byte LRC，共16 byte， 16级循环，共
				256 Byte

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 12-31-2014     Version 1.0.0           初始版本
-----------------------------------------------------------------------------------*/


#include "stm32f0xx.h"                     		// STM32器件寄存器定义头文件


// --------------------------- 运行记录结构体 -------------------------------------
// 长度固定为30字节!!!,不包括2字节校验码
typedef struct SYSTEM_RECORD {
	s32	TotalTime;					// 总运行时间，单位s
	s32	BatteryCapacity;			// 电池剩余电量，单位mAH
	s32	BatteryFullCapacity;		// 电池可存储电量，若此值小于设计电量，则SOC根据此值进行计算，单位mAH
	u16 SOC;						// 电池剩余电量百分比，单位0.1%
	u16	CircleNumber;				// 总循环次数，单位0.1次
	u16	LifeCalibrateCircleNumber;	// 电池寿命循环补偿次数，单位0.1次
	u16	MaxBatteryTemperature;		// 运行过程中电池最高温度，单位0.1摄氏度
    s32 DischargeTime;              // 总放电时间，单位s
    u16	rsvd[2];
    u16 BMSMaxTemp;                 // BMS最高温度
}SYSTEM_RECORD_structDef;


// End of system_record.h

