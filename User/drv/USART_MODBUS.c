/* ==================================================================================

 File name:     USART_MODBUS.c
 Originator:    BLJ
 Description:   串口通信模块，MODBUS从机模式，兼容RS485，
				强制工作在半双工下（即发送完成后才切换为接收状态，两者不可同时进行）

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 04-20-2016		Version 1.0.0			增加写入数据完成标志位更新，正式版本
 12-18-2015		Version 0.2.0			更新版本，完善功能
 03-26-2015     Version 0.1.0           MODBUS，多寄存器读写功能实现，待长期测试及改进，										
										可使用串口模块1，3，暂时未添加2
 01-17-2015     Version 0.0.1           测试功能
-----------------------------------------------------------------------------------*/


#include "target.h"					// 目标板选择配置头文件
//#include "USART_MODBUS.h"			// 串口烧录模块头文件
#include "CRC16.h"					// CRC16校验模块   
#include "system_core.h"

s32	t_MODBUS[10];
u8	t_MODBUS2[10];
u8	t_MODBUS3[10];

// 不再支持旧版无串口硬件
//#if (CONTROLLER_TARGET == BMS_EP_A5 || BMS_EP_20_REV1_1_2TEMP || CONTROLLER_TARGET == BMS_EP_200_A1 || CONTROLLER_TARGET == BMS_EP_200_A2 || CONTROLLER_TARGET == BMS_XINXIN_A2 || CONTROLLER_TARGET == BMS_EP_200_B1)				// EP BMS A3版本，带串口调试功能

// --------------------------- 串口模块相关IO口配置函数 -------------------------------
void USART_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;          				// GPIO配置寄存器结构体	

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    //UART 初始化设置
	
	GPIO_InitStructure.GPIO_Pin = USART_MODBUS_TX_PIN;			// 输出IO口
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;				// 强推挽复用
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(USART_MODBUS_TX_PORT, &GPIO_InitStructure);	

	// Take care: 复用IO口选择必须正确，取值根据请看STM32F072数据手册
	GPIO_PinAFConfig(USART_MODBUS_TX_PORT, USART_MODBUS_TX_AF_PIN, USART_MODBUS_TX_AF);	
		
	GPIO_InitStructure.GPIO_Pin = USART_MODBUS_RX_PIN;			// 输入IO口
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;				// 强推挽复用
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(USART_MODBUS_RX_PORT, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(USART_MODBUS_RX_PORT, USART_MODBUS_RX_AF_PIN, USART_MODBUS_RX_AF);	
}

// ------------------------------ 串口模块初始化函数 ----------------------------------
void USART_MODBUS_DeviceInit(USART_MODBUS_structDef* v)
{
	USART_InitTypeDef USART_InitStructure; 			// 声明USART初始化结构体
	
	USART_GPIO_Config();							// 配置IO口功能

	
// ----------------------- 根据串口模块选择，使能相应模块时钟 ----------------------------------
#if (USART_MODBUS_SELECT == 1)						// 配置USART1
		
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);				// 串口1模块时钟使能
	
#elif (USART_MODBUS_SELECT == 2)

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);				// 串口2模块时钟使能
	
#elif (USART_MODBUS_SELECT == 3)	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);				// 串口3模块时钟使能
	
#endif
	
	
// ------------- 根据不同串口配置模块，默认19200bps，8数据位，1停止位，无校验 ------------------------	
	
	USART_DeInit(USART_MODBUS);											// 串口模块配置初始化
	
	USART_InitStructure.USART_BaudRate = USART_MODBUS_BAUDRATE;			// 波特率设置，直接输入需要波特率，函数内部自动计算
  	USART_InitStructure.USART_WordLength = USART_WordLength_8b;			// 8数据位
  	USART_InitStructure.USART_StopBits = USART_StopBits_1;				// 1停止位
  	USART_InitStructure.USART_Parity = USART_Parity_No;					// 无校验
  	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		// 无硬件流控制
  	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;		// 使能接收，发送功能
  	USART_Init(USART_MODBUS, &USART_InitStructure);

  	USART_Cmd(USART_MODBUS, ENABLE);									// 使能串口模块

	USART_DMACmd(USART_MODBUS, USART_DMAReq_Rx, ENABLE);				// 使能DMA，串口接收
	USART_DMACmd(USART_MODBUS, USART_DMAReq_Tx, ENABLE); 				// 使能DMA，串口发送  
}


// ------------------------- 串口时基函数，运行在1ms时基中 ----------------------------
void USART_MODBUS_TimeTick(USART_MODBUS_structDef* v)
{
	// 超时计数器最大10000，10s
	v->Variable.TimeOutCount++;
	if (v->Variable.TimeOutCount > 10000)
	{
		v->Variable.TimeOutCount = 10000;
	}
}

