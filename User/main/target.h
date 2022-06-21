/* ==================================================================================

 File name:		target.h
 Originator:	BLJ
 Description:	DSP目标板选择头文件，用于目标硬件控制板选择

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 10-07-2014		Version 0.0.1			测试功能
-----------------------------------------------------------------------------------*/

// -------------------------------------基本宏定义 -------------------------------------
//#ifndef TRUE					       //宏定义TRUE,FALSE  //和MM32SPIN27PF库定义重复注释*
//#define	TRUE	1
//#define	FALSE	0
//#endif		// TRUE


//二代AFE温度采样测试
// -------------------------------------DSP可选择列表 ------------------------------------------


// ------------------------------------ 控制器硬件版本，可选择列表 ----------------------------------
#define	BMS_EP_A5				35				// 中力BMS硬件，A5
#define	BMS_EP_20_REV1_1_2TEMP	36				// 中力20AH量产PCB，V1.1，带2路温传，电池+BMS
#define BMS_EP_ALPHA            37
//#define	BMS_EP_200_A1			51			// 中力BMS硬件，对应60AH，外接分流器，正极接触器控制，A1
#define	BMS_EP_200_A2			52				// 中力BMS硬件，对应60AH，外接分流器，正极接触器控制，A2
#define	BMS_EP_200_B1			53				// 中力BMS硬件，对应60AH，外接分流器，正极接触器控制，Beta1.1
#define	BMS_EP_200_B2_1			54				// 中力BMS硬件，对应60AH，外接分流器，正极接触器控制,支持7-8串切换，Beta2.1
#define	BMS_XINXIN_A1			41				// 新昕BMS硬件，A1, 停止维护
#define	BMS_XINXIN_A2			42				// 新昕BMS硬件，A2_v3           


// ------------------------------------ 可选择电机列表 ------------------------------------
#define	MOTORTYPE_ACI			0				// 电机类型；0：交流异步电机
#define	MOTORTYPE_PMSM			1				// 电机类型：1：永磁同步电机

// ----------------------------------- BQ769x0芯片可选型号 --------------------------------
#define	BQ76920					1				// 1-5串
#define	BQ76930					2				// 6-10串
#define	BQ76940					3				// 11-15串


// --------------------- 以下为配置宏定义，可修改，修改需经过严格测试 --------------------------

// ----------------------- 选择控制器，电机，更改后需执行严格测试，确保无错误 -----------------------
#define	CONTROLLER_TARGET			BMS_EP_20_REV1_1_2TEMP  // 选择控制板硬件版本
#define	CONST_SOFTWARE_VERSION		11761					// 软件版本V1.2 50AH经济型锂电池参数
                                                            //软件版本v1.4.3      20AH12EZ锂电池参数
#define CONST_PARAMETER_VERSION     10                      //电芯类型，00代表三元，10代表铁锂                                                               
#define BATTERY_VERSION             121                     //121对应的电芯为经济型锂电池20AH，参数与原12EZ的参数不同
	                                                        //125对应的电芯为12EZ，电池容量为40AH
#define	MOTOR_TYPE					MOTORTYPE_ACI		    // 电机类型选择；0：异步电机ACI； 1：永磁同步电机PMSM

// ------------------------------- 设置控制器基本保护参数 ------------------------------------
#define	RATED_VOLTAGE_DEFAULT		_IQ(0.048)					// _IQdiv(48, VOLTAGE_BASE)	控制器默认电压； 48V
#define	MAX_VOLTAGE_DEFAULT			_IQ(0.065)					// _IQdiv(65, VOLTAGE_BASE) 控制器最大允许运行电压； 65V
#define	MIN_VOLTAGE_DEFAULT			_IQ(0.020)					// _IQdiv(20, VOLTAGE_BASE)	控制器最小允许运行电压； 20V
#define	MAX_CURRENT_DEFAULT			_IQ(0.65)					// _IQdiv(20, CURRENT_BASE)	控制器最大允许运行电流峰值; 650A

// --------------- 控制器特殊功能模式选择，部分设置牵扯底层驱动，勿随意改动 ---------------------
#define	ENCODER_ISR_MODE			0			// 编码器中断使能标志位
#define	FASTSAMPLEISR_MODE			0			// 高速采样模式，默认不启用
#define	PROGRAM_IN_RAM				0			// RAM中运行程序使能位

#define SVPWM_OVER_MODULATE			0			// 使用电压过调制

#define	RECORD_WRITE_TIMER			60			// 60s写入一次EEPROM运行记录，单位s

#define	EEPROM_FLASH_SIMULATE		1			// flash模拟EEPROM

// ------------------------- 测试用宏定义，正式代码中应禁用以下功能 -------------------------------


// --------------------------系统宏定义，标幺化基值，勿随意改动 -----------------------------------
#define EN_IWDG                     1

#define APPLICATION_ADDRESS     	0x08002000	// 系统代码启动位置，偏移0x2000，前部预留做bootloader

#define	INVERTER_PASSCODE			1234		// 逆变器启动密码，密码不合控制器禁止工作；0-9999

#define	DBTIME						2000		// 死区时间为2us， 部分产品死区不同，重点检查

#define SYSTEM_FREQUENCY			16			// Define the system frequency (MHz)

#define FASTSAMPLEISR_FREQUENCY 	20			// Define the FastSampleIsr frequency (kHz)
#define MAINISR_FREQUENCY 			10			// Define the MainIsr frequency (kHz)
#define VCISR_FRQUENCY				1			// Define the VCIsr frequency (kHz)

#define	CURRENT_BASE				1000		// 1000Apeak
#define VOLTAGE_BASE				1000		// 1000V
#define	FREQUENCY_BASE				1000		// 1000Hz
#define	TEMPERATURE_BASE			1000		// 1000℃

#define CAN_BAUDRATE                125         // CAN1波特率选择，125kbps

#define USART_BAUDRATE              115200       // 串口波特率，115200


// ---------------------- 根据控制器硬件版本，配置相应IO口 ------------------------

// ------------------------------------- 中力BMS硬件A5版本 -----------------------------------------
#if (CONTROLLER_TARGET == BMS_EP_A5)

#define MCU_ID_NUMBER                   0x1FFFF7AC  // mcu 硬件ID号起始地址，长度96位，对应m0内核

// --------------------------- flash模拟EEPROM ------------------------------------
// STM32F072Cx8，每页长度为2048Bytes，总共128K
#define FLASH_EEPROM_START_ADDRESS		0x0801D800		// 64K flash，总共32页，从第27页开始做EEPROM使用
#define	FLASH_EEPROM_BLOCK_LENGTH		0x0800			// 1页长度为2048bytes
#define	FLASH_EEPROM_BLOCK_NUMBER		5				// 限制最多使用5页，即27至31页

// --------------------- DMA通道有限，需统一分配 -----------------------------
#define USART_MODBUS_DMA_TX				DMA1_Channel4
#define USART_MODBUS_DMA_RX				DMA1_Channel5


#define ADC_CHANNEL_NUMBER				1					// ADC采样通道数
	
#define ADC_AIN1_CHANNEL				ADC_Channel_4		// 模拟输入通道1，温度采样1
#define ADC_AIN1_PORT					GPIOA
#define	ADC_AIN1_PIN					GPIO_Pin_4


// 串口MODBUS通信模块
#define USART_MODBUS_SELECT				2						// 选择串口2
#define	USART_MODBUS					USART2					
#define	USART_MODBUS_RS485_ENABLE		0						// RS485模式使能
#define	USART_MODBUS_REMAP_ENABLE		0						// IO口重映射使能

