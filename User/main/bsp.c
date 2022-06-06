/* ==================================================================================

 File name:     bsp.c
 Originator:    BLJ
 Description:   DSP硬件初始化，外设初始化，状态机配置，中断配置

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 12-05-2014     Version 0.1.0           基本架构搭建完成
 10-07-2014     Version 0.0.1           测试功能
-----------------------------------------------------------------------------------*/
#include "stm32f0xx.h"                      // STM32器件寄存器定义头文件
#include "string.h"                         // 调用memset函数用
#include "system_core.h"                    // 系统核心头文件


// --------------------- 中断向量表重映射用宏定义，禁止修改 --------------------------
#if   (defined ( __CC_ARM ))
  __IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));
#elif (defined (__ICCARM__))
#pragma location = 0x20000000
  __no_init __IO uint32_t VectorTable[48];
#elif defined   (  __GNUC__  )
  __IO uint32_t VectorTable[48] __attribute__((section(".RAMVectorTable")));
#elif defined ( __TASKING__ )
  __IO uint32_t VectorTable[48] __at(0x20000000);
#endif


// ------------------------------ 状态机配置 -------------------------------------
// Take care： m0芯片需更新
// 不暂停中断列表
enum KernelUnawareISRs {                               
    // ... 不暂停中断，如高速中断，电流环中断
    MAX_KERNEL_UNAWARE_CMSIS_PRI						// keep always last
};

// "kernel-unaware" interrupts can't overlap "kernel-aware" interrupts
Q_ASSERT_COMPILE(MAX_KERNEL_UNAWARE_CMSIS_PRI <= QF_AWARE_ISR_CMSIS_PRI);

// 可暂停中断列表
enum KernelAwareISRs {
    SYSTICK_PRIO = QF_AWARE_ISR_CMSIS_PRI,                  // see NOTE00
    // ... 填入可暂停中断，一般我不再设置更多中断
    MAX_KERNEL_AWARE_CMSIS_PRI                          // keep always last
};

// "kernel-aware" interrupts should not overlap the PendSV priority
Q_ASSERT_COMPILE(MAX_KERNEL_AWARE_CMSIS_PRI <= (0xFF >>(8-__NVIC_PRIO_BITS)));


//void HardFault_Handler(void)
//{	
//	while(1)
//	{
//		
//	}
//}

//u16 EXTI_test=0;

//低功耗唤醒中断
//void EXTI4_15_IRQHandler(void)
//{
// // EXTI_test++;

// //NVIC_SystemReset(); //软件复位
//	
// EXTI_ClearFlag(EXTI_Line15);//清除中断标志
//}

// ------------------------- 系统时基中断，1ms ---------------------------
void SysTick_Handler(void) 
{
    // 状态机定时器0,1时基，默认1ms
    QF_tickXISR(0U);          				// process time events at rate 0
    QF_tickXISR(1U);          				// process time events at rate 1    
    QF_tickXISR(2U);          				// process time events at rate 2 
    System_1msInterrupt();					// 系统1ms定时时基
}

// ------------------------- I2C中断，默认50us，优先级高于SysTick ----------------------
/**
  * @brief  This function handles TIM6 global interrupt request.
  * @param  None
  * @retval None
  */
//s32 l_I2CInterruptEnterTS = 0;              /* I2C中断进入时Systick->VAL */
//void TIM2_IRQHandler(void)
//{
//    /*必须手动清除中断标志位*/
//	
//    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
//}

