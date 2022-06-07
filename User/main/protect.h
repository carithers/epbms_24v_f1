/* ==================================================================================

 File name:     protect.h
 Originator:    BLJ
 Description:   保护模块，整系统保护

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 01-10-2015     Version 0.0.1           测试功能
-----------------------------------------------------------------------------------*/

#ifndef __PEOTECTION_H__
#define __PEOTECTION_H__

#include "stm32f0xx.h"                     		// STM32器件寄存器定义头文件


// To be update: 故障码转为10进制，范围1-254
// Take care：故障码禁止为255，会与其它处理逻辑冲突

//#define	FAULT_EEPROM_FAILURE					104			// Lv0: EEPROM failure


// Inner
//#define	ERROR_ILLEGAL_MODEL_NUMBER				0x0011		// Lv0: Software and hardware mismatch
//#define	ERROR_MOTOR_PARAMETER_MISMATCH			0x0012		// Lv0: Motor parameter mismatch
//#define	ERROR_MOTOR_TYPE_MISMATCH				0x0013		// Lv0: Motor Type err
#define	FAULT_EEPROM_FAILURE					20		// Lv0: EEPROM failure
#define	FAULT_SOFTWARE_FAILURE					21		// Lv0: Software failure,例如主循环未被执行
#define	ERROR_ILLEGAL_PARAMETER					22		// Lv1: 参数设置有误
#define	FAULT_OSC								25		// Lv0: 外部晶振起振失败
#define ERROR_CODE_COPY                         27      // Lv0: 芯片ID校验失败，禁止盗版

// Opening
//#define	FAULT_BOOTING_UNDER_VOLTAGE				0x0021		// Lv1: Under voltage when booting
//#define	FAULT_BOOTING_OVER_VOLTAGE				0x0022		// Lv1: Over voltage when booting
#define	SYS_STAT_NOT_CLEAR						35		// Lv1: BQ769x0芯片系统报错，无法恢复
//#define	FAULT_MOTOR_OPEN						0x0024		// Lv1: Motor open
//#define FAULT_MAIN_CONTACTOR_WELDED				0x0025		// Lv1: Main contactor welded，to be delete
//#define	FAULT_MAIN_CONTACOTR_DID_NOT_CLOSE		0x0026		// Lv1: Main contactor did not close
//#define	FAULT_CURRENT_SENSOR_FAILURE			0x0027		// Lv0: Current sensor failure
//#define	FAULT_MAIN_CONTACTOR_OPEN				0x0028		// Lv1: Main contactor open
//#define	FAULT_MAIN_CONTACTOR_SHORT				0x0029		// Lv1: Main contactor short
//#define	FAULT_CONTROLLER_TEMP_SENSOR_FAILURE	0x002A		// Lv0: Controller temperature sensor failure
//#define	FAULT_PEDAL_CALIBRATION_FAILURE			0x002B		// Lv0: Pedal calibration failure
//#define	FAULT_THETA_INIT_FAILURE				0x002C		// Lv0: 电机初始角度获取失败，PWM信号解码失败

// Fault in running, Disable PWM
//#define	FAULT_DRIVE_FAILURE						0x0031		// Lv0: Drive module failure, Hardware short
//#define FAULT_ENCODER_FAILURE					0x0032		// Lv1: Encoder failure
//#define	FAULT_CONTROLLER_OVER_CURRENT			0x0033		// Lv1: Over current
//#define	FAULT_CONTROLLER_SEVERE_UNDER_TEMP		0x0034		// Lv1: Controller severe under temperature
//#define	FAULT_CONTROLLER_SEVERE_OVER_TEMP		0x0035		// Lv1: Controller severe over temperature
//#define	FAULT_DSP_OVER_TEMP						0x0036		// Lv1: DSP over temperature
//#define	FAULT_SEVERE_UNDER_VOLTAGE				0x0037		// Lv1: Controller severe under voltage
//#define	FAULT_SEVERE_OVER_VOLTAGE				0x0038		// Lv1: Controller severe over voltage
//#define	FAULT_PWM_OUTPUT_FAILURE				0x0039		// Lv0: PWM output failure
//#define	FAULT_ASSIST_CONTACTOR_FAILURE			0x003A		// Lv1: Assist contactors failure
//#define	FAULT_MOTOR_SEVERE_OVER_TEMP			0x003B		// Lv1: Motor severe over temperature
//#define	FAULT_SPI_COMMUNICATION_FAILURE			0x003C		// Lv1: SPI communication failure