#define USART_MODBUS_BAUDRATE            9600       			   // 串口波特率，19200

#define USART_MODBUS_TX_PORT			GPIOA
#define	USART_MODBUS_TX_PIN				GPIO_Pin_2
#define	USART_MODBUS_TX_AF_PIN			GPIO_PinSource2
#define	USART_MODBUS_TX_AF				GPIO_AF_1

#define USART_MODBUS_RX_PORT			GPIOA
#define	USART_MODBUS_RX_PIN				GPIO_Pin_3
#define	USART_MODBUS_RX_AF_PIN			GPIO_PinSource3
#define	USART_MODBUS_RX_AF				GPIO_AF_1


// ------------------------ I2C模块，用于BQ769x0芯片通信 -------------------------
#define I2C_SELECT						2					// 选择I2C模块2
#define	I2C_BQ769						I2C2

#define I2C_SCL_PORT					GPIOB
#define	I2C_SCL_PIN						GPIO_Pin_13
#define	I2C_SCL_AF_PIN					GPIO_PinSource13
#define	I2C_SCL_AF						GPIO_AF_5

#define I2C_SDA_PORT					GPIOB
#define	I2C_SDA_PIN						GPIO_Pin_14
#define	I2C_SDA_AF_PIN					GPIO_PinSource14
#define	I2C_SDA_AF						GPIO_AF_5


// CAN模块
#define CAN_SELECT						1
#define	CAN_REMAP						0

#define CAN_RX_PORT						GPIOA
#define	CAN_RX_PIN						GPIO_Pin_11
#define	CAN_RX_AF_PIN					GPIO_PinSource11
#define	CAN_RX_AF						GPIO_AF_4

#define CAN_TX_PORT						GPIOA
#define	CAN_TX_PIN						GPIO_Pin_12
#define	CAN_TX_AF_PIN					GPIO_PinSource12
#define	CAN_TX_AF						GPIO_AF_4


// LCD接口IO定义
#define LCD_CS_PORT	          			GPIOB                   // 触发，低有效
#define LCD_CS_PIN                     	GPIO_Pin_1

#define	LCD_RST_PORT					GPIOB					// OLED复位，低有效
#define	LCD_RST_PIN						GPIO_Pin_0	

#define LCD_DC_PORT                    	GPIOA                   // 0:指令, 1:数据
#define LCD_DC_PIN                     	GPIO_Pin_3

#define	LCD_SPI							SPI1					// OLED用SPI模块选择

#define LCD_SPI_SCLK_PORT              	GPIOA                   // SPI时钟
#define LCD_SPI_SCLK_PIN               	GPIO_Pin_5

#define LCD_SPI_MOSI_PORT              	GPIOA                   // SPI输出数据
#define LCD_SPI_MOSI_PIN               	GPIO_Pin_7


// 驱动输出IO口
#define	GPIO_LED_RED_PORT				GPIOA					// LED驱动，红灯
#define GPIO_LED_RED_PIN				GPIO_Pin_10

#define	GPIO_LED_GREEN_PORT				GPIOA					// LED驱动，绿灯
#define GPIO_LED_GREEN_PIN				GPIO_Pin_9

#define	GPIO_LED_BLUE_PORT				GPIOA					// LED驱动，蓝灯
#define GPIO_LED_BLUE_PIN				GPIO_Pin_8

//#define GPIO_OUT1_PORT					GPIOB				// Out1，高有效
//#define GPIO_OUT1_PIN					GPIO_Pin_6

//#define GPIO_OUT2_PORT					GPIOA				// Out2, 高有效
//#define GPIO_OUT2_PIN					GPIO_Pin_10

// 数字输入IO口
// BQ769x0 ALERT 报警信号
// 0：无故障， 1：有故障或电流传感器数值有更新
// Take care： 此IO口也可以强制拉高，即强制触发BQ769x0芯片OVRD_ALERT故障，默认不使用，仅作为输入使用
#define GPIO_ALERT_PORT					GPIOB				
#define	GPIO_ALERT_PIN					GPIO_Pin_0

#define GPIO_KEY_PORT					GPIOB				// KEY，高有效
#define	GPIO_KEY_PIN					GPIO_Pin_12


#elif (CONTROLLER_TARGET == BMS_EP_20_REV1_1_2TEMP)

#define MCU_ID_NUMBER                   0x1FFFF7AC  // mcu 硬件ID号起始地址，长度96位，对应m0内核

// --------------------------- flash模拟EEPROM ------------------------------------
// STM32F072Cx8，每页长度为2048Bytes，总共64K           //0xFFFF -(0x400*5-1)=0xEC00
#define FLASH_EEPROM_START_ADDRESS		0x0800EC00		// 64K flash，总共64页，从第59页开始做EEPROM使用
#define	FLASH_EEPROM_BLOCK_LENGTH		0x0400			// 1页长度为1024bytes
#define	FLASH_EEPROM_BLOCK_NUMBER		5				// 限制最多使用5页，即59至63页

#define Addr_ZYBQ		0x36   //中颖地址
// --------------------- DMA通道有限，需统一分配 -----------------------------
// 设定使用的DMA通道及优先级
#define ADC_DMA							DMA1_Channel1
#define ADC_DMA_PRIORITY				DMA_Priority_VeryHigh

#define USART_MODBUS_DMA_TX				DMA1_Channel2
#define USART_MODBUS_DMA_TX_PRIORITY	DMA_Priority_Medium

#define USART_MODBUS_DMA_RX				DMA1_Channel3
#define USART_MODBUS_DMA_RX_PRIORITY	DMA_Priority_High


#define ADC_CHANNEL_NUMBER				3					// ADC采样通道数
	
#define ADC_AIN1_CHANNEL			    ADC_Channel_1		// 模拟输入通道1，温度采样1//电池内部
#define ADC_AIN1_PORT					GPIOA
#define	ADC_AIN1_PIN					GPIO_Pin_1

#define ADC_AIN2_CHANNEL				ADC_Channel_2		// 模拟输入通道2，温度采样2 //板载
#define ADC_AIN2_PORT					GPIOA
#define	ADC_AIN2_PIN					GPIO_Pin_2

#define ADC_AIN3_CHANNEL				ADC_Channel_8		// 模拟输入通道3，钥匙输入检测 //板载
#define ADC_AIN3_PORT					GPIOB
#define	ADC_AIN3_PIN					GPIO_Pin_0

// 串口MODBUS通信模块
#define USART_MODBUS_SELECT				1						// 选择串口2
#define	USART_MODBUS					USART1					
#define	USART_MODBUS_RS485_ENABLE		0						// RS485模式使能
#define	USART_MODBUS_REMAP_ENABLE		0						// IO口重映射使能

#define USART_MODBUS_BAUDRATE           9600       			// 串口波特率，19200

#define USART_MODBUS_TX_PORT			GPIOA
#define	USART_MODBUS_TX_PIN				GPIO_Pin_9
#define	USART_MODBUS_TX_AF_PIN			GPIO_PinSource9
#define	USART_MODBUS_TX_AF				GPIO_AF_1

#define USART_MODBUS_RX_PORT			GPIOA
#define	USART_MODBUS_RX_PIN				GPIO_Pin_10
#define	USART_MODBUS_RX_AF_PIN			GPIO_PinSource10
#define	USART_MODBUS_RX_AF				GPIO_AF_1


