/* ==================================================================================

 File name:     Output.c
 Originator:    BLJ
 Description:   驱动输出模块

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 04-11-2015		Version 1.0.0			正式版本
 01-15-2015     Version 0.0.1           测试功能
-----------------------------------------------------------------------------------*/

#include "target.h"
#include "Output.h"

// ---------------------------- 模块初始化 -----------------------------------

void LOCK_REST(void)
{
    GPIO_InitTypeDef GPIO_InitStructure; 

    RCC_AHBPeriphClockCmd(	0
//							| RCC_AHBPeriph_GPIOA
							| RCC_AHBPeriph_GPIOB
							| RCC_AHBPeriph_GPIOC
							| RCC_AHBPeriph_DMA1
							|0
							, ENABLE
						);	
	//电源控制脚   
    GPIO_InitStructure.GPIO_Pin = GPIO_LOCK_PIN;    		// 电源LOCK脚，推挽输出，高有效
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			// 开漏输出，禁用强推挽
    GPIO_Init(GPIO_LOCK_PORT, &GPIO_InitStructure);
	// 输出有效位，高有效
	GPIO_ResetBits(GPIO_LOCK_PORT, GPIO_LOCK_PIN);

}

void Output_DeviceInit(void)
{

	
#if (CONTROLLER_TARGET == BMS_EP_A5 || CONTROLLER_TARGET == BMS_EP_20_REV1_1_2TEMP || CONTROLLER_TARGET == BMS_EP_ALPHA || CONTROLLER_TARGET == BMS_EP_200_A2 || CONTROLLER_TARGET == BMS_EP_200_B1 || CONTROLLER_TARGET == BMS_EP_200_B2_1)					// EP 硬件第三版，带3个LED指示灯
		
	
	// 配置相应IO口为强推挽输出模式
	GPIO_InitTypeDef GPIO_InitStructure;     			// GPIO配置寄存器结构体
	//TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
   	//TIM_OCInitTypeDef  TIM_OCInitStructure;
	// Take care： LED采用外部供电3.3V，控制脚拉低使能模式，所以输出IO配置为开漏输出模式 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_LED_RED_PIN;    			// LED红色，开漏输出，低有效
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;		    // 开漏输出，禁用强推挽
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				// 开漏输出，禁用强推挽
    GPIO_Init(GPIO_LED_RED_PORT, &GPIO_InitStructure);
	
	// 输出无效位，关闭输出，低有效
	GPIO_SetBits(GPIO_LED_RED_PORT, GPIO_LED_RED_PIN);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_LED_GREEN_PIN;    		// LED绿色，开漏输出，低有效
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;			// 开漏输出，禁用强推挽
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				// 开漏输出，禁用强推挽
    GPIO_Init(GPIO_LED_GREEN_PORT, &GPIO_InitStructure);
    	
	// 输出无效位，关闭输出，低有效
	GPIO_SetBits(GPIO_LED_GREEN_PORT, GPIO_LED_GREEN_PIN);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_LED_BLUE_PIN;    		// LED蓝色，开漏输出，低有效
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;			// 开漏输出，禁用强推挽
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				// 开漏输出，禁用强推挽
	GPIO_Init(GPIO_LED_BLUE_PORT, &GPIO_InitStructure);
    	
	// 输出无效位，关闭输出，低有效
	GPIO_SetBits(GPIO_LED_BLUE_PORT, GPIO_LED_BLUE_PIN);


	GPIO_InitStructure.GPIO_Pin = GPIO_LED_YELLOW_PIN;    		// LED黄色，开漏输出，低有效
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;			// 开漏输出，禁用强推挽
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				// 开漏输出，禁用强推挽
    GPIO_Init(GPIO_LED_YELLOW_PORT, &GPIO_InitStructure);
    	
	// 输出无效位，关闭输出，低有效
	GPIO_SetBits(GPIO_LED_YELLOW_PORT, GPIO_LED_YELLOW_PIN);
    
    // 初始化蜂鸣器GPIO
    GPIO_InitStructure.GPIO_Pin = GPIO_BUZZER_PIN;    		// HORN_M，开漏输出，低有效
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;			// 开漏输出，禁用强推挽
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				// 开漏输出，禁用强推挽
    GPIO_Init(GPIO_BUZZER_PORT, &GPIO_InitStructure);
    	
	// 输出无效位，关闭输出，低有效
	GPIO_SetBits(GPIO_BUZZER_PORT, GPIO_BUZZER_PIN);
	
	
//	GPIO_InitStructure.GPIO_Pin = GPIO_PW_EN_PIN;    		// 
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;			// 开漏输出，禁用强推挽
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				// 开漏输出，禁用强推挽
//    GPIO_Init(GPIO_PW_EN_PORT, &GPIO_InitStructure);

//	GPIO_SetBits(GPIO_PW_EN_PORT, GPIO_PW_EN_PIN);// 输出无效位，关闭输出，低有效

	
	//DSGING控制引脚
	GPIO_InitStructure.GPIO_Pin = GPIO_DSGING_EN_PIN;    		// 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;			// 开漏输出，禁用强推挽
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				// 开漏输出，禁用强推挽
    GPIO_Init(GPIO_DSGING_EN_PORT, &GPIO_InitStructure);
	
	//GPIO_SetBits(GPIO_DSGING_EN_PORT, GPIO_DSGING_EN_PIN);// 输出无效位，关闭输出，低有效
    GPIO_ResetBits(GPIO_DSGING_EN_PORT,GPIO_DSGING_EN_PIN);
	
	//电源控制脚
	GPIO_SetBits(GPIO_LOCK_PORT, GPIO_LOCK_PIN);   
    GPIO_InitStructure.GPIO_Pin = GPIO_LOCK_PIN;    		// 电源LOCK脚，推挽输出，高有效
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			// 开漏输出，禁用强推挽
    GPIO_Init(GPIO_LOCK_PORT, &GPIO_InitStructure);
	// 输出有效位，高有效
	GPIO_SetBits(GPIO_LOCK_PORT, GPIO_LOCK_PIN);
	
	//AFE唤醒控制脚
	GPIO_SetBits(GPIO_EN_AFE_PORT, GPIO_EN_AFE_PIN);
    GPIO_InitStructure.GPIO_Pin = GPIO_EN_AFE_PIN;    		// AFE使能脚，开漏输出，低有效
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				// 开漏输出，禁用强推挽
    GPIO_Init(GPIO_EN_AFE_PORT, &GPIO_InitStructure);
	// 输出无效位，关闭输出，低有效
	GPIO_SetBits(GPIO_EN_AFE_PORT, GPIO_EN_AFE_PIN);
	
//==========================PDI输出引脚===================================
    
	//    GPIO_PinAFConfig(GPIO_PWM_OUT_PORT, GPIO_PinSource3, GPIO_AF_2); //功能预留不使用
//    GPIO_InitStructure.GPIO_Pin = GPIO_PWM_OUT_PIN;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIO_PWM_OUT_PORT, &GPIO_InitStructure);
	//GPIO_SetBits(GPIO_PWM_OUT_PORT, GPIO_PWM_OUT_PIN);
//=======================================================================

#elif (CONTROLLER_TARGET == BMS_XINXIN_A1 || CONTROLLER_TARGET == BMS_XINXIN_A2)				// 新昕滑板BMS待预放电功能
	
	
	// 配置相应IO口为强推挽输出模式
	GPIO_InitTypeDef GPIO_InitStructure;     			// GPIO配置寄存器结构体
	
	GPIO_InitStructure.GPIO_Pin = GPIO_PREDISCHARGE_PIN;    	// OUT1，强推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIO_PREDISCHARGE_PORT, &GPIO_InitStructure);
    	
	// 输出无效位，关闭输出，高有效
	GPIO_ResetBits(GPIO_PREDISCHARGE_PORT, GPIO_PREDISCHARGE_PIN);//电池预放电
	
	
#endif	


#if (CONTROLLER_TARGET == BMS_EP_200_B2_1)

	// 硬件带一个测试信号灯，低有效
	GPIO_InitStructure.GPIO_Pin = GPIO_LED_TEST_PIN;    		// 测试信号灯，开漏输出，低有效
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;				// 开漏输出，禁用强推挽
    GPIO_Init(GPIO_LED_TEST_PORT, &GPIO_InitStructure);
    	
	// 输出无效位，关闭输出，低有效
	GPIO_SetBits(GPIO_LED_TEST_PORT, GPIO_LED_TEST_PIN);


#endif



    //(96000/psc)/arr=khz

	
	
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);//10KHZ

