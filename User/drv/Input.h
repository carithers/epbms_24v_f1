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
#ifndef __INPUT_H
#define __INPUT_H

#include "stm32f0xx.h"

// ----------------------------  模块结构体定义 ---------------------------------
struct Input_DIN_bits {
	u16	ALERT:1;
	u16	KEY:1;
	u16	CHG:1;
	u16	DSG:1;
	u16	DIN4:1;
	u16	DIN5:1;
	u16	DIN6:1;
	u16	rsvd:9;
};

union Input_DIN_union {
	u16						all;
	struct Input_DIN_bits 	bit;
};
																
// 内部变量
struct Input_Variable {
	union Input_DIN_union	DINInput_bits;						// 输入
	union Input_DIN_union	DINFilter_bits;						// 滤波
	union Input_DIN_union	DINOld_bits;						// 上一次存储数据
	u8	PressFilterCount;										// Press变量滤波次数
	u8	DINFilterCount;											// DIN变量滤波次数
};

// 输出变量
struct Input_Output {
	union Input_DIN_union	DIN_bits;							// 已处理完毕数据，可外部读取
};

// Output模块结构体
typedef struct {
	struct 	Input_Variable		Variable;
    struct 	Input_Output		Output;
} Input_structDef;


// ----------------------------- 外部可调用函数 ---------------------------------
void Input_DeviceInit(void);									// 数字输入模块初始化
void Input_Update(Input_structDef* v);							// 数字输入口更新，运行在1ms定时时基中
void EXTI_ROUSE_Init(void);


#endif
// End of Input.h	
										