// ------------------------ I2C模块，用于BQ769x0芯片通信 -------------------------
#define I2C_SELECT						1					// 选择I2C模块2
#define	I2C_BQ769						I2C1

#define I2C_SCL_PORT					GPIOB
#define	I2C_SCL_PIN						GPIO_Pin_10
#define	I2C_SCL_AF_PIN					GPIO_PinSource13
#define	I2C_SCL_AF						GPIO_AF_5

#define I2C_SDA_PORT					GPIOB
#define	I2C_SDA_PIN						GPIO_Pin_11
#define	I2C_SDA_AF_PIN					GPIO_PinSource14
#define	I2C_SDA_AF						GPIO_AF_5




// 驱动输出IO口
#define	GPIO_LED_RED_PORT				GPIOB					// LED驱动，测试灯
#define GPIO_LED_RED_PIN				GPIO_Pin_9

#define	GPIO_LED_GREEN_PORT				GPIOA					// LED驱动，绿灯
#define GPIO_LED_GREEN_PIN				GPIO_Pin_1

#define	GPIO_LED_YELLOW_PORT			GPIOA					// LED驱动，黄灯
#define GPIO_LED_YELLOW_PIN				GPIO_Pin_2

#define	GPIO_LED_BLUE_PORT				GPIOA					// LED驱动，红灯
#define GPIO_LED_BLUE_PIN				GPIO_Pin_0

//#define	GPIO_PW_EN_PORT				    GPIOA					// PWR_EN驱动
//#define GPIO_PW_EN_PIN				    GPIO_Pin_12

#define	GPIO_DSGING_EN_PORT				GPIOB					// 电池欠压充电时打开 DSGING
#define GPIO_DSGING_EN_PIN				GPIO_Pin_2


#define GPIO_LOCK_PORT                  GPIOA                   //电源控制脚
#define GPIO_LOCK_PIN                   GPIO_Pin_4

#define	GPIO_EN_AFE_PORT				GPIOB					// 唤醒AFE
#define GPIO_EN_AFE_PIN				    GPIO_Pin_12


//#define	GPIO_PWM_OUT_PORT				GPIOB			       // PWM输出 用占空比表示电量
//#define GPIO_PWM_OUT_PIN				GPIO_Pin_3



#define GPIO_BUZZER_PORT                GPIOB                  //蜂鸣器驱动输出 就是HORN_M 控制接触器的     
#define GPIO_BUZZER_PIN                 GPIO_Pin_1

// 数字输入IO口
// BQ769x0 ALERT 报警信号
// 0：无故障， 1：有故障或电流传感器数值有更新
// Take care： 此IO口也可以强制拉高，即强制触发BQ769x0芯片OVRD_ALERT故障，默认不使用，仅作为输入使用
//#define GPIO_ALERT_PORT					GPIOB				
//#define	GPIO_ALERT_PIN					GPIO_Pin_15

//#define GPIO_KEY_PORT					GPIOB				// KEY，高有效
//#define	GPIO_KEY_PIN					GPIO_Pin_15

//#define GPIO_KEY_PORT2					GPIOA				// KEY，高有效
//#define	GPIO_KEY_PIN2					GPIO_Pin_7


#elif (CONTROLLER_TARGET == BMS_EP_ALPHA)

#define MCU_ID_NUMBER                   0x1FFFF7AC  // mcu 硬件ID号起始地址，长度96位，对应m0内核

// --------------------------- flash模拟EEPROM ------------------------------------
// STM32F072Cx8，每页长度为2048Bytes，总共64K
#define FLASH_EEPROM_START_ADDRESS		0x0800D800		// 64K flash，总共32页，从第27页开始做EEPROM使用
#define	FLASH_EEPROM_BLOCK_LENGTH		0x0800			// 1页长度为2048bytes
#define	FLASH_EEPROM_BLOCK_NUMBER		5				// 限制最多使用5页，即27至31页

// --------------------- DMA通道有限，需统一分配 -----------------------------
// 设定使用的DMA通道及优先级
#define ADC_DMA							DMA1_Channel1
#define ADC_DMA_PRIORITY				DMA_Priority_VeryHigh

#define USART_MODBUS_DMA_TX				DMA1_Channel4
#define USART_MODBUS_DMA_TX_PRIORITY	DMA_Priority_Medium

#define USART_MODBUS_DMA_RX				DMA1_Channel5
#define USART_MODBUS_DMA_RX_PRIORITY	DMA_Priority_High


#define ADC_CHANNEL_NUMBER				5					// ADC采样通道数

#define ADC_AIN1_CHANNEL				ADC_Channel_1		// 模拟输入通道1，温度采样1
#define ADC_AIN1_PORT					GPIOA
#define	ADC_AIN1_PIN					GPIO_Pin_1
	
#define ADC_AIN2_CHANNEL				ADC_Channel_4		// 模拟输入通道1，温度采样1
#define ADC_AIN2_PORT					GPIOA
#define	ADC_AIN2_PIN					GPIO_Pin_4

#define ADC_AIN3_CHANNEL				ADC_Channel_5		// 模拟输入通道2，温度采样2
#define ADC_AIN3_PORT					GPIOA
#define	ADC_AIN3_PIN					GPIO_Pin_5

#define ADC_AIN4_CHANNEL				ADC_Channel_6		// 模拟输入通道2，温度采样2
#define ADC_AIN4_PORT					GPIOA
#define	ADC_AIN4_PIN					GPIO_Pin_6

#define ADC_AIN5_CHANNEL				ADC_Channel_7		// 模拟输入通道2，温度采样2
#define ADC_AIN5_PORT					GPIOA
#define	ADC_AIN5_PIN					GPIO_Pin_7


// 串口MODBUS通信模块
#define USART_MODBUS_SELECT				2						// 选择串口2
#define	USART_MODBUS					USART2					
#define	USART_MODBUS_RS485_ENABLE		0						// RS485模式使能
#define	USART_MODBUS_REMAP_ENABLE		0						// IO口重映射使能

#define USART_MODBUS_BAUDRATE           19200       			// 串口波特率，19200

#define USART_MODBUS_TX_PORT			GPIOA
#define	USART_MODBUS_TX_PIN				GPIO_Pin_2
#define	USART_MODBUS_TX_AF_PIN			GPIO_PinSource2
#define	USART_MODBUS_TX_AF				GPIO_AF_1

#define USART_MODBUS_RX_PORT			GPIOA
#define	USART_MODBUS_RX_PIN				GPIO_Pin_3
#define	USART_MODBUS_RX_AF_PIN			GPIO_PinSource3
#define	USART_MODBUS_RX_AF				GPIO_AF_1


// ------------------------ I2C模块，用于BQ769x0芯片通信 -------------------------
#define I2C_SELECT						2					// 选择I2C模块2
#define	I2C_BQ769						I2C2

#define I2C_SCL_PORT					GPIOB
#define	I2C_SCL_PIN						GPIO_Pin_13
#define	I2C_SCL_AF_PIN					GPIO_PinSource13
#define	I2C_SCL_AF						GPIO_AF_5

#define I2C_SDA_PORT					GPIOB
#define	I2C_SDA_PIN						GPIO_Pin_14
#define	I2C_SDA_AF_PIN					GPIO_PinSource14
#define	I2C_SDA_AF						GPIO_AF_5


// CAN模块
#define CAN_SELECT						1
#define	CAN_REMAP						0

#define CAN_RX_PORT						GPIOA
#define	CAN_RX_PIN						GPIO_Pin_11
#define	CAN_RX_AF_PIN					GPIO_PinSource11
#define	CAN_RX_AF						GPIO_AF_4

