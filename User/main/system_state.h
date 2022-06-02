/* ==================================================================================

 File name:     system_state.h
 Originator:    BLJ
 Description:   系统状态数据结构体，记录系统状态及全局相关数据

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 01-03-2015     Version 0.0.1           测试功能
-----------------------------------------------------------------------------------*/

#include "stm32f0xx.h"                      // STM32器件寄存器定义头文件

// 全局系统状态量，表示系统状态
struct SYSTEM_STATE_State_bits {
	u16	EEPROMReadFinishFlag:1;				// EEPROM信息读取成功标志，暂时不用
	u16	IWDGReloadEnable:1;					// 0：禁止喂狗，1:正常喂狗
    u16 KEYState:1;                         // 外部KEY开关使能标志位，0：KEY断开，未使能，1：KEY闭合，使能
    u16 ChargerFaultFlag:1;                 // 充电机故障标志位
    u16 TestLED:1;                           
    u16 ChargeOnFlag:1;                     //充电开启标志
    u16 ChargeTempError:1;                  //充电温度报错
	u16 EnterBootloaderAsk:1;               // 进入bootloader请求，若此标志位置位，则发送进入Bootloader事件，并清除该标志位
	u16	rsvd:8;
};

union SYSTEM_STATE_union {
	u16								all;
	struct SYSTEM_STATE_State_bits	bit;
};

struct SYSTEM_STATE_Variable {
	u32	MCUIDNumber[3];						// 芯片ID number
	u16 EEPROMCheckValue[6];				// EEPROM中的校验信息
	u16 EEPROMCheckValue2[6];				// EEPROM中的校验信息
	u32	SystemMainLoopCountOld;				// 主循环计数，旧
	u16	g_systemMainLoopCountSameCount;		// 主循环未执行次数，即两次1ms时基中断之间，主循环未被执行
	u16	ParameterWriteAsk;					// 系统参数写入请求，当此位置位时，开始延时，超时后启动参数写入
	u16	ParameterWriteDelay;				// 系统参数写入之前延时1s，防止频繁写入
    u16 ChargeCurrentLimitDivision;         // 充电电流限制分频，例：2：即0.5C
};

struct SYSTEM_STATE_Output {
	u32	System1msInterruptCount;			// 1ms时基中断计数
	u32	SystemMainLoopCount;				// 主循环计数
	u32	SystemStartTime;					// 系统启动时间，以运行记录中TotalTime为准，当前时间减去启动时间即本次已运行时间
};

// 系统状态结构体
typedef struct {
	union 	SYSTEM_STATE_union		State;
	struct 	SYSTEM_STATE_Variable	Variable;
	struct 	SYSTEM_STATE_Output		Output;
} SYSTEM_STATE_structDef;

// End of system_state.h