// Fault in running, auto stop
//#define	FAULT_5V_OVER_CURRENT					0x0041		// Lv5:	5V output over current
//#define	FAULT_EMBRAKE_OPEN						0x0042		// Lv5: EM-Brake wire open
//#define	ERROR_DIRECTION_HANDLE					0x0043		// Lv5: Direction error
//#define	ERROR_SEQUENCING						0x0044		// Lv5: KSI, interlock, direction, throttle sequencing error
//#define	ERROR_STALL_DETECTED					0x0045		// Lv5: Stall detected
//#define	FAULT_CAN_COMMUNICATION_FAILURE			0x0046		// Lv5: CAN communication failure
//#define	ERROR_OVER_SPEED						0x0047		// Lv5: Over speed
//#define	FAULT_EMBRAKE_SHORT						0x0048		// Lv5: EM-Brake wire short

// Fault in analog input, auto stop
//#define	FAULT_THROTTLE1_WIPER_HIGH				0x0051		// Lv5: Throttle1 wire high
//#define	FAULT_THROTTLE1_WIPER_LOW				0x0052		// Lv5: Throttle1 wire low
//#define	FAULT_THROTTLE2_WIPER_HIGH				0x0053		// Lv5: Throttle2 wire high
//#define	FAULT_THROTTLE2_WIPER_LOW				0x0054		// Lv5: Throttle2 wire low
//#define	FAULT_THROTTLE3_WIPER_HIGH				0x0055		// Lv5: Throttle3 wire high
//#define	FAULT_THROTTLE3_WIPER_LOW				0x0056		// Lv5: Throttle3 wire low
//#define	FAULT_PEDAL_FAILURE						0x0057		// Lv5: Pedal failure
#define CHARGE_TEMPERATURE_FAULT               60
// Warning, limit output
//#define	WARNING_CONTROLLER_OVER_TEMP_CUTBACK	97			// Lv2: Over temperature
//#define	WARNING_UNDER_VOLTAGE_CUTBACK			98			// Lv2: Under voltage cutback
//#define	WARNING_OVER_VOLTAGE_CUTBACK			99			// Lv3: Over voltage cutback
//#define	WARNING_MOTOR_TEMP_HOT_CUTBACK			100			// Lv2: Motor over temperature
//#define	WARNING_MOTOR_TEMP_SENSOR_FAILURE		101			// Lv4: Motor temperature sensor failure
//#define	WARNING_ASSIST_THROTTLE_CHECK_FAILURE	102			// Lv4: Assist throttle check failure
//#define	WARNING_BATTERY_SEVERE_LOW				103			// Lv4: Battery severe low
#define	WARNING_BATTERY_OVER_TEMPERATURE		104			// Lv2: 电池温度过高
#define	WARNING_BATTERY_UNDER_TEMPERATURE		105			// Lv2: 电池温度过低
#define	WARNING_BMS_OVER_TEMPERATURE			106			// Lv2: BMS管理板温度过高
#define WARNING_BATTERY_NTC_FAILURE				107			// Lv2: 电池温度NTC断路
#define WARNING_BMS_NTC_FAILURE					108			// Lv2: BMS板温度NTC断路


#define FAULT_CELL_UNBALANCE                    122         // Lv2: 单体电压不平衡


