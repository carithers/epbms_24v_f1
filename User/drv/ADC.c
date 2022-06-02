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

#include "target.h"
#include "ADC.h"
#include "delay.h"
#include <math.h>

u8 l_ADCCalibrationWait = 10;				// 最多允许等待10次，ADC模块自校验

#if (CONTROLLER_TARGET == BMS_EP_A5 || CONTROLLER_TARGET == BMS_EP_20_REV1_1_2TEMP || CONTROLLER_TARGET == BMS_EP_ALPHA)
	
	
/* ADC1 DR register base address */
// Take care： 不同芯片地址可能不同
//#define DR_ADDRESS                  ((uint32_t)0x4001244C)
#define DR_ADDRESS                  ((uint32_t)0x40012440)


// --------------------------- ADC模块相关IO口配置函数 -------------------------------
void ADC_GPIO_Config(void)
{ 
	GPIO_InitTypeDef GPIO_InitStructure;          				// GPIO配置寄存器结构体	

#if (ADC_CHANNEL_NUMBER == 1)
	
	GPIO_InitStructure.GPIO_Pin = ADC_AIN1_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(ADC_AIN1_PORT, &GPIO_InitStructure);
	
#elif (ADC_CHANNEL_NUMBER == 2)

	GPIO_InitStructure.GPIO_Pin = ADC_AIN1_PIN;//电池内部
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(ADC_AIN1_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = ADC_AIN2_PIN;//板载
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(ADC_AIN2_PORT, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Pin = ADC_AIN3_PIN; //KEY检测
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(ADC_AIN3_PORT, &GPIO_InitStructure);
	
#elif (ADC_CHANNEL_NUMBER == 3)

	GPIO_InitStructure.GPIO_Pin = ADC_AIN1_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(ADC_AIN1_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = ADC_AIN2_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(ADC_AIN2_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = ADC_AIN3_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(ADC_AIN3_PORT, &GPIO_InitStructure);
    
#elif (ADC_CHANNEL_NUMBER == 5)

	GPIO_InitStructure.GPIO_Pin = ADC_AIN1_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(ADC_AIN1_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = ADC_AIN2_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(ADC_AIN2_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = ADC_AIN3_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(ADC_AIN3_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = ADC_AIN4_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(ADC_AIN4_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = ADC_AIN5_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(ADC_AIN5_PORT, &GPIO_InitStructure);

#endif

}

// ---------------------------- ADC采样模块硬件初始化 ---------------------------------
void ADC_DeviceInit(ADC_structDef* v)
{	

	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef	DMA_InitStructure;       					//DMA初始化结构体声明
	
	ADC_GPIO_Config();											// ADC模块IO口配置
	
	// RCC_CFGR寄存器的ADCPRE位，PCLK2为72MHz，ADCCLK为12MHz.ADCCLK最大不可超过14MHz
	// To be update: m0再说
//	RCC_ADCCLKConfig(RCC_PCLK2_Div4);	//
    RCC_ADCCLKConfig(RCC_ADCCLK_PCLK_Div4);
	// 开启ADC1模块时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	
	
	// 开启DMA1模块时钟
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);
	
// ---------------------------- ADC模块及规则组配置 -----------------------------

	// 不配置模拟看门狗
	ADC_AnalogWatchdogCmd(ADC1, DISABLE);

	ADC_DeInit(ADC1);											// Reset ADC1

//ADC_Cmd(ADC1, ENABLE);										// ADC_CR2寄存器的ADON位，第一次置位将ADC模块从掉电状态唤醒，第二次置位才是启动

	// 配置ADC1
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;							// 采样精度为12位
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; 							// 不连续采样
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;  	// 无触发，即仅允许软件触发  
	ADC_InitStructure.ADC_ExternalTrigConv =  ADC_ExternalTrigConv_T1_CC4;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Upward;					// 扫描方向，Upward：即从ADC0开始扫描
	ADC_Init(ADC1, &ADC_InitStructure); 


#if (ADC_CHANNEL_NUMBER == 1)

	// 采样1通道
	// 配置规则组采样通道,使能通道并配置采样保持时间
	ADC_ChannelConfig(ADC1, ADC_AIN1_CHANNEL , ADC_SampleTime_71_5Cycles);

#elif (ADC_CHANNEL_NUMBER == 2)

	// 采样2通道
	// 配置规则组采样通道,使能通道并配置采样保持时间

	ADC_ChannelConfig(ADC1, ADC_AIN1_CHANNEL , ADC_SampleTime_71_5Cycles);
	ADC_ChannelConfig(ADC1, ADC_AIN2_CHANNEL , ADC_SampleTime_71_5Cycles);
    ADC_ChannelConfig(ADC1, ADC_AIN3_CHANNEL , ADC_SampleTime_71_5Cycles);//KEY信号

#elif (ADC_CHANNEL_NUMBER == 5)
    
        // 采样5通道
        // 配置规则组采样通道,使能通道并配置采样保持时间
        ADC_ChannelConfig(ADC1, ADC_AIN1_CHANNEL , ADC_SampleTime_71_5Cycles);
        ADC_ChannelConfig(ADC1, ADC_AIN2_CHANNEL , ADC_SampleTime_71_5Cycles);
        ADC_ChannelConfig(ADC1, ADC_AIN3_CHANNEL , ADC_SampleTime_71_5Cycles);
        ADC_ChannelConfig(ADC1, ADC_AIN4_CHANNEL , ADC_SampleTime_71_5Cycles);
        ADC_ChannelConfig(ADC1, ADC_AIN5_CHANNEL , ADC_SampleTime_71_5Cycles);

#else

	// 其它状况均设为配置3个通道
	// 配置规则组采样通道,使能通道并配置采样保持时间
	ADC_ChannelConfig(ADC1, ADC_AIN1_CHANNEL , ADC_SampleTime_71_5Cycles);
	ADC_ChannelConfig(ADC1, ADC_AIN2_CHANNEL , ADC_SampleTime_71_5Cycles);
	ADC_ChannelConfig(ADC1, ADC_AIN3_CHANNEL , ADC_SampleTime_71_5Cycles);

#endif
	
	// ADC1规则组外部触发使能
//	ADC_ExternalTrigConvCmd(ADC1, ENABLE);						// ADC_CR寄存器的EXTTRIG位

// --------------------------- ADC模块自校验 ---------------------------------	
	// AD采样模块自校验，开始自校验前需停止ADC模块，ADON位	
	ADC_GetCalibrationFactor(ADC1);

	
// -------------------------------- DMA配置 ------------------------------------	
	// 规则组采样通过DMA总线传输到RAM中，注入组数据存放在ADC模块寄存器中，无需DMA读取
	// DMA配置，只有ADC1有DMA请求，ADC2同时被传输
	
	ADC_DMARequestModeConfig(ADC1, ADC_DMAMode_Circular);	

	// ADC模块DMA使能，否则会读不到ADC2的数据，ADC2无DMA功能，由ADC1代替
	ADC_DMACmd(ADC1, ENABLE);									// ADC_CR2寄存器的DMA位	
	
	DMA_DeInit(ADC_DMA);		  												// 开启DMA1的第一通道
	DMA_InitStructure.DMA_PeripheralBaseAddr = DR_ADDRESS;		  					// DMA对应的外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&v->ADCBuffer;   					// 内存存储基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;								// DMA的转换模式为SRC模式，由外设搬移到内存
	DMA_InitStructure.DMA_BufferSize = ADC_CHANNEL_NUMBER;		   					// DMA缓存大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;				// 接收一次数据后，设备地址不后移
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;							// 接收一次数据后，目标内存地址后移
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;		// 定义外设数据宽度为16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;  			// 目标存储区RAM数据宽度为16位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;   								// 转换模式，循环模式。
//	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;   								// 转换模式，正常模式。
	DMA_InitStructure.DMA_Priority = ADC_DMA_PRIORITY;								// DMA优先级非常高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;		  							// M2M模式禁用
	DMA_Init(ADC_DMA, &DMA_InitStructure);  

	// 使能DMA1通道1
	DMA_Cmd(ADC_DMA, ENABLE);	
	
	ADC_Cmd(ADC1, ENABLE); 
	
	ADC_StartOfConversion(ADC1);
	
	// Take care: 
	// There is one lowpass filter in The ADC result transform to resistance. So need initialize the resistance.
	// Initialize the NTC to a NORMAL or LARGE value.
	// Now design to 10K ohm.
	v->Output.NTCRes1 = 10000;			// The unit is 1 ohm.
	v->Output.NTCRes2 = 10000;			// The unit is 1 ohm.
	v->Output.NTCRes3 = 10000;			// The unit is 1 ohm.
}

//u16 t_ADCDelay = 0;
// In BMS, slow update is designed run 200ms once.   //ADC采集BMS中200ms触发一次
// Design one lowpass filter. The filter frequency is 0.26Hz, Parameter A is 0.75
void ADC_SlowUpdate(ADC_structDef* v)
{
	s32	l_temp = 0;
	s32	temp_1 = 0;
	// R = (ADC * 5100) / (1 * 4096 - ADC)			// 计算外部电阻阻值，单位1欧姆，上拉电阻=5.1K，放大比例=1
//	v->Output.NTCRes1 = (s32)v->ADCResult[0] * 5100 / ((s32)4096 - (s32)v->ADCResult[0]);		// 外部电阻，单位1欧姆
//	v->Output.NTCRes2 = (s32)v->ADCResult[1] * 5100 / ((s32)4096 - (s32)v->ADCResult[1]);		// 外部电阻，单位1欧姆
//	v->Output.NTCRes3 = (s32)v->ADCResult[2] * 5100 / ((s32)4096 - (s32)v->ADCResult[2]);		// 外部电阻，单位1欧姆
	
	// m0内部AD采样仅支持顺序通道采样，默认方向通道0->16，若定义通道顺序乱序，需要重新对应
	// 根据不同芯片配置，重新排序AD采样输出
	

#if (CONTROLLER_TARGET == BMS_EP_200_B2_1)

	// 由于采样通道6，5，4，buffer存储顺序强制为4，5，6，所以需重新排序
	v->Output.ADCResult[0] = v->ADCBuffer[2];		// 通道6
	v->Output.ADCResult[1] = v->ADCBuffer[1];		// 通道5
	v->Output.ADCResult[2] = v->ADCBuffer[0];		// 通道4
	
#else
	
	v->Output.ADCResult[0] = v->ADCBuffer[0];//地址1
	v->Output.ADCResult[1] = v->ADCBuffer[1];//
	v->Output.ADCResult[2] = v->ADCBuffer[2];
	
#endif

#if (CONTROLLER_TARGET == BMS_EP_ALPHA)

    // R = (ADC * 5100) / (1 * 4096 - ADC)          // 计算外部电阻阻值，单位1欧姆，上拉电阻=5.1K，放大比例=1
    
    // 电池包NTC采样1              
    l_temp = (s32)v->ADCBuffer[1] * 5100 / ((s32)4096 - (s32)v->ADCBuffer[1]);      // 外部电阻，单位1欧姆
    
    // The filter parameter = 7 / 8 = 0.875, the filter frequency is about 0.1Hz
    v->Output.BatteryNTCRes[0] = (v->Output.BatteryNTCRes[0] * 3 + l_temp) >> 2;
    
    // 电池包NTC采样2
    l_temp = (s32)v->ADCBuffer[0] * 5100 / ((s32)4096 - (s32)v->ADCBuffer[0]);      // 外部电阻，单位1欧姆
    
    // The filter parameter = 7 / 8 = 0.875, the filter frequency is about 0.1Hz
    v->Output.BatteryNTCRes[1] = (v->Output.BatteryNTCRes[1] * 3 + l_temp) >> 2;

    // 均流板NTC采样1
    l_temp = (s32)v->ADCBuffer[2] * 5100 / ((s32)4096 - (s32)v->ADCBuffer[2]);      // 外部电阻，单位1欧姆
    
    // The filter parameter = 7 / 8 = 0.875, the filter frequency is about 0.1Hz
    v->Output.BalanceBoardNTCRes[0] = (v->Output.BalanceBoardNTCRes[0] * 3 + l_temp) >> 2;
    
    // 均流板NTC采样2
    l_temp = (s32)v->ADCBuffer[3] * 5100 / ((s32)4096 - (s32)v->ADCBuffer[3]);      // 外部电阻，单位1欧姆
    
    // The filter parameter = 7 / 8 = 0.875, the filter frequency is about 0.1Hz
    v->Output.BalanceBoardNTCRes[1] = (v->Output.BalanceBoardNTCRes[1] * 3 + l_temp) >> 2; 
    
	// 目前仅在BMS_EP_200_B2_1硬件上使用通道3,直接通过ADC采样结果计算温度，外部温传型号，LMT87
	l_temp = (s32)v->ADCBuffer[4] * 3300 / 4096;		// 采样值转换为实际电压，单位1mV
	
	v->Output.LMT87Voltage = (v->Output.LMT87Voltage * 3 + l_temp) >> 2;
    
    ADC_StartOfConversion(ADC1);

#else	
	// NTC resistance 1:                   //5100***************8
	l_temp = (s32)v->Output.ADCResult[0] * 15000 / ((s32)4096 - (s32)v->Output.ADCResult[0]);		// 外部电阻，单位1欧姆
	
	// The filter parameter = 7 / 8 = 0.875, the filter frequency is about 0.1Hz  
	v->Output.BatteryNTCRes[0] = (v->Output.BatteryNTCRes[0] * 3 + l_temp) >> 2;//电池包温度
	
	// NTC resistance 2:                                                        //板载NTC
	temp_1 = (s32)v->Output.ADCResult[1] * 15000 / ((s32)4096 - (s32)v->Output.ADCResult[1]);		// 外部电阻，单位1欧姆
	
	// The filter parameter = 7 / 8 = 0.875, the filter frequency is about 0.1Hz
	v->Output.NTCRes2 = (v->Output.NTCRes2 * 3 + temp_1) >> 2;
	
	// 目前仅在BMS_EP_200_B2_1硬件上使用通道3,直接通过ADC采样结果计算温度，外部温传型号，LMT87
	//l_temp = (s32)v->Output.ADCResult[2] * 3300 / 4096;		// 采样值转换为实际电压，单位1mV
	
	//v->Output.LMT87Voltage = (v->Output.LMT87Voltage * 3 + l_temp) >> 2;
	
	// NTC resistance 3:
  //l_temp = (s32)v->Output.ADCResult[2] * 5100 / ((s32)4096 - (s32)v->Output.ADCResult[2]);		// 外部电阻，单位1欧姆
	
	// The filter parameter = 7 / 8 = 0.875, the filter frequency is about 0.1Hz
//	v->Output.NTCRes3 = (v->Output.NTCRes3 * 7 + l_temp) >> 3;
	
     
	 v->Output.KEY_adc = v->Output.ADCResult[2]; //KEY钥匙检测  ADC
	 
	 
    ADC_StartOfConversion(ADC1);
//  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
#endif
}


#endif


// End of ADC.c

