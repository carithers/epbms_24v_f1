/* ==================================================================================

 File name:     ADC.c
 Originator:    BLJ
 Description:   m0内核芯片ADC模块，采样外部模拟输入，实现基本功能，部分配置仍需检查完善。
				启动时序，初始化校正等需再检查。
 Take care:		m0内核芯片所带ADC模块功能较m3简化很多，ADC采样无注入组，规则组区分，当触发
				ADC采样时，依次从编号0,1,2......开始采样，只采样已使能的ADC通道。采样通道
				不可任意排布，注意影响
 
=====================================================================================
 History:
-------------------------------------------------------------------------------------
 07-12-2016		Version 0.9.0			基本功能完成，待进一步完善
 12-16-2014     Version 0.0.1           测试功能
-----------------------------------------------------------------------------------*/


#include "stm32f0xx.h"

struct ADC_Parameter {
    s32     CurrentSensorMaxCurrent;                            // 电流传感器放大比例，单位1A
    s32     CurrentSensorMaxCurrent2;                           // 电流传感器放大比例，单位1A，第二路
    s32     CurrentSensorMaxVoltage;			                // 电流传感器放大比例，单位1mV    
    s32     CurrentSensorMaxVoltage2;			                // 电流传感器放大比例，单位1mV，第二路
    s32     VdcSensorGain;                                      // 直流母线电压采样比例，实际值为上拉电阻+下拉电阻阻值，单位0.1欧姆
    s32     VdcSensorOffset;                                    // 直流母线电压采样误差，单位0.1V
    s32     InnerDriveSensorGain;                               // 内部驱动电压采样比例，实际值为上拉电阻+下拉电阻阻值，单位0.1欧姆
    s32     InnerDriveSensorOffset;                             // 内部驱动电压采样误差，单位0.1V    
    u16     BatteryCurrentMode;                                 // 采样电流功能模式，0：单分流器，1:单磁霍尔，2：双磁霍尔
};

// ----------------------------- ADC模块结构体 -------------------------------------
struct ADC_Output {
	s32		NTCRes1;
	s32		NTCRes2;
	s32		NTCRes3;
    s32   BatteryNTCRes[7];                                   // 电池包NTC温度，单位1欧姆
    s32   BalanceBoardNTCRes[2];                              // 均流板NTC温度，单位1欧姆  
    s32   DriveVoltage;                                       // 12V内部驱动电源电压，单位0.1V
	s32		LMT87Voltage;										// LMT87输出电压
	u16		ADCResult[3];										// ADC采样结果，分别对应通道1-3
	u16     KEY_adc;                                             //钥匙ADC检测值
};	

typedef struct {
    struct  ADC_Parameter   Parameter;                          // 参数
	struct	ADC_Output		Output;								// 输出变量
    u16		ADCBuffer[16];										// ADC采样结果，16级缓存，分别对应1-16次采样结果
} ADC_structDef;
																

// ----------------------------- 外部可调用函数 --------------------------------
void ADC_DeviceInit(ADC_structDef* v);							// ADC采样模块初始化
void ADC_SlowUpdate(ADC_structDef* v);							// ADC采样慢速更新


// End of ADC.h