u8 Check_step =0; //此变量为校验boot升级 指令 3A 00 00 0010 F0
u8 modbus_delay_nop;
// ----------------- 串口模块过程函数，需保证被执行频率，一般运行在主循环中 --------------------
void USART_MODBUS_Process(USART_MODBUS_structDef* v)
{	
	// 函数中需配置DMA，所以在函数起始处定义DMA参数结构体
	DMA_InitTypeDef	DMA_InitStructure;       		// DMA初始化结构体定义
	
	// 函数内部变量，强制定义为局部静态变量
	static u16	s_dataNumber = 0;
	u8 LRCValue = 0;
	
	// MODBUS从机处理逻辑，处理数据接收，功能响应，并回复，错误帧回复
	// 共分为以下几个状态：
	// 0：MODBUS_IDLE，MODBUS空闲状态，清除超时定时器，配置DMA接收并使能，跳转至MODBUS_CHECK_ID
	// 8：MODBUS_CHECK_ID，等待接收ID，清除超时定时器，若接收到数据，ID匹配则进入MODBUS_CHECK_FUNCTION_LENGTH，
	// 不匹配进入MODBUS_NOTMINE，不做任何处理，等待超时并返回MODBUS_IDLE
	// 1：MODBUS_CHECK_FUNCTION_LENGTH，当有新数据被接收时清除超时定时器，接收数据大于等于8个字节时，检查功能码及数据总长度
	// 2：MODBUS_WAITRECEIVE，当有新数据被接收时清除超时定时器，等待数据接收完成，执行CRC校验，根据功能码执行相应操作
	// 3：MODBUS_PROCESSREAD，多寄存器读取处理，清除超时定时器，准备回复帧并启动DMA发送
	// 4：MODBUS_PROCESSWRITE，多寄存器写入处理，清除超时定时器准备回复帧并启动DMA发送
	// 5：MODBUS_RESPONSE，回复帧由DMA自动发送，发送完成后进入MODBUS_IDLE状态
	// 6：MODBUS_ERROR，当有新数据被接收时清除超时定时器，超时2ms以上时，启动错误回复帧发送，即主机数据发送完成后再发送错误帧，强制总线工作在半双工模式
	// 7：MODBUS_NOTMINE，ID不匹配，主机数据无本从机无关，当有新数据被接收时清除超时定时器。超时后自动进入MODBUS_IDLE
	switch (v->State.State_bits.bit.MODBUSState)
	{
		case MODBUS_IDLE:				// 空闲状态，清除超时定时器，配置DMA接收并使能，跳转至MODBUS_CHECK_ID
		{
			// 清除超时定时器
			v->Variable.TimeOutCount = 0;
			
			// 空闲状态，无数据接收发送，接收数据临时变量清除
			v->Variable.ReceiveByteLength = 0;
			v->Variable.ReceiveByteNumber = 0;
			v->Variable.ReceiveByteNumberOld = 0;

			// 串口接收数据通过DMA传输至ram指定区域
			// Take care： 使用正常模式

			DMA_DeInit(USART_MODBUS_DMA_RX);		  										// 清除DMA对应通道配置，必须
			DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART_MODBUS->RDR;		  		// DMA对应的外设基地址  接收数据寄存器
			DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&v->ReceiveByteBuffer;   			// 内存存储基地址
			DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;								// DMA的转换模式为SRC模式，由外设搬移到内存
			DMA_InitStructure.DMA_BufferSize = BYTE_BUFFER_LENGTH;		   					// DMA缓存大小，默认300个
			DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;				// 接收一次数据后，设备地址不后移
			DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;							// 接收一次数据后，目标内存地址后移
			DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;			// 定义外设数据宽度为8位
			DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  				// 目标存储区RAM数据宽度为8位
			DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;   								// 转换模式，正常模式。
			DMA_InitStructure.DMA_Priority = USART_MODBUS_DMA_RX_PRIORITY;					// DMA优先级
			DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;		  							// M2M模式禁用
            //DMA_InitStructure.DMA_Auto_reload = DMA_Auto_Reload_Disable;
			DMA_Init(USART_MODBUS_DMA_RX, &DMA_InitStructure);   
			
			// 使能DMA对应通道，开始接收数据
			DMA_Cmd(USART_MODBUS_DMA_RX, ENABLE);	

			v->State.State_bits.bit.MODBUSState = MODBUS_CHECK_ID;			// 跳转至下一状态，等待数据接收，检查ID号
		}
		break;
		
		case MODBUS_CHECK_ID:			// 等待接收ID，无数据通信工况下，停留在该状态
		{
			// 等待ID接收状态，无超时功能，清除超时定时器 
			v->Variable.TimeOutCount = 0;
			
			// 根据DMA寄存器计算已接收字节
			v->Variable.ReceiveByteNumber = BYTE_BUFFER_LENGTH - DMA_GetCurrDataCounter(USART_MODBUS_DMA_RX);
			
			// 接收到第一个字节时判断ID号是否与本机匹配
			if (v->Variable.ReceiveByteNumber > 0)
			{
				if (v->ReceiveByteBuffer[MODBUS_ID] == MODBUS_ID_UMBER)		// ID号匹配  或串口通讯协议帧头第一段
				{
					v->State.State_bits.bit.MODBUSState = MODBUS_CHECK_FUNCTION_LENGTH;		// 跳转至下一状态，检查功能码及长度
				}
                else if(v->ReceiveByteBuffer[MODBUS_ID] == MODBUS_COMMUNICAT)
                {
                    v->State.State_bits.bit.MODBUSState = COMM_RECIVE_FUNCTION;          // 跳转至通讯指令接受状态，检查功能码
                }
				//==============================================================================
				else if(v->ReceiveByteBuffer[MODBUS_ID] == 0x3A)           //串口上位机帧头
                {
					Check_step=1;
                    v->State.State_bits.bit.MODBUSState = COMM_RECIVE_FUNCTION;          
                }
				//===============================================================================
                else
                {
				   Check_step=0;
                   v->State.State_bits.bit.MODBUSState = MODBUS_NOTMINE;		// 头码不匹配，跳转至不匹配等待状态
                }
			}
			
			// for test
			// 串口发送数据通过DMA传输
//			DMA_DeInit(USART_MODBUS_DMA_TX);		  										// DMA对应通道初始化，必须
//			DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART_MODBUS->TDR;		  		// DMA对应的外设基地址
//			DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&v->SendByteBuffer;   				// 内存存储基地址
//			DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;								// DMA的转换模式为DST模式，由内存搬移到外设
//			DMA_InitStructure.DMA_BufferSize = 8;		   			// 发送数据帧长度
//			DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;				// 接收一次数据后，设备地址不后移
//			DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;							// 接收一次数据后，内存地址后移
//			DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;			// 定义外设数据宽度为8位
//			DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  				// 目标存储区RAM数据宽度为8位
//			DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;   								// 转换模式，正常模式
//			DMA_InitStructure.DMA_Priority = DMA_Priority_Low;								// DMA优先级低
//			DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;		  							// M2M模式禁用
//			DMA_Init(USART_MODBUS_DMA_TX, &DMA_InitStructure);   
			
			// 使能DMA通道
			//DMA_Cmd(USART_MODBUS_DMA_TX, ENABLE);	
			
			//USART_SendData(USART_MODBUS, 0x5A);
			//v->SendByteBuffer[0]=0x5A;
			//v->State.State_bits.bit.MODBUSState = 8;
		}
		break;
        // 此状态机用于处理与串口之间的通讯
        case COMM_RECIVE_FUNCTION:  //处理与仪表之间的串口通讯指令
        {
			// 根据DMA寄存器计算已接收字节
			v->Variable.ReceiveByteNumber = BYTE_BUFFER_LENGTH - DMA_GetCurrDataCounter(USART_MODBUS_DMA_RX);
			
			// 判断是否接收到新数据，是的话清除超时定时器
			if (v->Variable.ReceiveByteNumber != v->Variable.ReceiveByteNumberOld)
			{
				// 清除超时定时器
				v->Variable.TimeOutCount = 0;
				
				// 赋值接收字节计数器
				v->Variable.ReceiveByteNumberOld = v->Variable.ReceiveByteNumber;
			}
	
//=====================================上位机升级指令解析======================================================================			
			if ((v->Variable.ReceiveByteNumber == 6)&&(Check_step==1)) //串口升级命令数据帧 3A 00 00 00 10 F0
			{
			  if(v->ReceiveByteBuffer[MODBUS_FUNCTION]==0x00)        //串口升级功能码
			  { Check_step=2;		
				if(v->ReceiveByteBuffer[MODBUS_ADDRESSHI] == 0x00)  //帧头第二段0x50
                {
                  if(v->ReceiveByteBuffer[MODBUS_ADDRESSLOW] == 0x00) // 类型码0x01主控设备发送指令
                   {
					if(v->ReceiveByteBuffer[MODBUS_NUMBERHI] == 0x10)    //帧头第二段0x50
                     {
					  if(v->ReceiveByteBuffer[MODBUS_NUMBERLOW] == 0xF0)    // 类型码0x01主控设备发送指令
					    {
						   Check_step=0;
						  // g_SystemState.State.bit.EnterBootloaderAsk =1;
						   QACTIVE_POST((QActive *)&g_AO_BMS, ENTER_BOOTLOADER_SIG, 0);
					      }
			          }
				   }
				}
			}
		}
			
		Check_step=0;
//===========================================================================================================	
			
			
			// 当读取多于等于6byte时，可以检查MODBUS功能码及计算数据总长度
			if (v->Variable.ReceiveByteNumber >= 6)
			{
                if(v->ReceiveByteBuffer[MODBUS_FUNCTION] == COMM_FUNCTION_HEAD_INFOR)  //帧头第二段0x50
                {
                    if(v->ReceiveByteBuffer[MODBUS_ADDRESSHI] == 0x01)    // 类型码0x01主控设备发送指令
                    {
                        switch(v->ReceiveByteBuffer[MODBUS_ADDRESSLOW])
                        {                        
                            case COMM_FUNCTION_BASIC_INFOR:   //返回仪表读取基本信息
                            {
                                // 无错误，读取数据，准备回复帧                                
                                // 准备回复字节数
                                v->SendByteBuffer[0] = v->ReceiveByteBuffer[0];                 // 回复帧ID号，即通讯帧头第一位
                                v->SendByteBuffer[1] = v->ReceiveByteBuffer[1];     // 回复帧功能码，即通讯帧头第二位  
                                v->SendByteBuffer[2] = 0x02;                        //BMS相应仪表
                                v->SendByteBuffer[3] = 0x56;  //响应仪表指令0x16
                                v->SendByteBuffer[4] = 2;     //数据长度
                                v->SendByteBuffer[5] = (g_AO_BMS.Output.SOC / 10) & 0xFF;
                                v->SendByteBuffer[6] = Protect_GetFaultCode(&g_Protect);
                                for(s_dataNumber = 0; s_dataNumber < 7; s_dataNumber++)
                                {
                                    LRCValue += v->SendByteBuffer[s_dataNumber];
                                }
                                v->SendByteBuffer[7] = ~LRCValue;
                                v->Variable.SendByteLength = 8;
                            }
                            break;
                            case COMM_FUNCTION_MORE_INFOR:  //返回仪表读取的其余基本信息
                            {
                                //准备回复字节
                                v->SendByteBuffer[0] = v->ReceiveByteBuffer[0];     // 回复帧ID号，即通讯帧头第一位
                                v->SendByteBuffer[1] = v->ReceiveByteBuffer[1];     // 回复帧功能码，即通讯帧头第二位  
                                v->SendByteBuffer[2] = 0x02;                        //BMS相应仪表
                                v->SendByteBuffer[3] = 0x61;                        //响应仪表读取的其余基本信息
                                v->SendByteBuffer[4] = 19;                          //数据长度
                                v->SendByteBuffer[5] = ((g_AO_SH36730x0.Output.BatteryVoltage / 100) >> 8) & 0xFF;
                        		v->SendByteBuffer[6] = (g_AO_SH36730x0.Output.BatteryVoltage / 100) & 0xFF;
                        		
                        		// 传输电流值单位转换为0.1A，可传输+-3200.0A
                        		v->SendByteBuffer[7] = ((g_AO_SH36730x0.Output.BatteryCurrent / 100) >> 8) & 0xFF;
                        		v->SendByteBuffer[8] = (g_AO_SH36730x0.Output.BatteryCurrent / 100) & 0xFF;
                        		
                        		// 传输电量单位为0.1AH
                        		v->SendByteBuffer[9] = ((g_AO_BMS.Output.BatteryCapacity / 100) >> 8) & 0xFF;
                        		v->SendByteBuffer[10] = (g_AO_BMS.Output.BatteryCapacity / 100) & 0xFF;
                        		
                        		// 传输SOC单位为1%
                        		v->SendByteBuffer[11] = (g_AO_BMS.Output.SOC / 10) & 0xFF;	
                                g_communication.BMSData1.BatteryState.all = 0;				// 首先清除旧的电池状态
                                // 准备电池状态数据
                            	switch (Protect_GetFaultCode(&g_Protect))
                        		{
                        			case FAULT_BQ769_SCD:					// 电池短路
                        			{
                        				g_communication.BMSData1.BatteryState.bit.ShortCurrent = 1;
                        			}
                        			break;
                        			
                        			case FAULT_BQ769_OCD:
                        			{
                        				g_communication.BMSData1.BatteryState.bit.OverCurrent = 1;
                        			}
                        			break;
                        			
                        			case FAULT_BQ769_UV:
                        			{
                        				g_communication.BMSData1.BatteryState.bit.UnderVoltage = 1;
                        			}
                        			break;
                        			
                        			case FAULT_BQ769_OV:
                        			{
                        				g_communication.BMSData1.BatteryState.bit.OverVoltage = 1;
                        			}
                        			break;
                        			
                        			case FAULT_START_FAIL:
                        			case FAULT_BQ769_FAIL:
                        			case FAULT_BQ769_OVRD_ALERT:
                        			case FAULT_BQ769_I2C_OVERTIME:
                        			{
                        				g_communication.BMSData1.BatteryState.bit.BMSFail = 1;
                        			}
                        			break;
                            	}
                                v->SendByteBuffer[12] = g_communication.BMSData1.BatteryState.all;
                                v->SendByteBuffer[13] = ((g_AO_SH36730x0.Output.SingleMaxVoltage >> 8) & 0x00FF);
                        		v->SendByteBuffer[14] = (g_AO_SH36730x0.Output.SingleMaxVoltage & 0x00FF);
                        		v->SendByteBuffer[15] = g_AO_SH36730x0.Output.SingleMaxVoltagePointer;
                        		
                        		v->SendByteBuffer[16] = ((g_AO_SH36730x0.Output.SingleMinVoltage >> 8) & 0x00FF);
                        		v->SendByteBuffer[17] = (g_AO_SH36730x0.Output.SingleMinVoltage & 0x00FF);
                        		v->SendByteBuffer[18] = g_AO_SH36730x0.Output.SingleMinVoltagePointer;
                        		
                        		v->SendByteBuffer[19] = g_AO_BMS.Output.BatteryTemperatureHi / 10;
                        		v->SendByteBuffer[20] = g_AO_BMS.Output.BatteryTemperatureLow / 10;
                                if (Protect_GetFaultCode(&g_Protect) > 0)
                                {
                                   v->SendByteBuffer[21] = Protect_GetFaultCode(&g_Protect); 
                                }
                                else 
                                {
                                   v->SendByteBuffer[21] = Protect_GetWarningCode(&g_Protect);
                                } 
                                v->SendByteBuffer[22] = (CONST_SOFTWARE_VERSION >> 8) & 0xFF;
                                v->SendByteBuffer[23] = CONST_SOFTWARE_VERSION & 0xFF;
                                for(s_dataNumber = 0; s_dataNumber < 24; s_dataNumber++)
                                {
                                    LRCValue += v->SendByteBuffer[s_dataNumber];
                                }
                                v->SendByteBuffer[24] = ~LRCValue;
                                v->Variable.SendByteLength = 25;
                            }
                            break;
                            default:					// 其它功能码，无法识别，回复错误帧
        					{					
        						v->Variable.MODBUSFaultInf = MODBUS_FAULT_WRONGFUNCTION;			// MODBUS错误：功能码错误，不支持
        						v->State.State_bits.bit.MODBUSState = MODBUS_ERROR;					// MODBUS数据有错误
        					}
        					break;
                        }
                        
    					// 清除超时定时器
    					v->Variable.TimeOutCount = 0;
    					

    					// 串口发送数据通过DMA传输
    					DMA_DeInit(USART_MODBUS_DMA_TX);		  										// DMA对应通道初始化，必须
    					DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART_MODBUS->TDR;		  		// DMA对应的外设基地址
    					DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&v->SendByteBuffer;   				// 内存存储基地址
    					DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;								// DMA的转换模式为DST模式，由内存搬移到外设
    					DMA_InitStructure.DMA_BufferSize = v->Variable.SendByteLength;		   			// 发送数据帧长度
    					DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;				// 接收一次数据后，设备地址不后移
    					DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;							// 接收一次数据后，内存地址后移
    					DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;			// 定义外设数据宽度为8位
    					DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  				// 目标存储区RAM数据宽度为8位
    					DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;   								// 转换模式，正常模式
    					DMA_InitStructure.DMA_Priority = USART_MODBUS_DMA_TX_PRIORITY;					// DMA优先级
    					DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;		  							// M2M模式禁用
                        //DMA_InitStructure.DMA_Auto_reload = DMA_Auto_Reload_Disable;
    					DMA_Init(USART_MODBUS_DMA_TX, &DMA_InitStructure);   
    					
    					// 使能DMA通道
    					DMA_Cmd(USART_MODBUS_DMA_TX, ENABLE);	

    					
    					// 跳转至等待回复帧发送完成状态 
    					v->State.State_bits.bit.MODBUSState = MODBUS_RESPONSE;
                    }
                }
                // 跳出for循环
				break;
                    	
			}	
        }
        break;
		
		case MODBUS_CHECK_FUNCTION_LENGTH:		// 检查功能码及数据总长度
		{
			// 根据DMA寄存器计算已接收字节
			v->Variable.ReceiveByteNumber = BYTE_BUFFER_LENGTH - DMA_GetCurrDataCounter(USART_MODBUS_DMA_RX);
			
			// 判断是否接收到新数据，是的话清除超时定时器
			if (v->Variable.ReceiveByteNumber != v->Variable.ReceiveByteNumberOld)
			{
				// 清除超时定时器
				v->Variable.TimeOutCount = 0;
				
				// 赋值接收字节计数器
				v->Variable.ReceiveByteNumberOld = v->Variable.ReceiveByteNumber;
			}
		
			// 当读取多于等于8byte时，可以检查MODBUS功能码及计算数据总长度
			if (v->Variable.ReceiveByteNumber >= 8)
			{
				switch (v->ReceiveByteBuffer[MODBUS_FUNCTION])					// 检测MODBUS功能码
				{
					case MODBUS_FUNCTION_MULTI_READ:							// 读多个寄存器，0x03
					{
						v->Variable.ReceiveByteLength = 8;						// MODBUS 0x03 读多个寄存器指令长度固定为8bytes

						v->State.State_bits.bit.MODBUSState = MODBUS_WAITRECEIVE;	// MODBUS等待数据接收完毕
					}
					break;

					case MODBUS_FUNCTION_MULTI_WRITE:							// 写多个寄存器，0x10
					{
						// 计算指令帧长度						
						// 以下代码实现判断主机指令帧中数据长度是否为字节长度2倍，否则数据帧错误
						v->Variable.ReceiveByteLength = ((v->ReceiveByteBuffer[MODBUS_NUMBERHI] << 8) + v->ReceiveByteBuffer[MODBUS_NUMBERLOW]) * 2;
						
						if (v->Variable.ReceiveByteLength == v->ReceiveByteBuffer[MODBUS_MULTI_WRITE_BYTE_NUMBER])
						{
							v->Variable.ReceiveByteLength = v->ReceiveByteBuffer[MODBUS_MULTI_WRITE_BYTE_NUMBER] + 9;
							
							v->State.State_bits.bit.MODBUSState = MODBUS_WAITRECEIVE;	// MODBUS等待数据接收完毕
						}
						else
						{
							v->Variable.MODBUSFaultInf = MODBUS_FAULT_LENGTH;				// MODBUS错误：多字节写入帧长度出错，写入数据数量与字节数量不是2倍关系
							v->State.State_bits.bit.MODBUSState = MODBUS_ERROR;				// MODBUS数据有错误									
						}
					}
					break;
                    
					default:					// 其它功能码，无法识别，回复错误帧
					{					
						v->Variable.MODBUSFaultInf = MODBUS_FAULT_WRONGFUNCTION;			// MODBUS错误：功能码错误，不支持
						v->State.State_bits.bit.MODBUSState = MODBUS_ERROR;					// MODBUS数据有错误
					}
					break;
				}	

				// 跳出for循环
				break;
			}		
		}
		break;
		
		case MODBUS_WAITRECEIVE:							// 等待接收全部数据帧，若是多寄存器读指令，此时接收字节长度与总长度一致，已完整接收
		{
			
			// 根据DMA寄存器计算已接收字节
			v->Variable.ReceiveByteNumber = BYTE_BUFFER_LENGTH - DMA_GetCurrDataCounter(USART_MODBUS_DMA_RX);
			
			// 判断是否接收到新数据，是的话清除超时定时器
			if (v->Variable.ReceiveByteNumber != v->Variable.ReceiveByteNumberOld)
			{
				// 清除超时定时器
				v->Variable.TimeOutCount = 0;
				
				// 赋值接收字节计数器
				v->Variable.ReceiveByteNumberOld = v->Variable.ReceiveByteNumber;
			}
			
			// 已接收字节数量大于等于接收字节长度，即数据帧已被完整接收
			if (v->Variable.ReceiveByteNumber >= v->Variable.ReceiveByteLength)
			{			
				// 全部数据已被接收，校验CRC

				// 提取数据帧中的CRC校验值，低8位在前，高8位在后
				v->Variable.MODBUSCRCValue = (v->ReceiveByteBuffer[v->Variable.ReceiveByteLength - 1] << 8) + v->ReceiveByteBuffer[v->Variable.ReceiveByteLength - 2];

				// 计算数据帧CRC值，并作比对
				if (GetCRC16(&v->ReceiveByteBuffer[0], v->Variable.ReceiveByteLength - 2) == v->Variable.MODBUSCRCValue)
				{
					t_MODBUS3[0] = v->ReceiveByteBuffer[0];
					t_MODBUS3[1] = v->ReceiveByteBuffer[1];
					t_MODBUS3[2] = v->ReceiveByteBuffer[2];
					t_MODBUS3[3] = v->ReceiveByteBuffer[3];
					t_MODBUS3[4] = v->ReceiveByteBuffer[4];
					t_MODBUS3[5] = v->ReceiveByteBuffer[5];
					t_MODBUS3[6] = v->ReceiveByteBuffer[6];
					t_MODBUS3[7] = v->ReceiveByteBuffer[7];
					t_MODBUS3[8] = v->ReceiveByteBuffer[8];
					t_MODBUS3[9] = v->ReceiveByteBuffer[9];					
					
					// CRC校验通过，数据指令帧完整无错误，根据不同功能码进行相应处理					
					switch (v->ReceiveByteBuffer[MODBUS_FUNCTION])						// 检测MODBUS功能码
					{
						case MODBUS_FUNCTION_MULTI_READ:						// 读多个寄存器
						{
							v->State.State_bits.bit.MODBUSState = MODBUS_PROCESSREAD;		// 跳转到多寄存器读处理状态
						}
						break;

						case MODBUS_FUNCTION_MULTI_WRITE:						// 写多个寄存器
						{
							v->State.State_bits.bit.MODBUSState = MODBUS_PROCESSWRITE;		// 跳转到多寄存器读处理状态
						}
						break;

						default:												// 其它功能码，冗余代码，此处不应该被执行
						{
							v->Variable.MODBUSFaultInf = MODBUS_FAULT_WRONGFUNCTION;		// MODBUS错误：功能码错误，不支持
							v->State.State_bits.bit.MODBUSState = MODBUS_ERROR;				// MODBUS数据有错误
						}
						break;
					}
				}
				else						// CRC校验失败
				{
					v->Variable.MODBUSFaultInf = MODBUS_FAULT_CRCFAILURE;		// MODBUS错误：CRC校验值错误
					v->State.State_bits.bit.MODBUSState = MODBUS_ERROR;			// MODBUS数据有错误
				}							
			}
		}
		break;
		
		case MODBUS_PROCESSREAD:				// 处理多寄存器连续读指令   //仪表读取时数据长度为最大长度读取
		{
			// 提取数据地址，长度
			v->Variable.MODBUSDataLength = (v->ReceiveByteBuffer[MODBUS_NUMBERHI] << 8) + v->ReceiveByteBuffer[MODBUS_NUMBERLOW];
			v->Variable.MODBUSDataAddress = (v->ReceiveByteBuffer[MODBUS_ADDRESSHI] << 8) + v->ReceiveByteBuffer[MODBUS_ADDRESSLOW];

			// 数据长度在MODBUS标准范围内,且起始地址不为0
			if (v->Variable.MODBUSDataLength >= 1 && v->Variable.MODBUSDataLength <= MODBUS_DATA_LENGTH_LIMIT)
			{
				// 遍历所有数据区块，若需读写数据均在区块数据范围内，则认定该区块为被读写区块。若无区块匹配，报数据地址错误
				for (v->Variable.DataBlock = 0; v->Variable.DataBlock <= MODBUS_DATA_BLOCK_MAX; v->Variable.DataBlock++)
				{
					// Take care: for循环多进行一次，判断条件带等于号，当最后一次时，不进行地址匹配，直接跳出
					// 地址 == MODBUS_DATA_BLOCK_MAX，作为地址未匹配信号, MODBUS_DATA_BLOCK_MAX = 16
					if (v->Variable.DataBlock < MODBUS_DATA_BLOCK_MAX)
					{
						// 匹配数据起始地址与结束地址，均在区块范围内，则对应区块已找到，跳出
						// 需读写数据区域起始地址大于等于区块数据起始地址，并且，需读写区域结束地址小于等于区块数据结束地址
						if (v->Variable.MODBUSDataAddress >= v->Parameter.DataBlock[v->Variable.DataBlock].BlockStartAddress
							&& ((v->Variable.MODBUSDataAddress + v->Variable.MODBUSDataLength) <= 
							(v->Parameter.DataBlock[v->Variable.DataBlock].BlockStartAddress + v->Parameter.DataBlock[v->Variable.DataBlock].BlockLength)))
						{
							break;			// 跳出for循环
						}
					}
				}

				if (v->Variable.DataBlock < MODBUS_DATA_BLOCK_MAX)			// 地址匹配成功
				{
					// 无错误，读取数据，准备回复帧
					v->SendByteBuffer[MODBUS_ID] = v->ReceiveByteBuffer[MODBUS_ID];					// 回复帧ID号，即读指令ID号
					v->SendByteBuffer[MODBUS_FUNCTION] = v->ReceiveByteBuffer[MODBUS_FUNCTION];		// 回复帧功能码，即读指令功能码			
					
					// 准备回复字节数
					v->SendByteBuffer[MODBUS_MULTI_READ_BYTE_NUMBER] = v->Variable.MODBUSDataLength * 2;

					// 准备读取的数据
					// 高8位在前。ram地址由区块首地址 + （MODBUS数据地址 - 区块数据地址） + 偏置量
					// 例：RAM地址为0x80000000，MODBUS地址为0x1000，区块地址0x1000，偏置量0-125
					for (s_dataNumber = 0; s_dataNumber < v->Variable.MODBUSDataLength; s_dataNumber++)
					{
						v->SendByteBuffer[MODBUS_MULTI_READ_BYTE_NUMBER + s_dataNumber * 2 + 1] = 
						(*(v->Parameter.DataBlock[v->Variable.DataBlock].pBuffer + v->Variable.MODBUSDataAddress - v->Parameter.DataBlock[v->Variable.DataBlock].BlockStartAddress + s_dataNumber) >> 8);
						
						v->SendByteBuffer[MODBUS_MULTI_READ_BYTE_NUMBER + s_dataNumber * 2 + 2] = 
						(*(v->Parameter.DataBlock[v->Variable.DataBlock].pBuffer + v->Variable.MODBUSDataAddress - v->Parameter.DataBlock[v->Variable.DataBlock].BlockStartAddress + s_dataNumber) & 0x00FF);
					}

					// 准备CRC校验值
					v->Variable.MODBUSCRCValue = GetCRC16(&v->SendByteBuffer[0], v->SendByteBuffer[MODBUS_MULTI_READ_BYTE_NUMBER] + 3);
					v->SendByteBuffer[v->SendByteBuffer[MODBUS_MULTI_READ_BYTE_NUMBER] + 3] = v->Variable.MODBUSCRCValue & 0x00FF;
					v->SendByteBuffer[v->SendByteBuffer[MODBUS_MULTI_READ_BYTE_NUMBER] + 4] = v->Variable.MODBUSCRCValue >> 8;

					// 数据准备完毕，计算发送数据帧长度
					v->Variable.SendByteLength = v->SendByteBuffer[MODBUS_MULTI_READ_BYTE_NUMBER] + 5;

					// 清除超时定时器
					v->Variable.TimeOutCount = 0;
					

					// 串口发送数据通过DMA传输
					DMA_DeInit(USART_MODBUS_DMA_TX);		  										// DMA对应通道初始化，必须
					DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART_MODBUS->TDR;		  		// DMA对应的外设基地址
					DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&v->SendByteBuffer;   				// 内存存储基地址
					DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;								// DMA的转换模式为DST模式，由内存搬移到外设
					DMA_InitStructure.DMA_BufferSize = v->Variable.SendByteLength;		   			// 发送数据帧长度
					DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;				// 接收一次数据后，设备地址不后移
					DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;							// 接收一次数据后，内存地址后移
					DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;			// 定义外设数据宽度为8位
					DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  				// 目标存储区RAM数据宽度为8位
					DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;   								// 转换模式，正常模式
					DMA_InitStructure.DMA_Priority = USART_MODBUS_DMA_TX_PRIORITY;					// DMA优先级
					DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;		  							// M2M模式禁用
                  //  DMA_InitStructure.DMA_Auto_reload = DMA_Auto_Reload_Disable;
					DMA_Init(USART_MODBUS_DMA_TX, &DMA_InitStructure);   
					
					// 使能DMA通道
					DMA_Cmd(USART_MODBUS_DMA_TX, ENABLE);	

					
					// 跳转至等待回复帧发送完成状态 
					v->State.State_bits.bit.MODBUSState = MODBUS_RESPONSE;
				}
				else				//  地址匹配失败
				{
					v->Variable.MODBUSFaultInf = MODBUS_FAULT_WRONGADDRESS;					// MODBUS错误：数据地址错误
					v->State.State_bits.bit.MODBUSState = MODBUS_ERROR;						// MODBUS数据有错误
				}
			}
			else		// 数据长度超出范围,MODBUS协议规定单次读写最大数目为125个
			{
				v->Variable.MODBUSFaultInf = MODBUS_FAULT_DATALENGTH;						// MODBUS错误：数据长度超限
				v->State.State_bits.bit.MODBUSState = MODBUS_ERROR;							// MODBUS数据有错误
			}			
		}
		break;
		
		case MODBUS_PROCESSWRITE:			// CRC校验通过，处理数据，写多个寄存器
		{
			// 提取数据地址，长度
			v->Variable.MODBUSDataLength = (v->ReceiveByteBuffer[MODBUS_NUMBERHI] << 8) + v->ReceiveByteBuffer[MODBUS_NUMBERLOW];
			v->Variable.MODBUSDataAddress = (v->ReceiveByteBuffer[MODBUS_ADDRESSHI] << 8) + v->ReceiveByteBuffer[MODBUS_ADDRESSLOW];
			
			// 数据长度在MODBUS标准范围内,且起始地址不为0
			if (v->Variable.MODBUSDataLength >= 1 && v->Variable.MODBUSDataLength <= MODBUS_DATA_LENGTH_LIMIT)
			{				
				// 遍历所有数据区块，若需写入数据均在区块数据范围内，则认定该区块为被写入区块。若无区块匹配，报数据地址错误
				for (v->Variable.DataBlock = 0; v->Variable.DataBlock <= MODBUS_DATA_WRITEBLOCK_MAX; v->Variable.DataBlock++)
				{
					// Take care: for循环多进行一次，当最后一次时，不进行地址匹配，直接跳出
					// 地址 == MODBUS_DATA_WRITEBLOCK_MAX，作为地址未匹配信号，MODBUS_DATA_WRITEBLOCK_MAX = 4，预留前4个区块用于写入
					if (v->Variable.DataBlock < MODBUS_DATA_WRITEBLOCK_MAX)			
					{
						// 匹配数据起始地址与结束地址，均在区块范围内，则对应区块已找到，跳出
						// 需读写数据区域起始地址大于等于区块数据起始地址，并且，需读写区域结束地址小于等于区块数据结束地址
						if (v->Variable.MODBUSDataAddress >= v->Parameter.DataBlock[v->Variable.DataBlock].BlockStartAddress
							&& ((v->Variable.MODBUSDataAddress + v->Variable.MODBUSDataLength) <= 
							(v->Parameter.DataBlock[v->Variable.DataBlock].BlockStartAddress + v->Parameter.DataBlock[v->Variable.DataBlock].BlockLength)))
						{				
							break;			// 跳出for循环
						}
					}
				}

				if (v->Variable.DataBlock < MODBUS_DATA_WRITEBLOCK_MAX)			// 地址匹配成功
				{
					t_MODBUS[6] = v->Variable.MODBUSDataLength;
					
					// 无错误，写入数据
					// 高8位在前。ram地址由区块首地址 + （MODBUS数据地址 - 区块数据地址） + 偏置量
					// 例：RAM地址为0x80000000，MODBUS地址为0x1000，区块地址0x1000，偏置量0-125					
					for (s_dataNumber = 0; s_dataNumber < v->Variable.MODBUSDataLength; s_dataNumber++)
					{
						t_MODBUS[1] = *(v->Parameter.DataBlock[v->Variable.DataBlock].pBuffer + v->Variable.MODBUSDataAddress - v->Parameter.DataBlock[v->Variable.DataBlock].BlockStartAddress + s_dataNumber);
						
						// 将对应数据从buffer写到ram中
						*(v->Parameter.DataBlock[v->Variable.DataBlock].pBuffer + v->Variable.MODBUSDataAddress - v->Parameter.DataBlock[v->Variable.DataBlock].BlockStartAddress + s_dataNumber) =
								(v->ReceiveByteBuffer[MODBUS_MULTI_WRITE_BYTE_NUMBER + s_dataNumber * 2 + 1] << 8) +
								v->ReceiveByteBuffer[MODBUS_MULTI_WRITE_BYTE_NUMBER + s_dataNumber * 2 + 2];
						
						t_MODBUS[0]++;
						
						t_MODBUS[2] = *(v->Parameter.DataBlock[v->Variable.DataBlock].pBuffer + v->Variable.MODBUSDataAddress - v->Parameter.DataBlock[v->Variable.DataBlock].BlockStartAddress + s_dataNumber);
					
						t_MODBUS[3] = v->ReceiveByteBuffer[MODBUS_MULTI_WRITE_BYTE_NUMBER + s_dataNumber * 2 + 1];
						t_MODBUS[4] = v->ReceiveByteBuffer[MODBUS_MULTI_WRITE_BYTE_NUMBER + s_dataNumber * 2 + 2];
						t_MODBUS[5] = (v->ReceiveByteBuffer[MODBUS_MULTI_WRITE_BYTE_NUMBER + s_dataNumber * 2 + 1] << 8) + v->ReceiveByteBuffer[MODBUS_MULTI_WRITE_BYTE_NUMBER + s_dataNumber * 2 + 2];
					
						t_MODBUS[7] = MODBUS_MULTI_WRITE_BYTE_NUMBER + s_dataNumber * 2 + 1;
						t_MODBUS[8] = MODBUS_MULTI_WRITE_BYTE_NUMBER + s_dataNumber * 2 + 2;
						
						t_MODBUS2[0] = v->ReceiveByteBuffer[0];
						t_MODBUS2[1] = v->ReceiveByteBuffer[1];
						t_MODBUS2[2] = v->ReceiveByteBuffer[2];
						t_MODBUS2[3] = v->ReceiveByteBuffer[3];
						t_MODBUS2[4] = v->ReceiveByteBuffer[4];
						t_MODBUS2[5] = v->ReceiveByteBuffer[5];
						t_MODBUS2[6] = v->ReceiveByteBuffer[6];
						t_MODBUS2[7] = v->ReceiveByteBuffer[7];
						t_MODBUS2[8] = v->ReceiveByteBuffer[8];
						t_MODBUS2[9] = v->ReceiveByteBuffer[9];
					}

					// 准备回复帧
					v->SendByteBuffer[MODBUS_ID] = v->ReceiveByteBuffer[MODBUS_ID];					// ID号即数据指令帧ID号
					v->SendByteBuffer[MODBUS_FUNCTION] = v->ReceiveByteBuffer[MODBUS_FUNCTION];		// 功能码即数据指令帧功能码			
					
					// 回复帧数据地址即数据指令帧数据地址
					v->SendByteBuffer[MODBUS_ADDRESSHI] = v->ReceiveByteBuffer[MODBUS_ADDRESSHI];	
					v->SendByteBuffer[MODBUS_ADDRESSLOW] = v->ReceiveByteBuffer[MODBUS_ADDRESSLOW];

					// 回复帧数据长度即数据指令帧数据长度
					v->SendByteBuffer[MODBUS_NUMBERHI] = v->ReceiveByteBuffer[MODBUS_NUMBERHI];
					v->SendByteBuffer[MODBUS_NUMBERLOW] = v->ReceiveByteBuffer[MODBUS_NUMBERLOW];
					
					// 准备CRC校验值
					v->Variable.MODBUSCRCValue = GetCRC16(&v->SendByteBuffer[0], 6);
					v->SendByteBuffer[6] = v->Variable.MODBUSCRCValue & 0x00FF;			// 写多个寄存器回复帧CRC校验占缓存第6，7字节
					v->SendByteBuffer[7] = v->Variable.MODBUSCRCValue >> 8;

					// 数据准备完毕，计算回复帧长度
					v->Variable.SendByteLength = 8;									// 写多个寄存器回复帧，总长度为8字节

					// 清除超时定时器
					v->Variable.TimeOutCount = 0;
					
					
					// 串口发送数据通过DMA传输
					DMA_DeInit(USART_MODBUS_DMA_TX);		  										// DMA对应通道初始化，必须
					DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART_MODBUS->TDR;		  		// DMA对应的外设基地址
					DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&v->SendByteBuffer;   				// 内存存储基地址
					DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;								// DMA的转换模式为DST模式，由内存搬移到外设
					DMA_InitStructure.DMA_BufferSize = v->Variable.SendByteLength;		   			// 发送数据帧长度
					DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;				// 接收一次数据后，设备地址不后移
					DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;							// 接收一次数据后，内存地址后移
					DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;			// 定义外设数据宽度为8位
					DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  				// 目标存储区RAM数据宽度为8位
					DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;   								// 转换模式，正常模式
					DMA_InitStructure.DMA_Priority = USART_MODBUS_DMA_TX_PRIORITY;					// DMA优先级
					DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;		  							// M2M模式禁用
                    //DMA_InitStructure.DMA_Auto_reload = DMA_Auto_Reload_Disable;
					DMA_Init(USART_MODBUS_DMA_TX, &DMA_InitStructure);   
					
					// 使能DMA通道
					DMA_Cmd(USART_MODBUS_DMA_TX, ENABLE);	
					
					// 相应数据已接收标志位置位，用于告知外部数据已接收完毕，可进行处理
					// Take care: 当系统参数对应接收标志位置位时，延时一段时间执行参数写入EEPROM操作，此功能在外部实现！！！
					v->Output.ReceiveBufferUpdated[v->Variable.DataBlock] = 1;
							
					// 跳转至等待回复帧发送完成状态 
					v->State.State_bits.bit.MODBUSState = MODBUS_RESPONSE;			
				}
				else				//  地址匹配失败
				{
					v->Variable.MODBUSFaultInf = MODBUS_FAULT_WRONGADDRESS;					// MODBUS错误：数据地址错误
					v->State.State_bits.bit.MODBUSState = MODBUS_ERROR;						// MODBUS数据有错误
				}
			}
			else		// 数据长度超出范围
			{
				v->Variable.MODBUSFaultInf = MODBUS_FAULT_DATALENGTH;						// MODBUS错误：数据长度超限
				v->State.State_bits.bit.MODBUSState = MODBUS_ERROR;							// MODBUS数据有错误
			}
		}
		break;
		
		case MODBUS_RESPONSE:				// 等待回复帧发送完成状态 
		{	
			// 超时100ms进入IDLE状态，由超时函数另外处理
			
			// 所有数据发送完成，跳转至IDLE状态。
			// Take care： 若在RS485工作模式下，发送转换为接收需做一定延时。仅理论推测
			if (DMA_GetCurrDataCounter(USART_MODBUS_DMA_TX) == 0)
			{
				v->State.State_bits.bit.MODBUSState = MODBUS_IDLE;				// 发送完成，跳转至等待状态
			}
		}
		break;
		
		// 当有新数据被接收时清除超时定时器，超时2ms以上时，启动错误回复帧发送，即主机数据发送完成后再发送错误帧，强制总线工作在半双工模式
		case MODBUS_ERROR:
		{
			// 计算已接收字节数量
			v->Variable.ReceiveByteNumber = BYTE_BUFFER_LENGTH - DMA_GetCurrDataCounter(USART_MODBUS_DMA_RX);						
			
			// 若新接收到数据，清除超时定时器
			if (v->Variable.ReceiveByteNumber != v->Variable.ReceiveByteNumberOld)
			{
				v->Variable.TimeOutCount = 0;
				
				v->Variable.ReceiveByteNumberOld = v->Variable.ReceiveByteNumber;
			}
			
			// 若发生字节间超时，即无新数据被接收，发送故障回复帧
			if (v->Variable.TimeOutCount > MODBUS_FAULT_RESPONSE_DELAY)		
			{
				// 无错误，读取数据，准备回复帧
				v->SendByteBuffer[MODBUS_ID] = v->ReceiveByteBuffer[MODBUS_ID];							// ID号即数据指令帧ID号
				v->SendByteBuffer[MODBUS_FUNCTION] = v->ReceiveByteBuffer[MODBUS_FUNCTION] + 0x80;		// 功能码为数据指令帧功能码+0x08			
				
				// 准备故障信息
				v->SendByteBuffer[MODBUS_FAULTINF] = v->Variable.MODBUSFaultInf;

				// 准备CRC校验值
				v->Variable.MODBUSCRCValue = GetCRC16(&v->SendByteBuffer[0], 3);
				v->SendByteBuffer[3] = v->Variable.MODBUSCRCValue & 0x00FF;
				v->SendByteBuffer[4] = v->Variable.MODBUSCRCValue >> 8;

				// 数据准备完毕，计算错误回复帧长度
				v->Variable.SendByteLength = 5;

				// 清除超时定时器
				v->Variable.TimeOutCount = 0;
				
				// 串口发送数据通过DMA传输
				DMA_DeInit(USART_MODBUS_DMA_TX);		  										// DMA对应通道初始化，必须
				DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART_MODBUS->TDR;		  		// DMA对应的外设基地址
				DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&v->SendByteBuffer;   				// 内存存储基地址
				DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;								// DMA的转换模式为DST模式，由内存搬移到外设
				DMA_InitStructure.DMA_BufferSize = v->Variable.SendByteLength;		   			// 发送数据帧长度
				DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;				// 接收一次数据后，设备地址不后移
				DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;							// 接收一次数据后，内存地址后移
				DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;			// 定义外设数据宽度为8位
				DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  				// 目标存储区RAM数据宽度为8位
				DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;   								// 转换模式，正常模式
				DMA_InitStructure.DMA_Priority = USART_MODBUS_DMA_TX_PRIORITY;					// DMA优先级
				DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;		  							// M2M模式禁用
               // DMA_InitStructure.DMA_Auto_reload = DMA_Auto_Reload_Disable;
				DMA_Init(USART_MODBUS_DMA_TX, &DMA_InitStructure);   
				
				// 使能DMA通道
				DMA_Cmd(USART_MODBUS_DMA_TX, ENABLE);

				
				// 跳转至等待回复帧发送完成状态 
				v->State.State_bits.bit.MODBUSState = MODBUS_RESPONSE;				
			}
		}
		break;
		
		case MODBUS_NOTMINE:
		{
			// 不做任何处理，静静的等待超时，然后回到Idle状态，重新开始接收新的数据
			// ID号不一致，即主机与其它节点正在通信，超时延时正好避开两者的通信时间
		}
		break;
	}
	// 数据指令帧接收，解码，处理，回复功能代码区结束
	
	// 超时处理，不同状态下处理略有不同
	if (v->State.State_bits.bit.MODBUSState == MODBUS_RESPONSE)
	{
		//处于回复状态时，超时MODBUS_BYTE_TIMEOUT == 1000ms进入IDLE状态，保证回复帧有足够发送时间
		if (v->Variable.TimeOutCount > MODBUS_FRAME_TIMEOUT)
		{
			// 关闭DMA通道，即关闭串口3发送
			DMA_Cmd(USART_MODBUS_DMA_TX, DISABLE);	
			
			// 空闲状态，无数据接收发送，接收数据临时变量清除
			// 接收相关临时变量在IDLE状态会被统一清除
		//	v->Variable.ReceiveByteLength = 0;
		//	v->Variable.ReceiveByteNumber = 0;		
			
			// 清除超时定时器
			v->Variable.TimeOutCount = 0;
						
			// 跳转至空闲状态，重新配置并等待接收下一组数据
			v->State.State_bits.bit.MODBUSState = MODBUS_IDLE;			
		}
	}
	else if (v->State.State_bits.bit.MODBUSState == MODBUS_ERROR)
	{
		// 故障状态有自己的处理逻辑，超时一定时间无数据被接收，即回复故障信息给主机，然后跳转至MODBUS_RESPONSE
	}
	else
	{
		// 当不处于回复状态时,超时10ms，自动进入IDLE状态
		if (v->Variable.TimeOutCount > MODBUS_BYTE_TIMEOUT)
		{						
			// 空闲状态，无数据接收发送，接收数据临时变量清除
			// 接收相关临时变量在IDLE状态会被统一清除
		//	v->Variable.ReceiveByteLength = 0;
		//	v->Variable.ReceiveByteNumber = 0;
			
			// 清除超时定时器
			v->Variable.TimeOutCount = 0;			

			// 跳转至空闲状态，重新配置并等待接收下一组数据
			v->State.State_bits.bit.MODBUSState = MODBUS_IDLE;					
		}	
	}	
}

//#endif					


// End of USART_MODBUS.c

