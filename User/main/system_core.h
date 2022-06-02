/* ==================================================================================

 File name:     system_core.h
 Originator:    BLJ
 Description:   系统核心头文件，全部程序连接头文件,用于全局数据声明，全局宏定义声明等

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 11-24-2014     Version 0.0.1           测试功能
-----------------------------------------------------------------------------------*/


// ------------------------- 包含其它模块头文件 ---------------------------
#include "target.h"
#include "systick.h"                            // 提供系统精确定时时基
#include "delay.h"                              // 软件延时模块
#include "bsp.h"                                // DSP系统初始化模块头文件
//#include "CAN.h"                              // CAN通信模块头文件
//#include "I2C_EEPROM.h"                       // EEPROM模块头文件
#include "I2C_BQ769xx.h"                		// I2C总线通讯模块头文件
#include "flash_EEPROM.h"               		// flash模拟EEPROM模块头文件
#include "USART_MODBUS.h"						// 串口MODBUS通讯模块头文件
//#include "LCD.h"                              // LCD显示模块头文件
#include "Input.h"								// 外部数字输入模块
#include "Output.h"								// 驱动输出模块
#include "ADC.h"								// ADC采样模块
#include "protect.h"							// Protect保护模块
#include "LED_flicker.h"
//#include "Contactor.h"					    // 主接触器模块
#include "qpn_port.h"                           // 状态机头文件
#include "communication.h"                      // 设备之间通信数据结构体头文件
#include "system_parameter.h"					// 系统参数头文件
#include "system_monitor.h"						// 系统采样头文件
#include "system_state.h"						// 系统运行状态头文件
#include "system_record.h"						// 系统运行记录头文件
#include "System_FaultRecord.h"					// 系统故障记录
#include "system_module.h"						// 系统级模块函数头文件

//状态机活动对象头文件
#include "AO_EEPROM.h"							// EEPROM活动对象
#include "AO_BMS.h"								// BMS管理状态机
#include "AO_SH36730x0.h"
// ---------------------------- 测试用宏定义 ------------------------------



// ---------------------------- 全局变量声明 -----------------------------
//extern CAN_structDef    			g_CAN;                  // CAN模块全局声明
//extern I2C_BQ769x0_structDef		g_I2C_BQ769x0;          // I2C BQ769x0模块
extern I2C_BQ769x0_structDef		g_I2C_SH36730x0;        // I2C BQ769x0模块

extern flash_EEPROM_structDef		g_flash_EEPROM;			// flash模拟EEPROM
extern USART_MODBUS_structDef		g_USART_MODBUS;			// 串口MODBUS通信模块
//extern LCD_structDef    			g_LCD;                  // LCD模块          
extern Protect_structDef			g_Protect;				// Protect模块
extern Input_structDef				g_Input;				// 数字输入模块
extern Output_structDef				g_Output;				// 数字驱动模块
extern ADC_structDef				g_ADC;					// ADC采样模块
	
extern Comm_structDef   			g_communication;        // 设备间通信数据结构体全局声明
extern SYSTEM_PARAMETER_structDef	g_SystemParameter;		// 系统参数
extern SYSTEM_MONITOR_structDef		g_SystemMonitor;		// 系统采样值
extern SYSTEM_STATE_structDef		g_SystemState;			// 系统状态
extern SYSTEM_RECORD_structDef		g_SystemRecord;			// 设备运行记录
extern FAULT_HISTORY_structDef		g_SystemFaultHistory;	// 系统故障记录
extern LED_Flicker_structDef		g_LED_Flicker;			// LED闪烁模块
//extern Contactor_structDef		g_Contactor;		    // 主接触器控制模块


// 活动对象全局声明
//extern Display					g_Display;				// LCD显示活动对象结构体
extern AO_EEPROM 					g_AO_EEPROM;			// EEPROM活动对象结构体
//extern AO_BQ769x0 				g_AO_BQ769x0;			// BQ769x0活动对象结构体
extern AO_BMS						g_AO_BMS;				// BMS管理状态机活动对象结构体

extern AO_SH36730x0 			    g_AO_SH36730x0;			// SH36730x0活动对象结构体


extern u16	AO_Record[10];

#define	PRESS_SIG_ALL		0xFF
#define	PRESS_SIG_ENTER		0
#define	PRESS_SIG_UP		1
#define	PRESS_SIG_DOWN		2
#define	PRESS_SIG_PLUS		3
#define	PRESS_SIG_MINUS		4
#define PRESS_SIG_ESCAPE	5

#define	PRESS_SIG_DIN0		10
#define	PRESS_SIG_DIN1		11

// ---------------------------- 状态机事件列表 ----------------------------
enum SystemSignals {                              
    FAULT_SIG = Q_USER_SIG,     				// 故障事件，系统活动对象接收,默认为8            
    EEPROM_FINISH_SIG,							// EEPROM指令完成事件，EEPROM活动对象接收
	EEPROM_FAIL_SIG,							// EEPROM指令出错事件，EEPROM活动对象接收
	EEPROM_LRC_FAIL_SIG,						// EEPROM读取校验失败事件，EEPROM活动对象接收
	EEPROM_WRITE_SIG,							// EEPROM写指令事件，EEPROM活动对象接收
	EEPROM_READ_SIG,							// EEPROM读指令事件，EEPROM活动对象接收
	
//-----------------------------------------------------------------------------------------------------------
	I2C_SH36730x0_FAIL_SIG,						// I2C SH36730x0 通信失败事件
	I2C_SH36730x0_FINISH_SIG,					// I2C SH36730x0 通信成功事件
//	I2C_SH36730x0_CRC_FAIL_SIG,					// I2C SH36730x0 CRC校验失败事件，目前仅读取时检查CRC是否正确
	START_SIG,									// 启动事件，具体内容由发生场合及接受方决定
	SH36730x0_CONTROL_SIG,						// SH36730x0 控制更新请求指令，带参数
//-----------------------------------------------------------------------------------------------------------
	DIN_SIG,									// 数字输入更新事件
	BMS_UPDATE_SIG,								// BMS更新事件，每次SH36730x0芯片数据读取完成后发生
	COMMUNICATION_UPDATE_SIG,					// CAN通信更新事件
	ENTER_BOOTLOADER_SIG,						// 进入Bootloader事件，AO_BMS状态机响应
};

// ---------------------------- 系统核心函数声明 ----------------------------
void System_1msInterrupt(void);                 // 1ms系统定时中断
void System_MainLoop(void);                     // 系统主循环函数



