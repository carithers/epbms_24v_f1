/* ==================================================================================

 File name:     Iutput.c
 Originator:    BLJ
 Description:   数字输入模块

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 04-11-2015     Version 1.0.0           正式版本
 01-15-2015     Version 0.0.1           测试功能
-----------------------------------------------------------------------------------*/

#include "target.h"
#include "Input.h"
#include "system_core.h"

// ---------------------------- 模块初始化 -----------------------------------

//void EXTI_ROUSE_Init(void)
//{

//// 配置相应IO口为强推挽输出模式
//GPIO_InitTypeDef GPIO_InitStructure;                        // GPIO配置寄存器结构体
//EXTI_InitTypeDef EXTI_InitStruct;
//NVIC_InitTypeDef NVIC_InitStructure;
//RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);   //系统配置时钟

//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);  //开启GPIOA时钟
//GPIO_InitStructure.GPIO_Pin = GPIO_KEY_PIN;                 // KEY，高有效
//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
//GPIO_Init(GPIO_KEY_PORT, &GPIO_InitStructure);

///*选择PB15位外部事件触发输入*/
//SYSCFG_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource15);

///*使能外部中断0,设置为事件模式,下降沿触发*/
//EXTI_InitStruct.EXTI_Line = EXTI_Line15;
//EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;//EXTI_Trigger_Falling;
//EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
//EXTI_InitStruct.EXTI_LineCmd = ENABLE;
//EXTI_Init(&EXTI_InitStruct);
///*不配置可嵌入中断向量,只做唤醒事件,对应位没有挂起,所以无需清除事件位*/

//NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;			//使能按键所在的外部中断通道
//NVIC_InitStructure.NVIC_IRQChannelPriority = 0x02;			//子优先级2
//NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//使能外部中断通道
//NVIC_Init(&NVIC_InitStructure);  	                        //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

//EXTI_ClearFlag(EXTI_Line15);//清除中断标志

//}


void Input_DeviceInit(void)
{
    // 配置相应IO口为强推挽输出模式
//    GPIO_InitTypeDef GPIO_InitStructure;                        // GPIO配置寄存器结构体
//    EXTI_InitTypeDef EXTI_InitStruct;
//	NVIC_InitTypeDef NVIC_InitStructure;
	
//    GPIO_InitStructure.GPIO_Pin = GPIO_ALERT_PIN;               // BQ769x0 芯片报错指示，高有效
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
////  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
////  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;    
//    GPIO_Init(GPIO_ALERT_PORT, &GPIO_InitStructure);
//===================================================钥匙检测采用ADC检测了
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);   //系统配置时钟

//	GPIO_InitStructure.GPIO_Pin = GPIO_KEY_PIN;                 // KEY，高有效
//    GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IN_FLOATING;// GPIO_Mode_IPU;  
//	
//    GPIO_Init(GPIO_KEY_PORT, &GPIO_InitStructure);

    
    /*选择PB15位外部事件触发输入*/
//    SYSCFG_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource15);
//    
//    /*使能外部中断0,设置为事件模式,下降沿触发*/
//    EXTI_InitStruct.EXTI_Line = EXTI_Line15;
//    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising,//EXTI_Trigger_Rising;//上升沿触发  
//    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
//    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
//    EXTI_Init(&EXTI_InitStruct);
//    /*不配置可嵌入中断向量,只做唤醒事件,对应位没有挂起,所以无需清除事件位*/
//	
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;			//使能按键所在的外部中断通道
//    NVIC_InitStructure.NVIC_IRQChannelPriority = 0x02;			//子优先级2
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//使能外部中断通道
//    NVIC_Init(&NVIC_InitStructure);  	  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
//	
//	EXTI_ClearFlag(EXTI_Line15);//清除中断标志
	//EXTI4_15_IRQHandler
    
    
#if (CONTROLLER_TARGET == BMS_EP_200_A1 || CONTROLLER_TARGET == BMS_EP_200_A2 || CONTROLLER_TARGET == BMS_EP_200_B1 || CONTROLLER_TARGET == BMS_EP_200_B2_1)    
    
    GPIO_InitStructure.GPIO_Pin = GPIO_CHG_PIN;                 // CHG，充电控制信号，高有效
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;    
    GPIO_Init(GPIO_CHG_PORT, &GPIO_InitStructure);  
    
    GPIO_InitStructure.GPIO_Pin = GPIO_DSG_PIN;                 // DSG，放电控制信号，高有效
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;    
    GPIO_Init(GPIO_DSG_PORT, &GPIO_InitStructure);  
        
#endif                  // CONTROLLER_TARGET == BMS_EP_200_A1
    
}
    
// -------------------------- 数字输入口更新,运行在1ms定时中断中 ---------------------------------
void Input_Update(Input_structDef* v)
{
    // 读取外部6路数字输入，低有效，所以取反
    // ALERT信号250ms周期不确定性地快速高低电平切换，所以暂时不采集。
   
    //v->Variable.DINInput_bits.bit.KEY =GPIO_ReadInputDataBit(GPIO_KEY_PORT, GPIO_KEY_PIN);//钥匙端口采集 //================================
	if(g_ADC.Output.KEY_adc > 1055) //0.8v 992
    {
        v->Variable.DINInput_bits.bit.KEY = 1;
    } else if(g_ADC.Output.KEY_adc < 930){
        v->Variable.DINInput_bits.bit.KEY = 0;
    }
    
#if (CONTROLLER_TARGET == BMS_EP_200_A1 || CONTROLLER_TARGET == BMS_EP_200_A2 || CONTROLLER_TARGET == BMS_EP_200_B1 || CONTROLLER_TARGET == BMS_EP_200_B2_1)    

    v->Variable.DINInput_bits.bit.CHG = GPIO_ReadInputDataBit(GPIO_CHG_PORT, GPIO_CHG_PIN);
    v->Variable.DINInput_bits.bit.DSG = GPIO_ReadInputDataBit(GPIO_DSG_PORT, GPIO_DSG_PIN);
    
#endif                  // CONTROLLER_TARGET == BMS_EP_200_A1
    
    if (v->Variable.DINFilter_bits.all == v->Variable.DINInput_bits.all)
    {
        v->Variable.DINFilterCount++;
        
        if (v->Variable.DINFilterCount >= 50)
        {           
            v->Variable.DINFilterCount = 0;
            
            v->Output.DIN_bits.all = v->Variable.DINFilter_bits.all;            
        }
    }
    else
    {
        v->Variable.DINFilter_bits.all = v->Variable.DINInput_bits.all;
        
        v->Variable.DINFilterCount = 0;
    }
}


// End of Input.c