#define CAN_TX_PORT						GPIOA
#define	CAN_TX_PIN						GPIO_Pin_12
#define	CAN_TX_AF_PIN					GPIO_PinSource12
#define	CAN_TX_AF						GPIO_AF_4

// 驱动输出IO口
#define	GPIO_LED_RED_PORT				GPIOA					// LED驱动，红灯
#define GPIO_LED_RED_PIN				GPIO_Pin_9

#define	GPIO_LED_GREEN_PORT				GPIOA					// LED驱动，绿灯
#define GPIO_LED_GREEN_PIN				GPIO_Pin_10

#define	GPIO_LED_BLUE_PORT				GPIOA					// LED驱动，蓝灯
#define GPIO_LED_BLUE_PIN				GPIO_Pin_8

// 数字输入IO口
// BQ769x0 ALERT 报警信号
// 0：无故障， 1：有故障或电流传感器数值有更新
// Take care： 此IO口也可以强制拉高，即强制触发BQ769x0芯片OVRD_ALERT故障，默认不使用，仅作为输入使用
#define GPIO_ALERT_PORT					GPIOB				
#define	GPIO_ALERT_PIN					GPIO_Pin_0

#define GPIO_KEY_PORT					GPIOB				// KEY，高有效
#define	GPIO_KEY_PIN					GPIO_Pin_12


// -------------------------- 中力BMS硬件,外接分流器，正极接触器控制，A1版本 -------------------------------
#elif (CONTROLLER_TARGET == BMS_EP_200_A1)

#define MCU_ID_NUMBER                   0x1FFFF7AC  // mcu 硬件ID号起始地址，长度96位，对应m0内核

// --------------------------- flash模拟EEPROM ------------------------------------
// STM32F072Cx8，每页长度为2048Bytes，总共64K
#define FLASH_EEPROM_START_ADDRESS		0x0800D800		// 64K flash，总共32页，从第27页开始做EEPROM使用
#define	FLASH_EEPROM_BLOCK_LENGTH		0x0800			// 1页长度为2048bytes
#define	FLASH_EEPROM_BLOCK_NUMBER		5				// 限制最多使用5页，即27至31页

// --------------------- DMA通道有限，需统一分配 -----------------------------
// 设定使用的DMA通道及优先级
#define ADC_DMA							DMA1_Channel1
#define ADC_DMA_PRIORITY				DMA_Priority_VeryHigh

#define USART_MODBUS_DMA_TX				DMA1_Channel4
#define USART_MODBUS_DMA_TX_PRIORITY	DMA_Priority_Medium

#define USART_MODBUS_DMA_RX				DMA1_Channel5
#define USART_MODBUS_DMA_RX_PRIORITY	DMA_Priority_High


#define ADC_CHANNEL_NUMBER				2					// ADC采样通道数
	
#define ADC_AIN1_CHANNEL				ADC_Channel_4		// 模拟输入通道1，温度采样1
#define ADC_AIN1_PORT					GPIOA
#define	ADC_AIN1_PIN					GPIO_Pin_4

#define ADC_AIN2_CHANNEL				ADC_Channel_5		// 模拟输入通道2，温度采样2
#define ADC_AIN2_PORT					GPIOA
#define	ADC_AIN2_PIN					GPIO_Pin_5


// 串口MODBUS通信模块
#define USART_MODBUS_SELECT				2						// 选择串口2
#define	USART_MODBUS					USART2					
#define	USART_MODBUS_RS485_ENABLE		0						// RS485模式使能
#define	USART_MODBUS_REMAP_ENABLE		0						// IO口重映射使能

#define USART_MODBUS_BAUDRATE           19200       			// 串口波特率，19200

#define USART_MODBUS_TX_PORT			GPIOA
#define	USART_MODBUS_TX_PIN				GPIO_Pin_2
#define	USART_MODBUS_TX_AF_PIN			GPIO_PinSource2
#define	USART_MODBUS_TX_AF				GPIO_AF_1

#define USART_MODBUS_RX_PORT			GPIOA
#define	USART_MODBUS_RX_PIN				GPIO_Pin_3
#define	USART_MODBUS_RX_AF_PIN			GPIO_PinSource3
#define	USART_MODBUS_RX_AF				GPIO_AF_1


// ------------------------ I2C模块，用于BQ769x0芯片通信 -------------------------
#define I2C_SELECT						2					// 选择I2C模块2
#define	I2C_BQ769						I2C2

#define I2C_SCL_PORT					GPIOB
#define	I2C_SCL_PIN						GPIO_Pin_13
#define	I2C_SCL_AF_PIN					GPIO_PinSource13
#define	I2C_SCL_AF						GPIO_AF_5

#define I2C_SDA_PORT					GPIOB
#define	I2C_SDA_PIN						GPIO_Pin_14
#define	I2C_SDA_AF_PIN					GPIO_PinSource14
#define	I2C_SDA_AF						GPIO_AF_5


// CAN模块
#define CAN_SELECT						1
#define	CAN_REMAP						0

#define CAN_RX_PORT						GPIOA
#define	CAN_RX_PIN						GPIO_Pin_11
#define	CAN_RX_AF_PIN					GPIO_PinSource11
#define	CAN_RX_AF						GPIO_AF_4

#define CAN_TX_PORT						GPIOA
#define	CAN_TX_PIN						GPIO_Pin_12
#define	CAN_TX_AF_PIN					GPIO_PinSource12
#define	CAN_TX_AF						GPIO_AF_4



// 驱动输出IO口
#define	GPIO_LED_RED_PORT				GPIOA					// LED驱动，红灯
#define GPIO_LED_RED_PIN				GPIO_Pin_10

#define	GPIO_LED_GREEN_PORT				GPIOA					// LED驱动，绿灯
#define GPIO_LED_GREEN_PIN				GPIO_Pin_8

#define	GPIO_LED_BLUE_PORT				GPIOA					// LED驱动，蓝灯
#define GPIO_LED_BLUE_PIN				GPIO_Pin_9


// 外部接触器PWM控制
#define	TIM_CONTACTOR_SELECT			2
#define TIM_CONTACTOR					TIM2
#define	TIM_CONTACTOR_CHANNEL			3

#define CONTACTOR_FREQUENCY				1						// 1kHz

#define GPIO_CONTACTOR_PORT				GPIOB					// 外部接触器控制，PWM输出，高有效
#define GPIO_CONTACTOR_PIN				GPIO_Pin_10
#define	GPIO_CONTACTOR_AF_PIN			GPIO_PinSource10
#define	GPIO_CONTACTOR_AF				GPIO_AF_2



//#define GPIO_OUT2_PORT					GPIOA				// Out2, 高有效
//#define GPIO_OUT2_PIN					GPIO_Pin_10

// 数字输入IO口
// BQ769x0 ALERT 报警信号
// 0：无故障， 1：有故障或电流传感器数值有更新
// Take care： 此IO口也可以强制拉高，即强制触发BQ769x0芯片OVRD_ALERT故障，默认不使用，仅作为输入使用
#define GPIO_ALERT_PORT					GPIOA				
#define	GPIO_ALERT_PIN					GPIO_Pin_6

// 外部控制开关，高有效
#define GPIO_KEY_PORT					GPIOB				// KEY，高有效
#define	GPIO_KEY_PIN					GPIO_Pin_12

// CHG：BQ769x0芯片充电控制引脚信号，高有效
#define GPIO_CHG_PORT					GPIOA				
#define	GPIO_CHG_PIN					GPIO_Pin_7