// Only warning
//#define	WARNING_BATTERY_LOW						113			// Lv6: Battery low
//#define WARNING_PARAMETER_CHANGE				114			// Lv6: Paramter change without restart
//#define	WARNING_COIL1_OPEN_SHORT				115			// Lv6: Coil1 open or short
//#define	WARNING_COIL2_OPEN_SHORT				116			// Lv6: Coil2 open or short
//#define	WARNING_COIL3_OPEN_SHORT				117			// Lv6: Coil3 open or short
//#define	WARNING_COIL4_OPEN_SHORT				118			// Lv6: Coil4 open or short
//#define	WARNING_RS485_COMMUNICATION_FAILURE		119			// Lv6: RS485 communication failure

// Other fault
//#define	ERROR_OEM								0x0081		// Lv0: OEM information error

// 硬件故障
//#define	FAULT_ADSAMPLE							0x0091		// Lv0: AD采样模块出错



// ------------------------------------ BMS用故障信息 ------------------------------------

#define	FAULT_START_FAIL						161			// Lv0: BMS启动超时，可能由于故障信息无法清除
#define	FAULT_BQ769_DEVICE_XREADY				162			// Lv0: BQ769x0芯片内部DEVICE_XREADY标志位置位
#define	FAULT_BQ769_OVRD_ALERT					163			// Lv0: BQ769x0 ALERT信号被外部强制拉高
#define	FAULT_BQ769_I2C_FAIL					164			// Lv0: BQ769x0 I2C通信失败
#define	FAULT_BQ769_I2C_OVERTIME				165			// Lv0: BQ769x0 I2C通信超时
#define FAULT_BATVOL_DISMATCH                   166         // Lv1: BQ769x0 电池组总电压与单体电压总和不一致
#define FAULT_BQ769_OVERTEMPERATURE             167         // Lv1: BQ769x0 芯片内部温度过高
#define FAULT_BQ769_CMD_DISMATCH                168         // Lv1: BQ769x0 指令与反馈状态不一致，该输出不输出

#define FAULT_BQ769_UV							177			// Lv1: BQ769x0 单体欠压
#define FAULT_BQ769_OV							178			// Lv1: BQ769x0 单体过压
#define FAULT_BQ769_SCD							179			// Lv1: BQ769x0 放电回路短路
#define FAULT_BQ769_OCD							180			// Lv1: BQ769x0 放电回路过流
#define	FAULT_BAT_OT							181			// Lv1: 电池温度过高，大于65摄氏度
#define	FAULT_BAT_UT							182			// Lv1: 电池温度过低，小于-20摄氏度
#define	FAULT_BAT_CHARGE_OVER_CURRENT			183			// Lv1: 电池充电电流过大

// ----------------------------------- 代码调试用故障码 ------------------------------------
// 不要使用0x00FF故障码

#define FAULT_BQ769_FAIL						243			// Lv0: BQ769芯片出现未知错误，进入故障状态，但无故障信息
#define	BQ769_UPDATE_TOO_SLOW					244			// Lv0：BQ769芯片采样值更新间隔超过500ms
#define	BQ769_UPDATE_TOO_SLOW2					245			// Lv0：BQ769芯片采样值更新间隔超过500ms
#define	BQ769_CRC								246			// Lv0：BQ769芯片出现CRC校验错误，且未被处理
#define	FLASH_WRITE_FAIL						247			// Lv1: 写入失败
#define	FLASH_WRITE_FORBID						248			// Lv1: 待写入区域不为0xFFFF，禁止写入
#define	FLASH_WRITE_CHECK_FAIL					249			// Lv1: flash写入数据检查错误
#define	EEPROM_PARAMETER_LRC_FAIL				250			// Lv0: 参数LRC校验错误
#define	EEPROM_PARAMETER_WRONG					251			// Lv0: 参数LRC校验通过，但是参数内容错误，例如全为0
#define	EEPROM_COMMAND_FAIL						252			// Lv1: EEPROM指令操作失败
#define EEPROM_COMMAND_OVERTIME					253			// Lv1: EEPROM指令超时
#define	EEPROM_COMMAND_OVER_FLOW				254			// Lv1: EEPROM模块写入指令队列溢出




