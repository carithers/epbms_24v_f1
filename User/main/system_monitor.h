/* ==================================================================================

 File name:     system_monitor.h
 Originator:    BLJ
 Description:   系统采样值，变量结构体

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 01-15-2016     Version 0.0.1           测试功能
-----------------------------------------------------------------------------------*/

#include "stm32f0xx.h"                      // STM32器件寄存器定义头文件


// 电池采样信息 0x00-0x0F
struct SYSTEM_MONITOR_Battery {
	s16	Voltage;							// 电池组总电压，单位0.1V  // //g_SystemMonitor.BMS.Battery.Temperature
	s16	Current;							// 电池组总电流，单位0.1A
	s16	CurrentCali;						// 电池组总电流校准，单位0.001A，范围-0.099-0.099A
	s16	Capacity;							// 电池剩余电量，单位0.1AH
	s16	CapacityCali;						// 电池剩余电量校准，单位0.001AH，范围0-0.099AH
	s16	SOC;								// 电池剩余电量百分比，单位0.1%
	s16	CellMaxVoltage;						// 单体最高电压，单位0.001V
	s16	CellMaxVoltagePointer;				// 单体最高电压对应第几节电池，最底部一节为第一节
	s16	CellMinVoltage;						// 单体最低电压，单位0.001V
	s16	CellMinVoltagePointer;				// 单体最低电压对应第几节电池，最底部一节为第一节
	s16	Temperature[3];						// 电池温度1，单位0.1摄氏度
	u16	CellBalanceEnable;					// 单体均衡使能
	u16	CellBalancing;						// 单体使能中标志位
	u16	CellBalancePointer;					// 单体均衡对应第几节电池
};

// 单体信息 0x10-0x1F
struct SYSTEM_MONITOR_Cell {
	s16	CellVoltage[15];					// 单体电池电压，单位0.001V
	s16	rsvd;
};

// AFE芯片信息 0x20-0x27
struct SYSTEM_MONITOR_AFE {
	s16	InnerTemperature[3];				// AFE芯片内部温度采样，单位1摄氏度
	s16 BatteryTemperature[4];              // 电池内部温度，单位0.1摄氏度
	u16	rsvd;
};

// 管理信息 0x28-0x2F
struct SYSTEM_MONITOR_Manage_bits {
	u16	DischargeEnable:1;					// 放电开关使能标志位，实际中充放电开关同时打开关闭
	u16	ChargeEnable:1;						// 充电开关使能标志位
	u16	DischargeState:1;					// 放电开关实际状态，0：关闭，1：使能
	u16	ChargeState:1;						// 充电开关实际状态，0：关闭，1：使能
	u16	DischargeIOState:1;					// BQ769x芯片放电开关控制IO口电平，0：关闭，1：高电平，使能放电
	u16	ChargeIOState:1;					// BQ769x芯片充电开关控制IO口电平，0：关闭，1：高电平，使能充电
	u16	KEYState:1;							// KEY输入信号
	u16	ExtraBalance:1;						// 额外均流被使能
	u16	rsvd:8;	
};

union SYSTEM_MONITOR_Manage_union {
	u16									all;
	struct SYSTEM_MONITOR_Manage_bits	bit;
};

struct SYSTEM_MONITOR_Manage {
	union SYSTEM_MONITOR_Manage_union 	Manage_bits;
	u16	ExtraBalanceLastTime;				// 额外均流已持续时间，单位min
	u16	ChargeCurrentLimit;					// 充电允许电流，单位0.1A
	s16	BMSTemperature[2];					// BMS管理板温度，单位0.1摄氏度
    u16 DriveVoltage;                       // 内部驱动电压，单位0.1V
    u16 BatteryPackageVoltage;              // 电池包输出总电压，ADC采样得到，单位0.1V
	u16	rsvd;
};