// --------------------------- 状态机启动 ----------------------------
void QF_onStartup(void) 
{

//	u16 i = 0;

// --------------- Relocate by software the vector table to the internal SRAM at 0x20000000 ------------------

	// Take care：由于中断向量表映射到ram区起始地址，注意在配置参数中将该区域空闲出来，避免重叠！！！！！
	
	// Copy the vector table from the Flash (mapped at the base of the application
	// load address 0x08002000) to the base address of the SRAM at 0x20000000.
	// 将位于flash首地址的中断向量表复制到ram中，地址0x20000000
//	for(i = 0; i < 48; i++)
//	{
//		VectorTable[i] = *(__IO uint32_t*)(APPLICATION_ADDRESS + (i<<2));
//	}

//	// Enable the SYSCFG peripheral clock
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); 
//	
//	// Remap SRAM at 0x00000000 
//	SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);
  
	
	// 设置中断向量表位于FLASH，由于使用IAP，地址向后偏移0x2000
	//NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x2000); 
	//NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000); 			// 正常启动，地址不偏移
	
    // assing all priority bits for preemption-prio. and none to sub-prio.
	// 全部优先级用于抢占式中断。
	// preemption-prio：抢占式，当前中断可被更高优先级中断打断
	// sub-prio：规则式，更高优先级中断会在当前中断执行完成后才被执行
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

/*	
    // 设置中断的优先级，优先级0，1，2无法被状态机打断
	// 定时器1更新中断，优先级2
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
*/

/*	NVIC_InitStructure.NVIC_IRQChannel = SysTick_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);*/
	
    // set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate
	// 设置SYSTICK中断频率，默认1ms，低优先级为7，待确认
    
}

// --------------------------- 状态机空闲函数 -------------------------
void QF_onIdle(void) 
{     

#ifdef NDEBUG			// 不执行，不进入睡眠
	
    /* Put the CPU and peripherals to the low-power mode.
    * you might need to customize the clock management for your application,
    * see the datasheet for your particular Cortex-M3 MCU.
    */
    QF_CPU_SLEEP();         /* atomically go to sleep and enable interrupts */
	
#else
    
	QF_INT_ENABLE();                    // 使能相应中断
	
    System_MainLoop();					// 系统主循环函数
 
#endif
	
}

// -------------------------- 故障处理 -------------------------------
void assert_failed(uint8_t* file, uint32_t line) {
//void assert_failed(char const *file, int line) {
    (void)file;          		// avoid compiler warning
    (void)line;             	// avoid compiler warning
    QF_INT_DISABLE();         	// make sure that all interrupts are disabled
	
	
	// NVIC_SetPriority();

	// Take care， To be update， 增加关闭输出函数，重要
}

// -------------------------- 状态机错误处理 ---------------------------
void Q_onAssert(char const Q_ROM * const file, int line) {
//    assert_failed(file, line);
}


// -------------- 系统时钟RCC初始化函数，配置系统时钟，晶振，PLL ----------------
void RCC_Config(void)
{   
    ErrorStatus HSEStartUpStatus;                	// 外部高速晶振（HSE）启动状态

    u16 i = 0;
    
// --------------- Relocate by software the vector table to the internal SRAM at 0x20000000 ------------------

	// Take care：由于中断向量表映射到ram区起始地址，注意在配置参数中将该区域空闲出来，避免重叠！！！！！
	
	// Copy the vector table from the Flash (mapped at the base of the application
	// load address 0x08002000) to the base address of the SRAM at 0x20000000.
	// 将位于flash首地址的中断向量表复制到ram中，地址0x20000000
	for(i = 0; i < 48; i++)
	{
		VectorTable[i] = *(__IO uint32_t*)(APPLICATION_ADDRESS + (i<<2));
	}

	// Enable the SYSCFG peripheral clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); 
	
	// Remap SRAM at 0x00000000 
	SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);
	
	
     
    
    RCC_DeInit();                                	// RCC系统复位(用于调试)   