#endif


/* Define the structure of the PROTECT object. ------------------------------------*/
/* Protect Mode -------------------------------------------------------------------*/
struct PROTECT_PROTECTMODE_BITS {
	u16	OverCurrentCheck:1;
	u16	BatteryTemperatureCheck:1;
	u16	rsvd:14;
};

union PROTECT_PROTECTMODE_UNION {
	u16									all;
	struct	PROTECT_PROTECTMODE_BITS	bit;
};

/* Input --------------------------------------------------------------------------*/
struct PROTECT_INPUT_BITS {
	u16	EEPROMFaultFlag:1;						// EEPROM故障标志，外部置位，内部消除
	u16	rsvd:15;
};

union PROTECT_INPUT_UNION {
	u16							all;
	struct	PROTECT_INPUT_BITS	bit;
};

struct PROTECT_INPUT {
	union 	PROTECT_INPUT_UNION	Input_bits;
	u32		FastInterruptCnt;
	u32		MainInterruptCnt;
	u32		Interrupt1msCnt;
	u32		MainLoopCnt;
};

/* Parameter ----------------------------------------------------------------------*/
struct PROTECT_MODE_BITS {
	u16	MainContactorCheckEnable:1;
	u16	EMBrakeCheckEnable:1;
	u16	MotorTemperatureCheckEnable:1;
	u16	SlaveContactorCheckEnable:1;
	u16	SlaveCommunicationCheckEnable:1;
	u16	DriveProtectDisableCheck:1;
	u16	rsvd:10;
};

union PROTECT_MODE_UNION {
	u16							all;
	struct PROTECT_MODE_BITS	bit;
};

struct PROTECT_INVERTER {
	s32		DriveProtectDisableTemperature;
	s32		InverterSevereOverTemperature;
	s32		InverterOverTemperature;
	s32		DSPOverTemperature;
};

struct PROTECT_MOTOR {
	s32		MotorOverCurrent;
	s32		MotorSevereOverTemperature;
	s32		MotorOverTemperature;
	s32		SpeedOverLimit;
	u16		TemperatureSensorType;
};

struct PROTECT_PERIPHERAL {
	s32		MaxContactorOpenVoltage;				// 接触器两端电压最大差值，超过此值认为接触器断开
};

struct PROTECT_BATTERY {
	s32		SevereOverVoltage;
	s32		OverVoltage;
	s32		UnderVoltage;
	s32		SevereUnderVoltage;
};

struct PROTECT_PARAMETER {
	union 	PROTECT_MODE_UNION	Mode;
	struct 	PROTECT_INVERTER	Inverter;
	struct	PROTECT_MOTOR		Motor;
	struct	PROTECT_PERIPHERAL	Peripheral;
	struct	PROTECT_BATTERY		Battery;
};

/* Variable -----------------------------------------------------------------------*/
struct PROTECT_VARIABLE_CUTBACK {
	s32		VoltageDrivePercent;
	s32		VoltageBrakePercent;
	s32		CurrentPercent;
	s32		InverterTemperaturePercent;
	s32		MotorTemperaturePercent;
	s32		TemperaturePercent;
};