// CHG：BQ769x0芯片放电控制引脚信号，高有效
#define GPIO_DSG_PORT					GPIOB				// KEY，高有效
#define	GPIO_DSG_PIN					GPIO_Pin_0


// -------------------------- 中力BMS硬件,外接分流器，正极接触器控制，A2版本 -------------------------------
#elif (CONTROLLER_TARGET == BMS_EP_200_A2)

#define MCU_ID_NUMBER                   0x1FFFF7AC  // mcu 硬件ID号起始地址，长度96位，对应m0内核

// --------------------------- flash模拟EEPROM ------------------------------------
// STM32F072Cx8，每页长度为2048Bytes，总共64K
#define FLASH_EEPROM_START_ADDRESS		0x0800D800		// 64K flash，总共32页，从第27页开始做EEPROM使用
#define	FLASH_EEPROM_BLOCK_LENGTH		0x0800			// 1页长度为2048bytes
#define	FLASH_EEPROM_BLOCK_NUMBER		5				// 限制最多使用5页，即27至31页

// --------------------- DMA通道有限，需统一分配 -----------------------------
// 设定使用的DMA通道及优先级
#define ADC_DMA							DMA1_Channel1
#define ADC_DMA_PRIORITY				DMA_Priority_VeryHigh

#define USART_MODBUS_DMA_TX				DMA1_Channel4
#define USART_MODBUS_DMA_TX_PRIORITY	DMA_Priority_Medium

#define USART_MODBUS_DMA_RX				DMA1_Channel5
#define USART_MODBUS_DMA_RX_PRIORITY	DMA_Priority_High


#define ADC_CHANNEL_NUMBER				2					// ADC采样通道数
	
#define ADC_AIN1_CHANNEL				ADC_Channel_4		// 模拟输入通道1，温度采样1
#define ADC_AIN1_PORT					GPIOA
#define	ADC_AIN1_PIN					GPIO_Pin_4

#define ADC_AIN2_CHANNEL				ADC_Channel_5		// 模拟输入通道2，温度采样2
#define ADC_AIN2_PORT					GPIOA
#define	ADC_AIN2_PIN					GPIO_Pin_5


// 串口MODBUS通信模块
#define USART_MODBUS_SELECT				2						// 选择串口2
#define	USART_MODBUS					USART2					
#define	USART_MODBUS_RS485_ENABLE		0						// RS485模式使能
#define	USART_MODBUS_REMAP_ENABLE		0						// IO口重映射使能

#define USART_MODBUS_BAUDRATE           19200       			// 串口波特率，19200

#define USART_MODBUS_TX_PORT			GPIOA
#define	USART_MODBUS_TX_PIN				GPIO_Pin_2
#define	USART_MODBUS_TX_AF_PIN			GPIO_PinSource2
#define	USART_MODBUS_TX_AF				GPIO_AF_1

#define USART_MODBUS_RX_PORT			GPIOA
#define	USART_MODBUS_RX_PIN				GPIO_Pin_3
#define	USART_MODBUS_RX_AF_PIN			GPIO_PinSource3
#define	USART_MODBUS_RX_AF				GPIO_AF_1


// ------------------------ I2C模块，用于BQ769x0芯片通信 -------------------------
#define I2C_SELECT						2					// 选择I2C模块2
#define	I2C_BQ769						I2C2

#define I2C_SCL_PORT					GPIOB
#define	I2C_SCL_PIN						GPIO_Pin_13
#define	I2C_SCL_AF_PIN					GPIO_PinSource13
#define	I2C_SCL_AF						GPIO_AF_5

#define I2C_SDA_PORT					GPIOB
#define	I2C_SDA_PIN						GPIO_Pin_14
#define	I2C_SDA_AF_PIN					GPIO_PinSource14
#define	I2C_SDA_AF						GPIO_AF_5


// CAN模块
#define CAN_SELECT						1
#define	CAN_REMAP						0

#define CAN_RX_PORT						GPIOA
#define	CAN_RX_PIN						GPIO_Pin_11
#define	CAN_RX_AF_PIN					GPIO_PinSource11
#define	CAN_RX_AF						GPIO_AF_4

#define CAN_TX_PORT						GPIOA
#define	CAN_TX_PIN						GPIO_Pin_12
#define	CAN_TX_AF_PIN					GPIO_PinSource12
#define	CAN_TX_AF						GPIO_AF_4



// 驱动输出IO口
#define	GPIO_LED_RED_PORT				GPIOA					// LED驱动，红灯
#define GPIO_LED_RED_PIN				GPIO_Pin_10

#define	GPIO_LED_GREEN_PORT				GPIOA					// LED驱动，绿灯
#define GPIO_LED_GREEN_PIN				GPIO_Pin_8

#define	GPIO_LED_BLUE_PORT				GPIOA					// LED驱动，蓝灯
#define GPIO_LED_BLUE_PIN				GPIO_Pin_9


// 外部接触器PWM控制
#define	TIM_CONTACTOR_SELECT			2
#define TIM_CONTACTOR					TIM2
#define	TIM_CONTACTOR_CHANNEL			3

#define CONTACTOR_FREQUENCY				1						// 1kHz

#define GPIO_CONTACTOR_PORT				GPIOB					// 外部接触器控制，PWM输出，高有效
#define GPIO_CONTACTOR_PIN				GPIO_Pin_10
#define	GPIO_CONTACTOR_AF_PIN			GPIO_PinSource10
#define	GPIO_CONTACTOR_AF				GPIO_AF_2



//#define GPIO_OUT2_PORT					GPIOA				// Out2, 高有效
//#define GPIO_OUT2_PIN					GPIO_Pin_10

// 数字输入IO口
// BQ769x0 ALERT 报警信号
// 0：无故障， 1：有故障或电流传感器数值有更新
// Take care： 此IO口也可以强制拉高，即强制触发BQ769x0芯片OVRD_ALERT故障，默认不使用，仅作为输入使用
#define GPIO_ALERT_PORT					GPIOA				
#define	GPIO_ALERT_PIN					GPIO_Pin_6

// 外部控制开关，高有效
#define GPIO_KEY_PORT					GPIOB				// KEY，高有效
#define	GPIO_KEY_PIN					GPIO_Pin_12

// CHG：BQ769x0芯片充电控制引脚信号，高有效
#define GPIO_CHG_PORT					GPIOA				
#define	GPIO_CHG_PIN					GPIO_Pin_7

// CHG：BQ769x0芯片放电控制引脚信号，高有效
#define GPIO_DSG_PORT					GPIOB				// KEY，高有效
#define	GPIO_DSG_PIN					GPIO_Pin_0


// -------------------------- 中力BMS硬件,外接分流器，正极接触器控制，Beta1.1版本 -------------------------------
#elif (CONTROLLER_TARGET == BMS_EP_200_B1)

#define MCU_ID_NUMBER                   0x1FFFF7AC  // mcu 硬件ID号起始地址，长度96位，对应m0内核

// --------------------------- flash模拟EEPROM ------------------------------------
// STM32F072Cx8，每页长度为2048Bytes，总共64K
#define FLASH_EEPROM_START_ADDRESS		0x0800D800		// 64K flash，总共32页，从第27页开始做EEPROM使用
#define	FLASH_EEPROM_BLOCK_LENGTH		0x0800			// 1页长度为2048bytes
#define	FLASH_EEPROM_BLOCK_NUMBER		5				// 限制最多使用5页，即27至31页