// 故障信息 0x30-0x37
struct SYSTEM_MONITOR_Fault_bits {
	u16	ShortCurrent:1;						// 电池短路
	u16	OverCurrent:1;						// 电池过流
	u16	CellOverVoltage:1;					// 单体电压过高
	u16	CellUnderVoltage:1;					// 单体电压过低
	u16	OverTemperature:1;					// 电池温度过高
	u16	UnderTemperature:1;					// 电压温度过低
	u16	rsvd:10;
};

union SYSTEM_MONITOR_Fault_union {
	u16									all;
	struct SYSTEM_MONITOR_Fault_bits	bit;
};

struct SYSTEM_MONITOR_Fault {
	u16	FaultCodeLv0;
	u16	FaultCodeLv1;
	union SYSTEM_MONITOR_Fault_union	FaultInf_bits;
	u16	rsvd[5];
};

//运行时间 
struct SYSTEM_MONITOR_Timer {
    u16 RunTimeS;                           // 上电运行时间，单位s
    u16 DischargeTimeS;                     // 本次放电累计时间，单位s
    u16 rsvd[3];
    u16 I2CInterruptCnt;                    // I2C中断次数
    u16 MainInterruptCnt;                   // 1ms定时中断次数
    u16 MainLoopCnt;                        // 主循环次数
};

//管理板温度 
struct SYSTEM_MONITOR_BMS_Temperature {
    s16 BMSTemperature[4];                  // BMS管理板温度，单位0.1摄氏度
    u16 rsvd[4];
};

// 分别对应综合输出，第一路，第二路
struct SYSTEM_MONITOR_Current {    
	s16	Current;							// 电池组总电流，单位0.1A
	s16	CurrentCali;						// 电池组总电流校准，单位0.001A，范围-0.099-0.099A
	s16	Current1;							// 电池组总电流1，单位0.1A
	s16	Current1Cali;						// 电池组总电流校准1，单位0.001A，范围-0.099-0.099A
	s16	Current2;							// 电池组总电流2，单位0.1A
	s16	Current2Cali;						// 电池组总电流校准2，单位0.001A，范围-0.099-0.099A
    u16 rsvd[2];
};

// ---------------- BMS信息，0x00-0x67 --------------------
struct SYSTEM_MONITOR_BMS {
	struct SYSTEM_MONITOR_Battery	                Battery;        // 电池采样信息 0x00-0x0F 
	struct SYSTEM_MONITOR_Cell		                Cell;           // 单体信息 0x10-0x1F
	struct SYSTEM_MONITOR_AFE		                AFE;            // AFE芯片信息 0x20-0x27
	struct SYSTEM_MONITOR_Manage	                Manage;         // 管理信息 0x28-0x2F
	struct SYSTEM_MONITOR_Fault	                    Fault;          // 故障信息 0x30-0x37
    struct SYSTEM_MONITOR_Timer                     Timer;          // 运行时间 0x38-0x3F
    struct SYSTEM_MONITOR_Cell		                Cell2;          // 单体信息高位 0x40-0x4F
    struct SYSTEM_MONITOR_AFE		                Temperature2;   // 对应高位AFE，电池内部温度5-8     ,0x50-0x57
    struct SYSTEM_MONITOR_BMS_Temperature           BMSTemperature; // BMS板载温度传感器, 0x58-0x5F
    struct SYSTEM_MONITOR_Current                   Current;        // 综合输出，0x60-0x67
    u16    rsvd[16];

};

// 系统预留采样信息，用于测试，0x38-0x47
// 系统预留采样信息2，用于测试，0x48-0x57
struct SYSTEM_MONITOR_Reverse {
	s16	Data[16];
};

// --------------- 系统采样值，测试用 ----------------------
struct SYSTEM_MONITOR_System {
	struct SYSTEM_MONITOR_Reverse	Reverse;
	struct SYSTEM_MONITOR_Reverse	Reverse2;
	struct SYSTEM_MONITOR_Reverse	Reverse3;
};


// ------------------------------ 系统采样结构体 -------------------------------
typedef struct {
	struct SYSTEM_MONITOR_BMS		BMS;
	struct SYSTEM_MONITOR_System	System;
} SYSTEM_MONITOR_structDef;

// End of system_monitor.h