struct PROTECT_VARIABLE_CNT {
	u32	FastInterruptCntOld;
	u32	MainInterruptCntOld;
	u32	Interrupt1msCntOld;
	u32	MainLoopCountOld;					// 上一次主循环被执行次数
	u16	MainLoopSameCount;					// 主循环未被执行计数器
	u16	MainContactorImeasCnt;
	u16	EMBrakeImeasCnt;
	u16	DriveFaultCnt;
	u16	EncoderFaultCnt;
	u16	EEPROMBusyCnt;
	u16	MotorStalledCnt;
	u16	AccelDirectionFaultCnt;
	u16	MotorOutputFaultCnt;
	u16	OverCurrentCnt;
	u16	ChargeOverCurrentCnt;
	u16	OverTemperatureCnt;					// 过温检测延时
	u16	BMSOverTemperatureCnt;				// BMS过温检测延时
	u16	FaultLv2RecoverCnt;					// 故障Lv2恢复计数器，默认1000次后才恢复
	u16 SlowProtectUpdateCnt;               // 慢速保护更新次数
	u16 ContactorShortCnt;
    u16 UnderTemperatureCnt;
    u16 OverVoltageCount;
    u16 UnderVoltageCount;
    u16 temp1_cnt;
    u16 temp2_cnt;
    u16 temp1_flg;
    u16 temp_c_cnt;
};

struct PROTECT_VARIABLE {
	struct	PROTECT_VARIABLE_CUTBACK	Cutback;
	struct	PROTECT_VARIABLE_CNT		Count;
};

/* Output -------------------------------------------------------------------------*/
struct PROTECT_OUTPUT {
	s32	MaxDrirvePercent;
	s32	MaxBrakePercent;
	u16	DriveProtectEnable;
    u16 ContactorShortFailure;      // 接触器黏连
    u16 CellUnBalance;              //1：单体电压不均衡
};

/* Fault Code ---------------------------------------------------------------------*/
// Fault code flag
struct FAULTCODE_CODE {
	u16	SystemLockLevel0;					// Level0 fault: Output disable and system lock  //输出关闭，系统锁定
	u16	OutputDisableLevel1;				// Level1 fault: Output disable(PWM disable)  //电平故障:输出关闭(PWM关闭)
	u16	DriveLimitLevel2;					// Level2 fault: Drive limit/Max speed limit //驱动限制最大驱动速度限制
	u16	BrakeLimitLevel3;					// Level3 fault: brake limit
	u16	SpeedLimitLevel4;					// Level4 fault: Speed limit
	u16	AutoStopLevel5;						// Level5 fault: Full brake to stop
	u16	WarningLevel6;						// Level6 fault: Warning
};

/* Protect ------------------------------------------------------------------------*/
typedef struct {
	union 	PROTECT_PROTECTMODE_UNION	ProtectMode;
	struct	PROTECT_INPUT				Input;
	struct 	PROTECT_PARAMETER			Parameter;
	struct	PROTECT_VARIABLE			Variable;
	struct	PROTECT_OUTPUT				Output;
	struct	FAULTCODE_CODE				FaultCode;
}Protect_structDef;

/* Prototypes for the functions in protection.c -----------------------------------*/

//void Protect_StartUpCheck(PPROTECT pProtect);
void Protect_FastUpdate(Protect_structDef* v);
void Protect_SlowUpdate(Protect_structDef* v);
void Protect_TemperatureUpdate(Protect_structDef* v);
void Protect_ResetFault(Protect_structDef* v);

void Protect_SetFaultCodeLv0(Protect_structDef* v, u16 FaultCode);
void Protect_SetFaultCodeLv1(Protect_structDef* v, u16 FaultCode);
void Protect_SetFaultCodeLv2(Protect_structDef* v, u16 FaultCode);
//void Protect_SetFaultCodeLv3(PPROTECT pProtect, Uint16 FaultCode);
//void Protect_SetFaultCodeLv4(PPROTECT pProtect, Uint16 FaultCode);
//void Protect_SetFaultCodeLv5(PPROTECT pProtect, Uint16 FaultCode);

u16	Protect_GetFaultCode(Protect_structDef* v);				// 获取故障代码，若无故障，返回0
u16	Protect_GetWarningCode(Protect_structDef* v);			// 获取警告代码，若无警告，则返回0

u16	Protect_ClearFault(Protect_structDef* v);				// 清除故障函数

//#endif 												// __PEOTECTION_H__


