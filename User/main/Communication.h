/* ==================================================================================

 File name:     communication.h
 Originator:    BLJ
 Description:   设备之间通信数据内容结构体头文件.

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 11-28-2014     Version 0.0.1          
-----------------------------------------------------------------------------------*/

#include "stm32f0xx.h"

// ------------------------------ 模块结构体---------------------------------
struct Comm_NMT {
	u8	ControlCommond;
	u8	rsvd[7];
};

/*
// 牵引控制器通信数据内容
struct Comm_Traction1_Input_bit {
    u8 Input1:1;
    u8 Input2:1;
    u8 Input3:1;
    u8 Input4:1;
    u8 Input5:1;
    u8 Input6:1;
    u8 Input7:1;
    u8 Input8:1;
};

union Comm_Traction1_Input_union {
    u8                                 all;
    struct Comm_Traction1_Input_bit     bit;
};

struct Comm_Traction1 {
    union Comm_Traction1_Input_union    Input;      // 数字输入
    u8 FaultCode;              // 故障反馈
    u8 SpeedHzHi;              // 速度反馈高位
    u8 SpeedHzLow;             // 速度反馈低位
};
*/

struct Comm_Traction1_Input_bit {
    u8 Input0:1;
    u8 Input1:1;
    u8 Input2:1;
    u8 Input3:1;
    u8 Input4:1;
	u8 Input5:1;
    u8 Input6:1;
    u8 Input7:1;
};

union Comm_Traction1_Input_union {
    u8                                 all;
    struct Comm_Traction1_Input_bit     bit;
};

struct Comm_Traction1 {
	u8	rsvd;
	u8	BDI;
	u8 	SpeedHzHi;              // 閫熷害鍙嶉楂?浣嶏紝鍗曚綅涓洪鐜?
    u8 	SpeedHzLow;             // 閫熷害鍙嶉浣?浣嶏紝鍗曚綅涓洪鐜?
	s8	Steer;
    union Comm_Traction1_Input_union    Input;      // 鏁板瓧杈撳叆鍙?
	u8	rsvd2;
	u8	rsvd3;
};

struct Comm_Traction2_Input_bit {
    u8 Seat:1;
    u8 Forward:1;
    u8 Input3:1;
    u8 Input4:1;
    u8 Input5:1;
	u8 Input6:1;
    u8 Input7:1;
    u8 Input8:1;
};

union Comm_Traction2_Input_union {
    u8                                 all;
    struct Comm_Traction2_Input_bit     bit;
};

struct Comm_Traction2 {
    u8 rsvd1;
    u8 rsvd2;
    u8 rsvd3;
    u8 rsvd4;
    u8 rsvd5;	
	union Comm_Traction2_Input_union    Input;      // 鏁板瓧杈撳叆鍙?
    u8 rsvd[2];
};

struct Comm_Fault {
    u8 rsvd1;
    u8 rsvd2;
    u8 FaultCode;
    u8 rsvd[5];
};



struct Comm_SmartDisplay_Input_bits {
    u8 bit0:1;				// 手刹
    u8 bit1:1;
    u8 bit2:1;				// 托盘举升
    u8 bit3:1;
	u8 bit4:1;				// 托盘后移
    u8 bit5:1;				// 托盘平移	
    u8 bit6:1;				// 泵机速度	
    u8 bit7:1;	
};

union Comm_SmartDisplay_Input_union {
    u8                                 		all;
    struct Comm_SmartDisplay_Input_bits     bit;
};

struct Comm_SmartDisplay {
	union Comm_SmartDisplay_Input_union	Input;
    u8 rsvd[7];
};

struct Comm_SmartSpeedLimit {
	u8	rsvd1;
	u8	rsvd2;
	u8	SpeedLimit;
	u8	rsvd[5];
};

struct Comm_TractionTimeAsk {
	u8	TimeAsk;
	u8 rsvdp[7];
};

struct Comm_TractionTime {
	u8	rsvd1;
	u8	rsvd2;
	u8	TimeHi;
	u8	TimeLow;
	u8	rsvd[4];
};

struct Comm_MDI_CAN_DIN1 {
	u8	BYTE0;
	u8	BYTE1;
	u8	rsvd[6];
};

struct Comm_BMS_Data_Test {
	u8	CapacityByte3;
	u8	CapacityByte2;
	u8	CapacityByte1;
	u8	CapacityByte;
	u8	VoltageHi;
	u8	VoltageLow;
	u8	CurrentHi;
	u8	CurrentLow;
};

struct Comm_BMS_Control_Test {
	u8	DSGControl;
	u8	CHGControl;
	u8	rsvd[6];
};

struct Comm_Test {
	u8	Byte[8];
};


struct Comm_BMS_Data_BatteryState_bits {
    u8 Charge:1;						// 0：未充电＿：充电中
    u8 UnderTemperature:1;				// 电池温度过低
    u8 OverTemperature:1;				// 电池温度过高
    u8 ShortCurrent:1;					// 电池短路
	u8 OverCurrent:1;					// 电池过流
    u8 UnderVoltage:1;					// 单体欠压	
    u8 OverVoltage:1;					// 单体过压
	u8 BMSFail:1;						// BMS故障
};


union Comm_BMS_Data_BatteryState_union {
    u8                                 			all;
    struct Comm_BMS_Data_BatteryState_bits     	bit;
};