//    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
//    TIM_TimeBaseStructure.TIM_Period = 4799;
//    TIM_TimeBaseStructure.TIM_Prescaler = 0;
//    //Setting Clock Segmentation
//    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
//    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
//    ///TIM Upward Counting Mode
//    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

//    TIM_OCStructInit(&TIM_OCInitStructure);
//    //Select Timer Mode: TIM Pulse Width Modulation Mode 2
//    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
//    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//    //Setting the Pulse Value of the Capture Comparison Register to be Loaded
//    TIM_OCInitStructure.TIM_Pulse = 0;
//    //Output polarity: TIM output is more polar
//    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
//    TIM_OC2Init(TIM2, &TIM_OCInitStructure);

//    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
//    TIM_ARRPreloadConfig(TIM2, ENABLE);
//    TIM_CtrlPWMOutputs(TIM2, ENABLE);

//    TIM_Cmd(TIM2, ENABLE);
//	TIM_SetCompare2(TIM2,0);
}


#if (CONTROLLER_TARGET == BMS_XINXIN_A1 || CONTROLLER_TARGET == BMS_XINXIN_A2)				// 新昕滑板BMS待预放电功能

// ---------------------------- 更新电池预放电控制 -------------------------------
void Output_PreDischarge_Update(Output_structDef* v, u8 NewState)
{
	switch (NewState)
	{
		case 0:				// 输出无效位
		{
			GPIO_ResetBits(GPIO_PREDISCHARGE_PORT, GPIO_PREDISCHARGE_PIN);	// 输出无效位，关闭输出，高有效

			v->State.bit.PreDischargeEnable = 0;
		}
		break;
		
		default:			// 输出有效位
		{
			GPIO_SetBits(GPIO_PREDISCHARGE_PORT, GPIO_PREDISCHARGE_PIN);		// 输出有效位，使能输出，高有效

			v->State.bit.PreDischargeEnable = 1;			
		}
		break;
	}
}