// --------------------- DMA通道有限，需统一分配 -----------------------------
// 设定使用的DMA通道及优先级
#define ADC_DMA							DMA1_Channel1
#define ADC_DMA_PRIORITY				DMA_Priority_VeryHigh

#define USART_MODBUS_DMA_TX				DMA1_Channel4
#define USART_MODBUS_DMA_TX_PRIORITY	DMA_Priority_Medium

#define USART_MODBUS_DMA_RX				DMA1_Channel5
#define USART_MODBUS_DMA_RX_PRIORITY	DMA_Priority_High


#define ADC_CHANNEL_NUMBER				2					// ADC采样通道数
	
#define ADC_AIN1_CHANNEL				ADC_Channel_4		// 模拟输入通道1，温度采样1
#define ADC_AIN1_PORT					GPIOA
#define	ADC_AIN1_PIN					GPIO_Pin_4

#define ADC_AIN2_CHANNEL				ADC_Channel_5		// 模拟输入通道2，温度采样2
#define ADC_AIN2_PORT					GPIOA
#define	ADC_AIN2_PIN					GPIO_Pin_5


// 串口MODBUS通信模块
#define USART_MODBUS_SELECT				2						// 选择串口2
#define	USART_MODBUS					USART2					
#define	USART_MODBUS_RS485_ENABLE		0						// RS485模式使能
#define	USART_MODBUS_REMAP_ENABLE		0						// IO口重映射使能

#define USART_MODBUS_BAUDRATE           19200       			// 串口波特率，19200

#define USART_MODBUS_TX_PORT			GPIOA
#define	USART_MODBUS_TX_PIN				GPIO_Pin_2
#define	USART_MODBUS_TX_AF_PIN			GPIO_PinSource2
#define	USART_MODBUS_TX_AF				GPIO_AF_1

#define USART_MODBUS_RX_PORT			GPIOA
#define	USART_MODBUS_RX_PIN				GPIO_Pin_3
#define	USART_MODBUS_RX_AF_PIN			GPIO_PinSource3
#define	USART_MODBUS_RX_AF				GPIO_AF_1


// ------------------------ I2C模块，用于BQ769x0芯片通信 -------------------------
#define I2C_SELECT						2					// 选择I2C模块2
#define	I2C_BQ769						I2C2

#define I2C_SCL_PORT					GPIOB
#define	I2C_SCL_PIN						GPIO_Pin_13
#define	I2C_SCL_AF_PIN					GPIO_PinSource13
#define	I2C_SCL_AF						GPIO_AF_5

#define I2C_SDA_PORT					GPIOB
#define	I2C_SDA_PIN						GPIO_Pin_14
#define	I2C_SDA_AF_PIN					GPIO_PinSource14
#define	I2C_SDA_AF						GPIO_AF_5


// CAN模块
#define CAN_SELECT						1
#define	CAN_REMAP						0

#define CAN_RX_PORT						GPIOA
#define	CAN_RX_PIN						GPIO_Pin_11
#define	CAN_RX_AF_PIN					GPIO_PinSource11
#define	CAN_RX_AF						GPIO_AF_4

#define CAN_TX_PORT						GPIOA
#define	CAN_TX_PIN						GPIO_Pin_12
#define	CAN_TX_AF_PIN					GPIO_PinSource12
#define	CAN_TX_AF						GPIO_AF_4



// 驱动输出IO口
#define	GPIO_LED_RED_PORT				GPIOA					// LED驱动，红灯
#define GPIO_LED_RED_PIN				GPIO_Pin_10

#define	GPIO_LED_GREEN_PORT				GPIOA					// LED驱动，绿灯
#define GPIO_LED_GREEN_PIN				GPIO_Pin_8

#define	GPIO_LED_BLUE_PORT				GPIOA					// LED驱动，蓝灯
#define GPIO_LED_BLUE_PIN				GPIO_Pin_9


// 外部接触器PWM控制
#define	TIM_CONTACTOR_SELECT			2
#define TIM_CONTACTOR					TIM2
#define	TIM_CONTACTOR_CHANNEL			3

#define CONTACTOR_FREQUENCY				1						// 1kHz

#define GPIO_CONTACTOR_PORT				GPIOB					// 外部接触器控制，PWM输出，高有效
#define GPIO_CONTACTOR_PIN				GPIO_Pin_10
#define	GPIO_CONTACTOR_AF_PIN			GPIO_PinSource10
#define	GPIO_CONTACTOR_AF				GPIO_AF_2



//#define GPIO_OUT2_PORT					GPIOA				// Out2, 高有效
//#define GPIO_OUT2_PIN					GPIO_Pin_10

// 数字输入IO口
// BQ769x0 ALERT 报警信号
// 0：无故障， 1：有故障或电流传感器数值有更新
// Take care： 此IO口也可以强制拉高，即强制触发BQ769x0芯片OVRD_ALERT故障，默认不使用，仅作为输入使用
#define GPIO_ALERT_PORT					GPIOA				
#define	GPIO_ALERT_PIN					GPIO_Pin_15

// 外部控制开关，高有效
#define GPIO_KEY_PORT					GPIOB				// KEY，高有效
#define	GPIO_KEY_PIN					GPIO_Pin_12

// CHG：BQ769x0芯片充电控制引脚信号，高有效
#define GPIO_CHG_PORT					GPIOB		
#define	GPIO_CHG_PIN					GPIO_Pin_3

// CHG：BQ769x0芯片放电控制引脚信号，高有效
#define GPIO_DSG_PORT					GPIOB				// KEY，高有效
#define	GPIO_DSG_PIN					GPIO_Pin_4


// -------------------------- 中力BMS硬件,外接分流器，正极接触器控制，支持7-8串切换，Beta2.1版本 -------------------------------
#elif (CONTROLLER_TARGET == BMS_EP_200_B2_1)

#define MCU_ID_NUMBER                   0x1FFFF7AC  // mcu 硬件ID号起始地址，长度96位，对应m0内核

// --------------------------- flash模拟EEPROM ------------------------------------
// STM32F072Cx8，每页长度为2048Bytes，总共64K
#define FLASH_EEPROM_START_ADDRESS		0x0800D800		// 64K flash，总共32页，从第27页开始做EEPROM使用
#define	FLASH_EEPROM_BLOCK_LENGTH		0x0800			// 1页长度为2048bytes
#define	FLASH_EEPROM_BLOCK_NUMBER		5				// 限制最多使用5页，即27至31页

// --------------------- DMA通道有限，需统一分配 -----------------------------
// 设定使用的DMA通道及优先级
#define ADC_DMA							DMA1_Channel1
#define ADC_DMA_PRIORITY				DMA_Priority_High

#define USART_MODBUS_DMA_TX				DMA1_Channel4
#define USART_MODBUS_DMA_TX_PRIORITY	DMA_Priority_Medium

#define USART_MODBUS_DMA_RX				DMA1_Channel5
#define USART_MODBUS_DMA_RX_PRIORITY	DMA_Priority_VeryHigh


#define ADC_CHANNEL_NUMBER				3					// ADC采样通道数
	
#define ADC_AIN1_CHANNEL				ADC_Channel_6		// 模拟输入通道1，温度采样1，采样电池包温度
#define ADC_AIN1_PORT					GPIOA
#define	ADC_AIN1_PIN					GPIO_Pin_6

#define ADC_AIN2_CHANNEL				ADC_Channel_5		// 模拟输入通道2，温度采样2，采样电池包温度
#define ADC_AIN2_PORT					GPIOA
#define	ADC_AIN2_PIN					GPIO_Pin_5