//    RCC_HSEConfig(RCC_HSE_ON);                   	// 使能外部高速晶振（HSE）
//    
//    HSEStartUpStatus = RCC_WaitForHSEStartUp();  	// 等待外部高速晶振（HSE）稳定
//    if(HSEStartUpStatus == SUCCESS)                          // 外部晶振稳定
//    {
        // 配置flash
        FLASH_PrefetchBufferCmd(ENABLE);           //使能预取缓冲
        FLASH_SetLatency(FLASH_Latency_0);         	// 设置Flash等待延时 1等待延时 
        
        // 配置时钟
        RCC_HCLKConfig(RCC_SYSCLK_Div1);           	// 设置AHB时钟，HCLK = SYSCLK  
		RCC_PCLKConfig(RCC_HCLK_Div1);				// 设置APB时钟，PCLK = HCLK
		
		// Take care: ADC时钟取决于ADC时钟模式，详情查看ADC_ClockModeConfig();
		RCC_ADCCLKConfig(RCC_ADCCLK_PCLK_Div4);		// 设置ADC时钟，ADCCLK = PCLK / 2
		
		RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);		// 设置I2C模块时钟，I2CCLK = SYSCLK
	
        //RCC_PCLK1Config(RCC_HCLK_Div2);            	// 设置低速AHB时钟，PCLK1 = HCLK/2      
        //RCC_PCLK2Config(RCC_HCLK_Div1);            	// 设置高速AHB时钟，PCLK2 = HCLK     

        // 配置锁相环PLL，
        RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_4);        //PLLCLK=4MHz*4=16 MHz 
        RCC_PLLCmd(ENABLE);                        //使能PLL
		
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)          //等待PLL稳定
        {
        }
        
        // 配置系统时钟源
        // 0x00：HSI作为系统时钟源
        // 0x04：HSE作为系统时钟源
        // 0x08：PLL作为系统时钟源
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);  //选择PLL作为系统时钟源RCC_SYSCLKSource_PLLCLK		 //RCC_SYSCLKSource_HSE
        while(RCC_GetSYSCLKSource() != 0x08)        //等待HSE被用作系统时钟源
        {
        }
//    }
//    else
//    {
//		// 当外部HSE无法稳定时，使用内部HSI晶振，8MHz，并报错
//		
//		// 使能内部8MHz晶振
//		RCC_HSICmd(ENABLE);

//		// 配置系统时钟源
//		// 0x00：HSI作为系统时钟源
//		// 0x04：HSE作为系统时钟源
//		// 0x08：PLL作为系统时钟源
//		RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);  		//选择HSI作为系统时钟源
//		while(RCC_GetSYSCLKSource() != 0x00)        	//等待HSI被用作系统时钟源
//		{   
//			// 增加外部故障信号输出
//		}	
//	
//        // 配置flash
//        FLASH_PrefetchBufferCmd(ENABLE);           //使能预取缓冲
//        FLASH_SetLatency(FLASH_Latency_0);         	// 设置Flash等待延时 1等待延时 
//		
//		// ------------- 设置晶振起振失败标志位 -------------------
//		Protect_SetFaultCodeLv0(&g_Protect, FAULT_OSC);
//    }
	
	// 使能GPIO及DMA！！！！！
	RCC_AHBPeriphClockCmd(	0
							| RCC_AHBPeriph_GPIOA
							| RCC_AHBPeriph_GPIOB
							| RCC_AHBPeriph_GPIOC
							| RCC_AHBPeriph_DMA1
							|0
							, ENABLE
						);

	// Take care: 默认外设模块由外设模块初始化时开启时钟
/*	RCC_APB1PeriphClockCmd( 0						*/
/*							| RCC_APB1Periph_TIM2	*/
/*							| RCC_APB1Periph_TIM3	*/	
/*							| RCC_APB1Periph_TIM4	*/		/* unused */
/*							| RCC_APB1Periph_TIM5	*/		/* unused */
/*							| RCC_APB1Periph_TIM6	*/		/* unused */
/*							| RCC_APB1Periph_TIM7	*/		/* unused */
/*							| RCC_APB1Periph_WWDG	*/		/* unused */
//							| RCC_APB1Periph_SPI2
/*							| RCC_APB1Periph_SPI3	*/
/*							| RCC_APB1Periph_USART2	*/		/* unused */
//							| RCC_APB1Periph_USART3
/*							| RCC_APB1Periph_UART4	*/		/* unused */
/*							| RCC_APB1Periph_UART5	*/		/* unused */
/*							| RCC_APB1Periph_I2C1	*/
/*							| RCC_APB1Periph_I2C2	*/		/* unused */
/*							| RCC_APB1Periph_USB	*/		/* unused */
/*							| RCC_APB1Periph_CAN	*/
/*							| RCC_APB1Periph_BKP
							| RCC_APB1Periph_PWR*/