#endif				// CONTROLLER_TARGET == BMS_XINXIN_A1 || CONTROLLER_TARGET == BMS_XINXIN_A2	

		
// -------------------------- 外部驱动1，out1输出更新 ---------------------------------
void Output_Out1_Update(u16 State)
{
/*	switch (State)
	{
		case 0:				// 0: 输出无效位
		{
			GPIO_ResetBits(GPIO_OUT1_PORT, GPIO_OUT1_PIN);			// 高有效
		}
		break;
		
		default:			// 其它情况：输出有效位
		{
			GPIO_SetBits(GPIO_OUT1_PORT, GPIO_OUT1_PIN);			// 高有效
		}
		break;
	}*/
}

/*
// -------------------------- 外部驱动2，out2输出更新 ---------------------------------
void Output_Out2_Update(u16 State)
{
	switch (State)
	{
		case 0:				// 0: 输出无效位
		{
			GPIO_ResetBits(GPIO_OUT2_PORT, GPIO_OUT2_PIN);			// 高有效
		}
		break;
		
		default:			// 其它情况：输出有效位
		{
			GPIO_SetBits(GPIO_OUT2_PORT, GPIO_OUT2_PIN);			// 高有效
		}
		break;
	}
}
*/


#if (CONTROLLER_TARGET == BMS_EP_A5 || CONTROLLER_TARGET == BMS_EP_20_REV1_1_2TEMP || CONTROLLER_TARGET == BMS_EP_ALPHA || CONTROLLER_TARGET == BMS_EP_200_A2 || CONTROLLER_TARGET == BMS_EP_200_B1 || CONTROLLER_TARGET == BMS_EP_200_B2_1)					// EP 硬件第三版，带3个LED指示灯

// -------------------------- LED红灯输出更新 ---------------------------------
void Output_LED_RED_Update(Output_structDef* v, u16 State)
{
	switch (State)
	{
		case 0:				// 0: 输出无效位
		{
			GPIO_SetBits(GPIO_LED_RED_PORT, GPIO_LED_RED_PIN);			// 低有效
			
			v->State.bit.LED_RED = 0;
		}
		break;
		
		default:			// 其它情况：输出有效位
		{
			GPIO_ResetBits(GPIO_LED_RED_PORT, GPIO_LED_RED_PIN);		// 低有效
			
			v->State.bit.LED_RED = 1;
		}
		break;
	}
}