#define ADC_AIN3_CHANNEL				ADC_Channel_4		// 模拟输入通道3，温度采样3，采样均流板温度
#define ADC_AIN3_PORT					GPIOA
#define	ADC_AIN3_PIN					GPIO_Pin_4


// 串口MODBUS通信模块
#define USART_MODBUS_SELECT				2						// 选择串口2
#define	USART_MODBUS					USART2					
#define	USART_MODBUS_RS485_ENABLE		0						// RS485模式使能
#define	USART_MODBUS_REMAP_ENABLE		0						// IO口重映射使能

#define USART_MODBUS_BAUDRATE           19200       			// 串口波特率，19200

#define USART_MODBUS_TX_PORT			GPIOA
#define	USART_MODBUS_TX_PIN				GPIO_Pin_2
#define	USART_MODBUS_TX_AF_PIN			GPIO_PinSource2
#define	USART_MODBUS_TX_AF				GPIO_AF_1

#define USART_MODBUS_RX_PORT			GPIOA
#define	USART_MODBUS_RX_PIN				GPIO_Pin_3
#define	USART_MODBUS_RX_AF_PIN			GPIO_PinSource3
#define	USART_MODBUS_RX_AF				GPIO_AF_1


// ------------------------ I2C模块，用于BQ769x0芯片通信 -------------------------
#define I2C_SELECT						2					// 选择I2C模块2
#define	I2C_BQ769						I2C2

#define I2C_SCL_PORT					GPIOB
#define	I2C_SCL_PIN						GPIO_Pin_13
#define	I2C_SCL_AF_PIN					GPIO_PinSource13
#define	I2C_SCL_AF						GPIO_AF_5

#define I2C_SDA_PORT					GPIOB
#define	I2C_SDA_PIN						GPIO_Pin_14
#define	I2C_SDA_AF_PIN					GPIO_PinSource14
#define	I2C_SDA_AF						GPIO_AF_5


// CAN模块
#define CAN_SELECT						1
#define	CAN_REMAP						0

#define CAN_RX_PORT						GPIOA
#define	CAN_RX_PIN						GPIO_Pin_11
#define	CAN_RX_AF_PIN					GPIO_PinSource11
#define	CAN_RX_AF						GPIO_AF_4

#define CAN_TX_PORT						GPIOA
#define	CAN_TX_PIN						GPIO_Pin_12
#define	CAN_TX_AF_PIN					GPIO_PinSource12
#define	CAN_TX_AF						GPIO_AF_4



// 驱动输出IO口
#define	GPIO_LED_RED_PORT				GPIOA					// LED驱动，红灯，低有效
#define GPIO_LED_RED_PIN				GPIO_Pin_7

#define	GPIO_LED_GREEN_PORT				GPIOB					// LED驱动，绿灯，低有效
#define GPIO_LED_GREEN_PIN				GPIO_Pin_1

#define	GPIO_LED_BLUE_PORT				GPIOB					// LED驱动，蓝灯，低有效
#define GPIO_LED_BLUE_PIN				GPIO_Pin_0

#define	GPIO_LED_TEST_PORT				GPIOB					// LED驱动，测试灯，低有效
#define GPIO_LED_TEST_PIN				GPIO_Pin_2


// 外部接触器PWM控制
#define	TIM_CONTACTOR_SELECT			2
#define TIM_CONTACTOR					TIM2
#define	TIM_CONTACTOR_CHANNEL			3

#define CONTACTOR_FREQUENCY				1						// 1kHz

#define GPIO_CONTACTOR_PORT				GPIOB					// 外部接触器控制，PWM输出，高有效
#define GPIO_CONTACTOR_PIN				GPIO_Pin_10
#define	GPIO_CONTACTOR_AF_PIN			GPIO_PinSource10
#define	GPIO_CONTACTOR_AF				GPIO_AF_2



//#define GPIO_OUT2_PORT					GPIOA				// Out2, 高有效
//#define GPIO_OUT2_PIN					GPIO_Pin_10

// 数字输入IO口
// BQ769x0 ALERT 报警信号
// 0：无故障， 1：有故障或电流传感器数值有更新
// Take care： 此IO口也可以强制拉高，即强制触发BQ769x0芯片OVRD_ALERT故障，默认不使用，仅作为输入使用
#define GPIO_ALERT_PORT					GPIOA				
#define	GPIO_ALERT_PIN					GPIO_Pin_15

// 外部控制开关，高有效
#define GPIO_KEY_PORT					GPIOB				// KEY，高有效
#define	GPIO_KEY_PIN					GPIO_Pin_5

// CHG：BQ769x0芯片充电控制引脚信号，高有效
#define GPIO_CHG_PORT					GPIOB		
#define	GPIO_CHG_PIN					GPIO_Pin_3

// CHG：BQ769x0芯片放电控制引脚信号，高有效
#define GPIO_DSG_PORT					GPIOB				// KEY，高有效
#define	GPIO_DSG_PIN					GPIO_Pin_4


// --------------------------------------- 新昕BMS硬件第一版 ------------------------------------------
#elif (CONTROLLER_TARGET == BMS_XINXIN_A1)

#define MCU_ID_NUMBER                   0x1FFFF7AC  // mcu 硬件ID号起始地址，长度96位，对应m0内核

// --------------------------- flash模拟EEPROM ------------------------------------
// STM32F072Cx8，每页长度为2048Bytes，总共64K
#define FLASH_EEPROM_START_ADDRESS		0x0800D800		// 64K flash，总共32页，从第27页开始做EEPROM使用
#define	FLASH_EEPROM_BLOCK_LENGTH		0x0800			// 1页长度为2048bytes
#define	FLASH_EEPROM_BLOCK_NUMBER		5				// 限制最多使用5页，即27至31页	
	
#define ADC_CHANNEL_NUMBER				3					// ADC采样通道数
	
#define ADC_AIN1_CHANNEL				ADC_Channel_4		// 模拟输入通道1，温度采样1
#define ADC_AIN1_PORT					GPIOA
#define	ADC_AIN1_PIN					GPIO_Pin_4

#define ADC_AIN2_CHANNEL				ADC_Channel_4		// 模拟输入通道2，温度采样2
#define ADC_AIN2_PORT					GPIOA
#define	ADC_AIN2_PIN					GPIO_Pin_4

#define ADC_AIN3_CHANNEL				ADC_Channel_4		// 模拟输入通道3，温度采样3
#define ADC_AIN3_PORT					GPIOA
#define	ADC_AIN3_PIN					GPIO_Pin_4


// 串口通信模块
#define USART_SELECT					1					// 选择串口1
#define	USART1_REMAP_ENABLE				1					// IO口重映射使能

#define USART_TX_PORT					GPIOB
#define	USART_TX_PIN					GPIO_Pin_6

#define USART_RX_PORT					GPIOB
#define	USART_RX_PIN					GPIO_Pin_7

// ------------------------ I2C模块，用于BQ769x0芯片通信 -------------------------
#define I2C_SELECT						2					// 选择I2C模块2
#define	I2C_BQ769						I2C2

#define I2C_SCL_PORT					GPIOB
#define	I2C_SCL_PIN						GPIO_Pin_10
#define	I2C_SCL_AF_PIN					GPIO_PinSource10
#define	I2C_SCL_AF						GPIO_AF_1

#define I2C_SDA_PORT					GPIOB
#define	I2C_SDA_PIN						GPIO_Pin_11
#define	I2C_SDA_AF_PIN					GPIO_PinSource11
#define	I2C_SDA_AF						GPIO_AF_1