struct Comm_BMS_Data1 {
	u8	BatteryVoltageHi;				// 电池总电压，单位0.1V，高8使
	u8	BatteryVoltageLow;
	u8	BatteryCurrentHi;				// 电池总电流，充电为正电流，单使.001A，高8使
	u8	BatteryCurrentLow;
	u8	CapacityHi;						// 电池剩余电量，单使.001AH，高8使
	u8	CapacityLow;
	u8	SOC;							// 电池剩余电量百分比，1%
	union Comm_BMS_Data_BatteryState_union	BatteryState;
};


struct Comm_BMS_Data2 {
	u8	SingleMaxVoltageHi;				// 单体最高电压，单位0.001V，高8使
	u8	SingleMaxVoltageLow;
	u8	SingleMaxPointer;				// 最高电压单体对应第几节
	u8	SingleMinVoltageHi;				// 单体最低电压，单位0.001V，高8使
	u8	SingleMinVoltageLow;			
	u8	SingleMinPointer;				// 最低电压单体对应第几节
	u8	BatteryTemperature;				// 电池温度，有符号，单使摄氏庿
	u8	BatteryTemperature2;			// 电池温度，有符号，单使摄氏庿
};

struct Comm_BMS_Data3 {
	u8	BDILowLevel;					// 单体最高电压，单位0.001V，高8使
	u8	rsvd[7];
};

struct Comm_Curtis1212C_Ask {
	u8	rsvd[8];
};

struct Comm_BMS_LowSOC_bits {
    u8 LiftLimit:1;						//
	u8 SpeedLimit:1;
	u8 rsvd:6;
};

union Comm_BMS_LowSOC_union {
    u8                         		all;
    struct Comm_BMS_LowSOC_bits    	bit;
};

struct Comm_Curtis1212C_Response {
	union Comm_BMS_LowSOC_union		LowSOC;
	u8	rsvd[7];
};


struct Comm_Charger {
	u8	VoltageHi;
	u8	VoltageLow;
	u8	CurrentHi;
	u8	CurrentLow;
    u8  ChargerState;
	u8	rsvd[2];
	u8  ChargeFaultCode;	
};

struct Comm_BMS_ChargeCommand {
	u8	ChargeVoltageLimitHi;
	u8	ChargeVoltageLimitLow;
	u8	ChargeCurrentLimitHi;
	u8	ChargeCurrentLimitLow;
	u8	ChargeAllow;					// 充电允许标志位，1：禁止充电，0：允许充甿
// 电池状态正常情况下值为0x00，如果禁止充电，则设置如下：0x01 已充滿 0x02 电池温度过高 0x03 电池温度过低 0x04 单体电压过高 0x05 
// 单体电压过低 0x06 电池总压过高 0x07 电池总压过低 0x08 电池单体电压不均衿0x09 充电过流  ...(其余是否有限制充电的原因，罗刿
    u8  BMSState; 
    u8  SOCFullFlag;                    // 电池充满标志使
	u8	rsvd;
};

struct Comm_FaultCode {
	u8	FaultCodeLv1;					// 1级故障，直接切断输出
	u8	WarningCodeLv2;					// 2级警抿仅报警不动作
	u8	FaultCodeLv3;
	u8	FaultCodeLv4;
	u8	rsvd[3];
    u8  BMSTemperature;
};

struct Comm_BMSFault_Data {
	u8      BYTE0;
	u8		BYTE1;
	u8		BYTE2;
	u8		BYTE3;
	u8		BYTE4;
	u8		BYTE5;
	u8		BYTE6;
	u8		BYTE7;
};


// 设备间通信数据结构使
typedef struct {
	struct Comm_NMT						NMT;						// 0x00
    struct Comm_Traction1   			TractionInf;				// 0x388
	struct Comm_Traction2				TractionInf2;				// 0x18C
    struct Comm_Fault   				FaultInf1;					// 0x288		// Node 3, Traction
	struct Comm_Fault   				FaultInf2;					// 0x28C		// Node 5, Pump
	struct Comm_Fault   				FaultInf3;					// 0x290		// Node 4, Traction2
	struct Comm_Fault					FaultInf4;					// 0x294		// Node 7, Pump2
	struct Comm_Fault					FaultInf5;					// 0x298		// Node 6, Eps
	struct Comm_SmartDisplay			SmartDispaly;				// 0x3C0
	struct Comm_SmartSpeedLimit			SmartSpeedLimit;			// 0x60C
	struct Comm_TractionTimeAsk			TractionTimeAsk;			// 0x608
	struct Comm_TractionTime			TractionTime;				// 0x588
	struct Comm_MDI_CAN_DIN1			MDI_CAN_DIN1;				// 0x240
	struct Comm_BMS_Data_Test			BMSDataTest;				// 0x600
	struct Comm_BMS_Control_Test		BMSControlTest;				// 0x681
	struct Comm_Test					Test1;						// 0x682
	struct Comm_FaultCode				FaultCode;					// 0x683
	struct Comm_BMS_Data1				BMSData1;					// 0x2F0
	struct Comm_BMS_Data2				BMSData2;					// 0x2F1
	struct Comm_BMS_Data3				BMSData3;					// 0x2F2
	struct Comm_Curtis1212C_Ask			Curtis1212CAskBMS;			// 0x27C
	struct Comm_Curtis1212C_Response	Curtis1212CResponseBMS;		// 0x1FC
	struct Comm_Charger					ChargerState;				// 0x0F5
	struct Comm_BMS_ChargeCommand 		BMSChargeCommand;			// 0x0F4
	
	struct Comm_BMSFault_Data           BootCMD;                    // 0x797上位机重启握手接收
	struct Comm_BMSFault_Data           BootRev;                    // 0x7AB设备回复上位机
	
	u8	ReceiveFrame[8];
	u8	SendFrame[8];
} Comm_structDef;