// -------------------------- LED绿灯输出更新 ---------------------------------
void Output_LED_GREEN_Update(Output_structDef* v, u16 State)
{
	switch (State)
	{
		case 1:				// 0: 输出无效位
		{
			GPIO_SetBits(GPIO_LED_GREEN_PORT, GPIO_LED_GREEN_PIN);		// 低有效
			
			v->State.bit.LED_GREEN = 0;
		}
		break;
		
		default:			// 其它情况：输出有效位
		{
			GPIO_ResetBits(GPIO_LED_GREEN_PORT, GPIO_LED_GREEN_PIN);	// 低有效
			
			v->State.bit.LED_GREEN = 1;
		}
		break;
	}
}

// -------------------------- LED蓝灯输出更新 ---------------------------------
void Output_LED_BLUE_Update(Output_structDef* v, u16 State)
{
	switch (State)
	{
		case 1:				// 0: 输出无效位
		{
			GPIO_SetBits(GPIO_LED_BLUE_PORT, GPIO_LED_BLUE_PIN);			// 低有效
			
			v->State.bit.LED_BLUE = 0;
		}
		break;
		
		default:			// 其它情况：输出有效位
		{
			GPIO_ResetBits(GPIO_LED_BLUE_PORT, GPIO_LED_BLUE_PIN);		// 低有效
			
			v->State.bit.LED_BLUE = 1;
		}
		break;
	}
}

// -------------------------- LED蓝灯输出更新 ---------------------------------
void Output_LED_YELLOW_Update(Output_structDef* v, u16 State)
{
	switch (State)
	{
		case 1:				// 0: 输出无效位
		{
			GPIO_SetBits(GPIO_LED_YELLOW_PORT, GPIO_LED_YELLOW_PIN);			// 低有效
			
			v->State.bit.LED_YELLOW = 0;
		}
		break;
		
		default:			// 其它情况：输出有效位
		{
			GPIO_ResetBits(GPIO_LED_YELLOW_PORT, GPIO_LED_YELLOW_PIN);		// 低有效
			
			v->State.bit.LED_YELLOW = 1;
		}
		break;
	}
}

// -------------------------- 蜂鸣器输出更新 ---------------------------------
void Output_BUZZER_Update(Output_structDef* v, u16 State)
{
	switch (State)
	{
		case 1:				// 0: 输出无效位
		{
			GPIO_SetBits(GPIO_BUZZER_PORT, GPIO_BUZZER_PIN);			// 低有效
			
			v->State.bit.Output1 = 0;
		}
		break;
		
		default:			// 其它情况：输出有效位
		{
			GPIO_ResetBits(GPIO_BUZZER_PORT, GPIO_BUZZER_PIN);		// 低有效
			
			v->State.bit.Output1 = 1;
		}
		break;
	}
}


void Output_LOCK_Update(Output_structDef* v, u16 State) //进入低功耗用
{
	switch (State)
	{
		case 1:				// 0: 输出无效位
		{
			GPIO_SetBits(GPIO_LOCK_PORT, GPIO_LOCK_PIN);			// 低有效
		}
		break;
		
		default:			// 其它情况：输出有效位
		{
			GPIO_ResetBits(GPIO_LOCK_PORT, GPIO_LOCK_PIN);		// 低有效
		}
		break;
	}
}


#endif				


#if (CONTROLLER_TARGET == BMS_EP_200_B2_1)				// 此硬件带测试灯

// ---------------------------- 更新测试灯控制 -------------------------------
void Output_TestLED_Update(Output_structDef* v, u8 NewState)
{
	switch (NewState)
	{
		case 0:				// 输出无效位
		{
			GPIO_SetBits(GPIO_LED_TEST_PORT, GPIO_LED_TEST_PIN);				// 输出无效位，关闭输出，低有效

			v->State.bit.TestLED = 0;
		}
		break;
		
		default:			// 输出有效位
		{
			GPIO_ResetBits(GPIO_LED_TEST_PORT, GPIO_LED_TEST_PIN);				// 输出有效位，关闭输出，低有效

			v->State.bit.TestLED = 1;			
		}
		break;
	}
}

#endif				


// End of Output.c