// CAN模块
#define CAN_SELECT						1
#define	CAN_REMAP						0

#define CAN_RX_PORT						GPIOA
#define	CAN_RX_PIN						GPIO_Pin_11
#define	CAN_RX_AF_PIN					GPIO_PinSource11
#define	CAN_RX_AF						GPIO_AF_4

#define CAN_TX_PORT						GPIOA
#define	CAN_TX_PIN						GPIO_Pin_12
#define	CAN_TX_AF_PIN					GPIO_PinSource12
#define	CAN_TX_AF						GPIO_AF_4


// LCD接口IO定义
#define LCD_CS_PORT	          			GPIOB                   // 触发，低有效
#define LCD_CS_PIN                     	GPIO_Pin_1

#define	LCD_RST_PORT					GPIOB					// OLED复位，低有效
#define	LCD_RST_PIN						GPIO_Pin_0	

#define LCD_DC_PORT                    	GPIOA                   // 0:指令, 1:数据
#define LCD_DC_PIN                     	GPIO_Pin_3

#define	LCD_SPI							SPI1					// OLED用SPI模块选择

#define LCD_SPI_SCLK_PORT              	GPIOA                   // SPI时钟
#define LCD_SPI_SCLK_PIN               	GPIO_Pin_5

#define LCD_SPI_MOSI_PORT              	GPIOA                   // SPI输出数据
#define LCD_SPI_MOSI_PIN               	GPIO_Pin_7


// 驱动输出IO口
#define GPIO_PREDISCHARGE_PORT			GPIOA					// 电池预放电控制，高有效
#define GPIO_PREDISCHARGE_PIN			GPIO_Pin_3


// 数字输入IO口
// BQ769x0 ALERT 报警信号
// 0：无故障， 1：有故障，SYS_STAT(0x00)寄存器中有故障信息
// Take care： 此IO口也可以强制拉高，即强制触发BQ769x0芯片OVRD_ALERT故障，默认不使用，仅作为输入使用
#define GPIO_ALERT_PORT					GPIOB				
#define	GPIO_ALERT_PIN					GPIO_Pin_13

#define GPIO_KEY_PORT					GPIOB				// KEY，高有效
#define	GPIO_KEY_PIN					GPIO_Pin_12


// 新昕BMS硬件第二版
#elif (CONTROLLER_TARGET == BMS_XINXIN_A2)

#define MCU_ID_NUMBER                   0x1FFFF7AC  // mcu 硬件ID号起始地址，长度96位，对应m0内核

// --------------------------- flash模拟EEPROM ------------------------------------
// STM32F072Cx8，每页长度为2048Bytes，总共64K
#define FLASH_EEPROM_START_ADDRESS		0x0800D800		// 64K flash，总共32页，从第27页开始做EEPROM使用
#define	FLASH_EEPROM_BLOCK_LENGTH		0x0800			// 1页长度为2048bytes
#define	FLASH_EEPROM_BLOCK_NUMBER		5				// 限制最多使用5页，即27至31页	

// --------------------- DMA通道有限，需统一分配 -----------------------------
// 设定使用的DMA通道及优先级
#define ADC_DMA							DMA1_Channel1
#define ADC_DMA_PRIORITY				DMA_Priority_High

#define USART_MODBUS_DMA_TX				DMA1_Channel2
#define USART_MODBUS_DMA_TX_PRIORITY	DMA_Priority_Medium

#define USART_MODBUS_DMA_RX				DMA1_Channel3
#define USART_MODBUS_DMA_RX_PRIORITY	DMA_Priority_VeryHigh


#define ADC_CHANNEL_NUMBER				0					// ADC采样通道数
	
#define ADC_AIN1_CHANNEL				ADC_Channel_4		// 模拟输入通道1，温度采样1
#define ADC_AIN1_PORT					GPIOA
#define	ADC_AIN1_PIN					GPIO_Pin_4


// 串口MODBUS通信模块
#define USART_MODBUS_SELECT				1						// 选择串口1
#define	USART_MODBUS					USART1					
#define	USART_MODBUS_RS485_ENABLE		0						// RS485模式使能
#define	USART_MODBUS_REMAP_ENABLE		0						// IO口重映射使能

#define USART_MODBUS_BAUDRATE           19200       			// 串口波特率，19200

#define USART_MODBUS_TX_PORT			GPIOB
#define	USART_MODBUS_TX_PIN				GPIO_Pin_6
#define	USART_MODBUS_TX_AF_PIN			GPIO_PinSource6
#define	USART_MODBUS_TX_AF				GPIO_AF_0

#define USART_MODBUS_RX_PORT			GPIOB
#define	USART_MODBUS_RX_PIN				GPIO_Pin_7
#define	USART_MODBUS_RX_AF_PIN			GPIO_PinSource7
#define	USART_MODBUS_RX_AF				GPIO_AF_0


// ------------------------ I2C模块，用于BQ769x0芯片通信 -------------------------
#define I2C_SELECT						2					// 选择I2C模块2
#define	I2C_BQ769						I2C2

#define I2C_SCL_PORT					GPIOB
#define	I2C_SCL_PIN						GPIO_Pin_13
#define	I2C_SCL_AF_PIN					GPIO_PinSource13
#define	I2C_SCL_AF						GPIO_AF_5

#define I2C_SDA_PORT					GPIOB
#define	I2C_SDA_PIN						GPIO_Pin_14
#define	I2C_SDA_AF_PIN					GPIO_PinSource14
#define	I2C_SDA_AF						GPIO_AF_5


// CAN模块
#define CAN_SELECT						1

/*
#define CAN_RX_PORT						GPIOA
#define	CAN_RX_PIN						GPIO_Pin_11
#define	CAN_RX_AF_PIN					GPIO_PinSource11
#define	CAN_RX_AF						GPIO_AF_4

#define CAN_TX_PORT						GPIOA
#define	CAN_TX_PIN						GPIO_Pin_12
#define	CAN_TX_AF_PIN					GPIO_PinSource12
#define	CAN_TX_AF						GPIO_AF_4
*/

#define CAN_RX_PORT						GPIOB
#define	CAN_RX_PIN						GPIO_Pin_8
#define	CAN_RX_AF_PIN					GPIO_PinSource8
#define	CAN_RX_AF						GPIO_AF_4

#define CAN_TX_PORT						GPIOB
#define	CAN_TX_PIN						GPIO_Pin_9
#define	CAN_TX_AF_PIN					GPIO_PinSource9
#define	CAN_TX_AF						GPIO_AF_4




// 驱动输出IO口
#define GPIO_PREDISCHARGE_PORT			GPIOA					// 电池预放电控制，高有效
#define GPIO_PREDISCHARGE_PIN			GPIO_Pin_8


// 数字输入IO口
// BQ769x0 ALERT 报警信号
// 0：无故障， 1：有故障，SYS_STAT(0x00)寄存器中有故障信息
// Take care： 此IO口也可以强制拉高，即强制触发BQ769x0芯片OVRD_ALERT故障，默认不使用，仅作为输入使用
#define GPIO_ALERT_PORT					GPIOB				
#define	GPIO_ALERT_PIN					GPIO_Pin_15

#define GPIO_KEY_PORT					GPIOB				// KEY，高有效
#define	GPIO_KEY_PIN					GPIO_Pin_12


#endif          // CONTROLLER_TARGET == SMARTDISPLAY_EP_A2