/*							| RCC_APB1Periph_DAC	*/		/* unused */
/*							| RCC_APB1Periph_CEC	*/		/* unused */
/*							| RCC_APB1Periph_TIM12	*/		/* unused */
/*							| RCC_APB1Periph_TIM13	*/		/* unused */
/*							| RCC_APB1Periph_TIM14	*/		/* unused */
/*							| 0
							, ENABLE
						 );							*/

/*	RCC_APB2PeriphClockCmd( 0						*/
/*							| RCC_APB2Periph_AFIO
							| RCC_APB2Periph_GPIOA
							| RCC_APB2Periph_GPIOB
							| RCC_APB2Periph_GPIOC
							| RCC_APB2Periph_GPIOD
							| RCC_APB2Periph_GPIOE
							| RCC_APB2Periph_GPIOF	
							| RCC_APB2Periph_GPIOG	
							| RCC_APB2Periph_ADC1
//							| RCC_APB2Periph_ADC2*/
/*							| RCC_APB2Periph_TIM1	*/
/*							| RCC_APB2Periph_SPI1	*/
/*							| RCC_APB2Periph_TIM8	*/
//							| RCC_APB2Periph_USART1 
/*							| RCC_APB2Periph_ADC3	*/
/*							| RCC_APB2Periph_TIM15	*/
/*							| RCC_APB2Periph_TIM16	*/
/*							| RCC_APB2Periph_TIM17	*/
/*							| RCC_APB2Periph_TIM9	*/
/*							| RCC_APB2Periph_TIM10	*/
//							| RCC_APB2Periph_TIM11	
/*							, ENABLE
						  );						*/
						   
    // 未使用的GPIO将被保持在高阻输入状态
	// STM32F042只有PA,PB，貌似乱设为会导致芯片死锁
/*    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_All; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN; //输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;		// 无上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);  
    GPIO_Init(GPIOB, &GPIO_InitStructure);  
    GPIO_Init(GPIOC, &GPIO_InitStructure);  
    GPIO_Init(GPIOD, &GPIO_InitStructure);  
    GPIO_Init(GPIOE, &GPIO_InitStructure); 	
    GPIO_Init(GPIOF, &GPIO_InitStructure);  */
//    GPIO_Init(GPIOG, &GPIO_InitStructure); 	

	
}


// -------------------------- 系统数据清零，初始化时必须执行 --------------------------------
void Data_Init(void)
{  
	
    memset(&g_AO_EEPROM, 0, sizeof(g_AO_EEPROM)); 
    memset(&g_AO_SH36730x0, 0, sizeof(g_AO_SH36730x0));                           
    memset(&g_AO_BMS, 0, sizeof(g_AO_BMS));  
    
//  memset(&g_CAN, 0, sizeof(g_CAN));                           // CAN模块
	memset(&g_I2C_SH36730x0, 0, sizeof(g_I2C_SH36730x0));       // BQ769x0 I2C通信模块
	memset(&g_flash_EEPROM, 0, sizeof(g_flash_EEPROM));         // flash模拟EEPROM块
	memset(&g_USART_MODBUS, 0, sizeof(g_USART_MODBUS));         // 串口MODBUS通讯模块	

	memset(&g_Protect, 0, sizeof(g_Protect));             		// Protect模块	

	memset(&g_Input, 0, sizeof(g_Input));             			// 数字输入模块	
	memset(&g_ADC, 0, sizeof(g_ADC));             				// ADC采样模块	
	memset(&g_LED_Flicker, 0, sizeof(g_LED_Flicker));           // LED闪烁模块		
	
    memset(&g_communication, 0, sizeof(g_communication));       // 设备间通信数据结构体
	
	memset(&g_SystemState, 0, sizeof(g_SystemState));       	// 设备状态	
	memset(&g_SystemParameter, 0, sizeof(g_SystemParameter));   // 设备参数
	memset(&g_SystemMonitor, 0, sizeof(g_SystemMonitor));   	// 系统采样值
	memset(&g_SystemRecord, 0, sizeof(g_SystemRecord));       	// 设备运行记录
	
	// 状态机会自己初始化，无需外部介入
}

