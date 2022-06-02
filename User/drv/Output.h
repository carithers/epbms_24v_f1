/* ==================================================================================

 File name:     Output.h
 Originator:    BLJ
 Description:   驱动输出模块

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 04-11-2015		Version 1.0.0			正式版本
 01-15-2015     Version 0.0.1           测试功能
-----------------------------------------------------------------------------------*/


#include "stm32f0xx.h"
#include "target.h"


// ----------------------------  模块结构体定义 ---------------------------------
struct Output_State_bits {
	u16	Output1:1;
	u16	Output2:1;
	u16	Output3:1;
	u16	Output4:1;
	u16	TestLED:1;					// 测试用LED
	u16	PWMEnable:1;				// PWM输出使能
	u16	ContactorEnable:1;			// 继电器使能
	u16	PreDischargeEnable:1;		// 电池预预放电使能
	u16	LED_RED:1;
	u16	LED_GREEN:1;
	u16	LED_BLUE:1;
    u16	LED_YELLOW:1;
    u16 FanEnable:1;                // 用于指示蜂鸣器工作状态
	u16	rsvd:4;
};

union Output_State_union {
	u16							all;
	struct Output_State_bits	bit;
};

// Output模块结构体
typedef struct {
    union Output_State_union	State;
} Output_structDef;


// ----------------------------- 外部可调用函数 ---------------------------------
void Output_DeviceInit(void);						// output模块初始化
void PWR_EN_Init(void);
#if (CONTROLLER_TARGET == BMS_XINXIN_A1 || CONTROLLER_TARGET == BMS_XINXIN_A2)				// 新昕滑板BMS待预放电功能

void Output_PreDischarge_Update(Output_structDef* v, u8 NewState);				// 电池预放电控制更新

#endif				// CONTROLLER_TARGET == BMS_XINXIN_A1 || CONTROLLER_TARGET == BMS_XINXIN_A2	


// 带3个LED指示灯		
#if (CONTROLLER_TARGET == BMS_EP_A5 || CONTROLLER_TARGET == BMS_EP_20_REV1_1_2TEMP || CONTROLLER_TARGET == BMS_EP_ALPHA || CONTROLLER_TARGET == BMS_EP_200_A2 || CONTROLLER_TARGET == BMS_EP_200_B1 || CONTROLLER_TARGET == BMS_EP_200_B2_1)					

void Output_LED_RED_Update(Output_structDef* v, u16 State);
void Output_LED_GREEN_Update(Output_structDef* v, u16 State);
void Output_LED_BLUE_Update(Output_structDef* v, u16 State);
void Output_LED_YELLOW_Update(Output_structDef* v, u16 State);
void Output_BUZZER_Update(Output_structDef* v, u16 State);
void Output_LOCK_Update(Output_structDef* v, u16 State);//进入低功耗用
#endif


#if (CONTROLLER_TARGET == BMS_EP_200_B2_1)				// 此硬件带测试灯

void Output_TestLED_Update(Output_structDef* v, u8 NewState);
	
#endif


// End of Output.h