//==================MM32 中断向量表复制，放到RAM中===============================

//void IAP_Config(void)
//{
//    uint32_t i = 0;
//   //memcpy((void*)0x20000000, (void*)APP_START_ADDRESS, 0xC0);  //把中断向量表映射到RAM里面跑  （M0）
//    for(i = 0; i < 48; i++)
//    {
//        *((uint32_t *)(SRAM_BASE + (i * 4))) = *(uint32_t *)(APP_START_ADDRESS + (i * 4));
//    }

//    /* Enable the SYSCFG peripheral clock*/ 
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

//    /* Remap SRAM at 0x00000000 */
//    SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM); 
//}

//===========================================================================
RCC_ClocksTypeDef  clk;
//RCC_GetClocksFreq(&clk);

extern void SystemInit(void);
// ----------------------------------- DSP硬件初始化函数 -----------------------------------
void System_Init(void)
{
   
    RCC_Config();                   		// 配置为外部高频晶振，默认无PLL，主频8MHz

	Data_Init();							// 数据变量初始化
	
	flash_EEPROM_DeviceInit();				// flash模拟EEPROM硬件初始化，解锁flash
	
	
    
#if (CONTROLLER_TARGET == BMS_EP_A5 || CONTROLLER_TARGET == BMS_EP_20_REV1_1_2TEMP || CONTROLLER_TARGET == BMS_EP_ALPHA || CONTROLLER_TARGET == BMS_EP_200_A2 || CONTROLLER_TARGET == BMS_XINXIN_A1 || CONTROLLER_TARGET == BMS_EP_200_B1 || CONTROLLER_TARGET == BMS_EP_200_B2_1)
	
	ADC_DeviceInit(&g_ADC);					// ADC采样模式初始化
	
#endif
	
	
	
	Input_DeviceInit();						// 数字输入IO口初始化

    Output_DeviceInit();					// 驱动模块初始化//======================================

	I2C_BQ769xx_DeviceInit();				// BQ769xx芯片I2C总线通信模块初始化//IIC_STOP 注释了延时注意***
	
	GPIO_SetBits(GPIO_LOCK_PORT, GPIO_LOCK_PIN);
	
 /* g_USART_MODBUS.Parameter.DataBlock[2].pBuffer = (u8*)&g_communication.ReceiveFrame; //区块2，接受仪表指令
    g_USART_MODBUS.Parameter.DataBlock[2].BlockLength = 6;
    g_USART_MODBUS.Parameter.DataBlock[2].BlockStartAddress = 0x1000;

    g_USART_MODBUS.Parameter.DataBlock[3].pBuffer = (u8*)&g_communication.SendFrame;
    g_USART_MODBUS.Parameter.DataBlock[3].BlockLength = 23;
    g_USART_MODBUS.Parameter.DataBlock[3].BlockStartAddress = 0x4000;*/
			
// 不再支持旧版无串口硬件
//#if (CONTROLLER_TARGET == BMS_EP_A5 || BMS_EP_20_REV1_1_2TEMP || CONTROLLER_TARGET == BMS_EP_200_A1 || CONTROLLER_TARGET == BMS_EP_200_A2 || CONTROLLER_TARGET == BMS_XINXIN_A2 || CONTROLLER_TARGET == BMS_EP_200_B1)				// EP BMS A3版本，带串口调试功能

	// 串口MODBUS通讯模块初始化
	// Take care: 区块0-3读写，区块4以后仅允许读取
/*	g_USART_MODBUS.Parameter.DataBlock[0].pBuffer = (u16*)&g_SystemDebugRef;		// 区块0，读写，控制量
	g_USART_MODBUS.Parameter.DataBlock[0].BlockLength = 100;	
	g_USART_MODBUS.Parameter.DataBlock[0].BlockStartAddress = 0x1000;
*/	
   
    
     
	g_USART_MODBUS.Parameter.DataBlock[1].pBuffer = (u16*)&g_SystemParameter;	// 区块1，读写，设备信息及参数
	g_USART_MODBUS.Parameter.DataBlock[1].BlockLength = 300;	
	g_USART_MODBUS.Parameter.DataBlock[1].BlockStartAddress = 0x2000;

	g_USART_MODBUS.Parameter.DataBlock[4].pBuffer = (u16*)&g_SystemMonitor;		// 区块4，读取，运行状态，常规数据,
	g_USART_MODBUS.Parameter.DataBlock[4].BlockLength = 300;	
	g_USART_MODBUS.Parameter.DataBlock[4].BlockStartAddress = 0x3000;
	
/*	
    g_USART_MODBUS.Parameter.DataBlock[5].pBuffer = (u16*)&g_SystemDebugFdb;	// 区块5，读取，电机调试用，快速运行状态
	g_USART_MODBUS.Parameter.DataBlock[5].BlockLength = 300;	
	g_USART_MODBUS.Parameter.DataBlock[5].BlockStartAddress = 0x4000;
*/
	g_USART_MODBUS.Parameter.DataBlock[6].pBuffer = (u16*)&g_SystemRecord;		// 区块6，读取，运行记录，行车记录
	g_USART_MODBUS.Parameter.DataBlock[6].BlockLength = RECORD_CIRCLE_LENGTH;	
	g_USART_MODBUS.Parameter.DataBlock[6].BlockStartAddress = 0x5000;
	
	g_USART_MODBUS.Parameter.DataBlock[7].pBuffer = (u16*)&g_SystemFaultHistory.FaultHistory;		// 区块7，读取，故障历史记录
	g_USART_MODBUS.Parameter.DataBlock[7].BlockLength = 100;	
	g_USART_MODBUS.Parameter.DataBlock[7].BlockStartAddress = 0x6000;

	g_USART_MODBUS.Parameter.DataBlock[8].pBuffer = (u16*)&g_SystemFaultHistory.FaultCount;			// 区块8，读取，故障次数记录
	g_USART_MODBUS.Parameter.DataBlock[8].BlockLength = 100;	
	g_USART_MODBUS.Parameter.DataBlock[8].BlockStartAddress = 0x7000;
	
	
	USART_MODBUS_DeviceInit(&g_USART_MODBUS);

//#endif				


	// --------------------------------- LED闪烁模块初始化 -------------------------------------
	// 配置闪烁时间及间隔
	g_LED_Flicker.LEDONLength = 500;
	g_LED_Flicker.LEDOFFLength = 500;
	g_LED_Flicker.LEDWaitLength1 = 2000;
	g_LED_Flicker.LEDWaitLength2 = 3000;
	
    
	//AFE唤醒

	// 初始化全部完成后再配置，由于状态机配置，还是可能发生未初始化完成即触发中断状况
	SysTick_Config((u32)SYSTEM_FREQUENCY * 1000);     			// Systick中断，时基1ms	
	//SysTick_Config((u32)24 * 1000); 
    //RCC_GetClocksFreq(&clk); //测试中主频48MHZ======================================================

	// --------------------- 配置IWDG独立硬件看门狗 --------------------------
	// IWDG硬件独立看门狗使用LSI时钟，主频40kHz，波动较大。
	/*启动内部低速时钟,等待时钟就绪*/
	
   

#if EN_IWDG ==1
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	
	IWDG_SetPrescaler(IWDG_Prescaler_32);						// 32倍分频
	
	// LSI频率约40kHz左右,32分频，设计超时为1s
	IWDG_SetReload(1280);

	IWDG_ReloadCounter();										// 重置看门狗计数器
	
	g_SystemState.State.bit.IWDGReloadEnable = 1;				// 使能喂狗
	
	IWDG_Enable();												// 看门狗使能开关   
    
	g_SystemState.State.bit.IWDGReloadEnable = 1;				// 使能喂狗			
#else
#warning "Did not open"//未打开看门狗
#endif

   
//test
   GPIO_SetBits(GPIO_EN_AFE_PORT, GPIO_EN_AFE_PIN);//唤醒AFE
    
	
}


// End of bsp.c

