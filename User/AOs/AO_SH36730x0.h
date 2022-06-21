/* ==================================================================================

 File name:     AO_BQ769x0.h
 Originator:    BLJ
 Description:   BQ769x0通信及控制相关状态机，主要处理I2C通信及寄存器读写

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 07-14-2016     Version 0.9.1           增加CRC校验错误处理，待验证。
 07-01-2016     Version 0.9.0           代码整理完成，部分功能可再优化，待长期测试
 01-02-2015     Version 0.0.1           测试功能通过
-----------------------------------------------------------------------------------*/
#ifndef __AO_SH36730X0_H
#define __AO_SH36730X0_H
#include "stm32f0xx.h"                                         // STM32器件寄存器定义头文件
#include "qpn_port.h"                                           // 状态机头文件
                                                                
// ------------------------------- 全局宏定义 ---------------------------------------
//#define BQ769X0_CLEAR_SYS_STAT          1                     // BQ769x0 芯片系统状态清除指令
//#define BQ769X0_CONTROL_ON              2                     // BQ769x0 芯片更新充放电回路指令,开启充放电回路
//#define BQ769X0_CONTROL_OFF             3                     // BQ769x0 芯片更新充放电回路指令,关闭充放电回路

#define SH36730x0_RECORD_LENGTH           16                    // BQ769x0状态机跳转记录长度

// -------------------------- 显示模块活动对象结构体 ---------------------------------
struct AO_SH36730x0_State {
    u32 SampleUpdateCount;                                      // 采样更新次数
    u16 EnterSHIPModeAsk;                                       // 进入低功耗模式请求，外部置位
    u16 BatteryBalanceEnable;                                   // 电池均衡使能
    u16 BatteryBalancing;                                       // 电池均衡中，0：无电池均衡，1：均衡中
    u16 BatteryBalancePointer;                                  // 第几节电池在均衡
    u16 Timer1sUpdateAsk;                                       // 1s定时更新请求标志位，若收到定时更新请求时正在执行通信，则此标志位置位，回到Idle时立刻执行更新指令
    u16 SH36730x0ControlAsk;                                    // SH36730x0芯片控制指令，若收到指令时正在执行通信，则此数据设置为控制指令，回到Idle时立刻执行指令
    u16 DSGControl;                                             // 放电控制控制状态，0：关闭，1：开启
    u16 CHGControl;                                             // 充电控制控制状态，0：关闭，1：开启   
    u16 CANSendEnable;                                          // CAN通信发送使能
	u16	test_ForceBatteryBalancePointer;						// 测试用，强制指定均流对象
};

struct AO_SH36730x0_Parameter {
    s32 ADCGain;                                                // 电压放大比例，单位uV
    s32 ADCOffset;                                              // 电压采样静态偏差，单位mV
    s32 SingleOverVoltage;                                      // 单体电池过压
    s32 SingleUnderVoltage;                                     // 单体电池欠压
    s32 CCGain;                                                 // 电流传感器放大比例，直接比例，需设置
    s32 CCOffset;                                               // 电流传感器偏置，直接作用在采样结果上
    s32 BallanceErrVoltage;                                     // 电池均流误差电流，单位mV
    u16 SH36730x0_Type;                                         // BQ769x0芯片选型，1:SH367303,2:SH367305,3:SH36736
    u16 CellNumber;                                             // 总共电池串联数
    u8  CellSelect[10];                                         // 单体连接选择
};

struct AO_SH36730x0_Variable_bits {
    u32 Block:4;                                                // Variable: EEPROM has five blocks for information, parameter, record ......
    u32 BlockStack1:4;                                          // Variable: EEPROM block in stack
    u32 BlockStack2:4;                                          // Variable: EEPROM block in stack
    u32 StackCount:4;                                           // Variable: block numbers in stack
    u32 SystemRecordPageFoundFlag:1;                            // 系统运行记录所在页被找到
    u32 SystemRecordPage:2;                                     // 系统运行记录所在页
    u32 SystemRecordFoundFlag:1;                                // Variable: The last vehicle record has been found flag
    u32 WriteParameterAllow:1;                                  // 收到EEPROM模块发出的系统参数正常读取事件，允许配置SH36730x0芯片参数
    u32 rsvd:10;
};

struct AO_SH36730x0_Variable_union {
    u32                                   all;
    struct AO_SH36730x0_Variable_bits     bit;
};

struct AO_SH36730x0_Variable {
    struct  AO_SH36730x0_Variable_union   Variable_bits;        //
    u32 TotalTimeRecord;                                        // Variable: use for total time record in first read //总时间记录变量
    u32 mileageOld;                                             // Variable: Old mileage， use for record as mileage 100m once//里程记录
    u16 SampleUpdateCount;                                      // 采样次数，第一次采样读取可能出错，屏蔽之
    u16 EEPROMFaultInf;                                         // Variable: EEPROM fault information             //故障信息
    u16 RecordWriteDelay;                                       // Variable: 运行记录写入EEPROM延时计数，单位s
    u16 EEPROMWriteBlock;                                       // Variable: EEPROM写入指令区块
    u16 EEPROMWriteBlockWait;                                   // 等待写入区块
    u16 SystemRecordCircleNumber;                               // 系统运行记录数组编号，0-63  *2
    u16 SystemRecordRetryCount;                                 // 系统运行记录读取重试次数，若读取的运行记录LRC校验出错，则读取上一次记录，此变量++，限制重试次数
    u16 CellNumberPointer;                                      // 已计算单体电压数量
    u16 ClearSystemStateCount;                                  // 系统故障恢复尝试次数
    u16 ParameterWriteNumber;                                   // 系统参数写入块数
    u16 UpdateCount;                                            // 电池数据更新计数器，每60s更新一次电池均流及充放电回路管理
    u16 EnterSHIPStep;                                          // 进入SHIP模式需发送指定数据，分三步进行
    u16 ReadValueCount;                                         // 数据读取次数
    u16 CCReadedFlag;                                           // 电流采样值已读取标志位
    u16 I2CFaultRetryCount;                                     // I2C通讯失败重试次数，默认限制最多5次
    u16 CRCCheckRetryCount;                                     // BSH36730芯片通信CRC校验失败重试次数，最多限制为3次，收到正确数据清除该计数器
	u16 FaultEnterSHIPDelay;									// SH芯片故障时，进入低功耗模式前延时
    u16 I2CTryRecoverFlag;                                      // I2C总线出错，尝试修复时置位，修复完成清零。
};

struct AO_SH36730x0_Output {                                    //传出参数
    s32 SingleVoltage[15];                                      // 电池单体电压，单位mV，最多10串
    s32 BatteryVoltage;                                         // 电池总电压，单位mV
    s32 BatteryCurrent;                                         // 电池电流，单位mA，
    s32 InnerTemperature[3];                                    // BQ769x0温度传感器，测量内部温度，单位0.1摄氏度
    s32 ExternalTemperature[3];                                 // BQ769x0温度传感器，测量外部温度，单位0.1摄氏度
    s32 SingleMaxVoltage;                                       // 单体最高电压
    s32 SingleMinVoltage;                                       // 单体最低电压
    u32 ReadValueTime;                                          // 读取数据时，的时间撮，即1ms定时器计数值
    u16 SingleMaxVoltagePointer;                                // 单体最高电压对应第几节，1-7
    u16 SingleMinVoltagePointer;                                // 单体最高电压对应第几节，1-7
};

// -------------------------------------------------- 搭建SH36730x0寄存器结构体 -------------------------------------------------------------------------------------
// 0x00：SYS_STAT 系统标志寄存器   复位值：0 
struct SH36730x0_FLAG1_bits {
    u8  TWI:1;                                                  // 1：TWI通讯发生 Timeout
    u8  WDT:1;                                                  // 1：看门狗溢出
    u8  OV:1;                                                   // 1：触发硬件过充电保护
    u8  SC:1;                                                   // 1：触发硬件短路保护
	u8  rsvd:4;                                                 //补位值
                                           
};

union SH36730x0_FLAG1_union {                                   //共用体列表成员共用一个地址
    u8                              all;
    struct  SH36730x0_FLAG1_bits   bit;
};



// 0x01：FLAG2  标志位寄存器2  复位值;0

struct SH36730x0_FLAG2_bits {
    u8  VADC:1;   //  1：VADC 中断发生标志位  读取后硬件清除
    u8  CADC:1;   //  1：CADC 中断发生标志位  读取后硬件清除
    u8  RST:1;    //  1：V33  Reset发生标志   读取后硬件清除
	u8  rsvd:5;

};

union SH36730x0_FLAG2_union {
    u8                              all;
    struct  SH36730x0_FLAG2_bits   bit;
};

// 0x02：BSTATUS 状态寄存器  复位值;0
struct SH36730x0_BSTATUS_bits {
    u8  CHGR:1;     // 1：充电器连接状态位
    u8  LOAD:1;     // 1：负载连接状态位
    u8  CHGING:1;   // 1：充电状态位
    u8  DSGING:1;   // 1: 放电状态位
    u8  CHG:1;      // 1；充电MOS开启状态位
    u8  DSG:1;      // 1：放电MOS开启状态位
	u8  rsvd:2;
};

union SH36730x0_BSTATUS_union {
    u8                              all;
    struct  SH36730x0_BSTATUS_bits   bit;
};

// 0x03：INT_EN  ALARM 控制寄存器
struct SH36730x0_INT_EN_bits {
    u8  TWI_INT:1;      //1：TWI Timeout中断使能位 
    u8  WDT_INT:1;      //1: 看门狗WDT中断使能为
    u8  VADC_INT:1;     //1: VADC转换中断使能（转换完成后ALARM管脚输出低电平脉冲）
    u8  CADC_INT:1;     //1: CADC转换中断使能（转换完成后ALARM管脚输出低电平脉）
    u8  CD_INT:1;       //1: 充放电状态中断使能位 检测到充放电后 ALARM管脚输出报警信号
    u8  OV_INT:1;       //1: 硬件过充保护中断使能位
	u8  SC_INT:1;       //1: 硬件短路保护中断使能位
	u8  rsvd:1;
};

union SH36730x0_INT_EN_union {
    u8                              all;
    struct  SH36730x0_INT_EN_bits   bit;
};

// 0x04: SCONF1  系统控制寄存器1
struct SH36730x0_SCONF1_bits {
    u8  CHGR_EN:1;        //1：充电器检测使能位
    u8  LOAD_EN:1;        //1：负载检测使能控制位
    u8  OV_EN:1;          //1：硬件过充保护使能控制位
    u8  SC_EN:1;          //1：硬件短路保护使能控制位
    u8  WDT_EN:1;         //1: 看门狗使能控制位
    u8  PD_EN:1;          //1：低功耗状态控制位
    u8  CTLD_EN:1;        //1: CTLD功能控制位  (使能时CTLD管脚优先控制DSG(放电MOS)管脚功能)
	u8  LTCLR:1;          //1: LTCLR 清除标志寄存器（FLAG1）
};

union SH36730x0_SCONF1_union {
    u8                              all;
    struct  SH36730x0_SCONF1_bits  bit;
};

// 0x05: SCONF2 系统控制寄存器2
struct SH36730x0_SCONF2_bits {
    u8  CHG_C:1;          // 1：充电MOS控制位
    u8  DSG_C:1;          // 1：放电MOS控制位
    u8  ALARM_C:1;        // 1: ALARM输出选择位 （0输出低电平脉冲 1 输出低电平） 
    u8  RESETorPF:1;      // 1：复位外部MCU/二级保护选择位
	u8  rsvd:4;
                         
};

union SH36730x0_SCONF2_union {
    u8                              all;
    struct  SH36730x0_SCONF2_bits  bit;
};

// 0x06: SCONF3 系统控制寄存器3
struct SH36730x0_SCONF3_bits {
    u8  SCAN_C:3;         // VADC转换周期选择位  手册46页
    u8  VADC_C:1;         // 1：VADC采样方式选择位 0开启电压采集 1开启电压和温度采集
    u8  VADC_EN:1;        // 1：VADC使能控制位
    u8  CBIT_C:1;         // 1：CADC精度选择位 0->10 bit  1->13 bit
    u8  CADC_M:1;         // 1: CADC采样方式选择位 0单次采集完成后EN位自动清零  连续采集
    u8  CADC_EN:1; 	      // 1: CADC使能控制位
};

union SH36730x0_SCONF3_union {
    u8                              all;
    struct  SH36730x0_SCONF3_bits   bit;
};

// 0x07: SCONF4  
struct SH36730x0_SCONF4_bits {
    u8  CB6:1;            //1:平衡回路控制位 6 
    u8  CB7:1;            //1:平衡回路控制位 7         
    u8  CB8:1;            //1:平衡回路控制位 8
	u8  CB9:1;            //1:平衡回路控制位 9
	u8  CB10:1;           //1:平衡回路控制位 10
	u8  rsvd:3;
};

union SH36730x0_SCONF4_union {
    u8                              all;
    struct  SH36730x0_SCONF4_bits   bit;
};

// 0x08: SCONF5
struct SH36730x0_SCONF5_bits {
    u8  CB1:1;            //1:平衡回路控制位 1 
    u8  CB2:1;            //1:平衡回路控制位 2         
    u8  CB3:1;            //1:平衡回路控制位 3
	u8  CB4:1;            //1:平衡回路控制位 4
	u8  CB5:1;            //1:平衡回路控制位 5                                         // 0-3：欠压延时
	u8  rsvd:3;
};

union SH36730x0_SCONF5_union {
    u8                              all;
    struct  SH36730x0_SCONF5_bits   bit;
};

// 0x09: SCONF6
struct SH36730x0_SCONF6_bits {
    u8  SCTn:2;            //[0:1]硬件短路保护延时设置选择位
    u8  SCVn:2;            //[2:3]硬件短路保护电压设置选择位         
    u8  RSTn:2;            //[4:5]复位外部MCU脉冲宽度选择位
	u8  RSNSn:2;           //[6:7]CADC采集范围选择位              
};

union SH36730x0_SCONF6_union {
    u8                              all;
    struct  SH36730x0_SCONF6_bits   bit;
};

// 0x0A: SCONF7
struct SH36730x0_SCONF7_bits {
    u8  WDTtn:2;            //[0:1]看门狗溢出时间选择位
    u8  SHSn:2;             //[2:3]充放电状态检测阈值选择位         
    u8  OVTn:3;             //[4:6]硬件过充电保护延时选择位  当检测到过充且持续时间超过设置时间开启保护功能
	u8  rsvd:1;
};

union SH36730x0_SCONF7_union {
    u8                              all;
    struct  SH36730x0_SCONF7_bits   bit;
};


// 0x0B: SCONF8
struct SH36730x0_SCONF8_bits {
    u8  OVD8:1;             //硬件过充保护阈值   计算方式：寄存器值*5.86mv  //*注意 *注意 *注意SCONF8/9寄存器复位值为1
    u8  OVD9:1;             //硬件过充保护阈值      
    u8  rsvd:6;	
};

union SH36730x0_SCONF8_union {
    u8                              all;
    struct  SH36730x0_SCONF8_bits   bit;
};

// 0x0C: SCONF9
struct SH36730x0_SCONF9_bits {
    u8  OVD0:1;             
    u8  OVD1:1;
    u8  OVD2:1;             
    u8  OVD3:1; 
    u8  OVD4:1;             
    u8  OVD5:1; 	
    u8  OVD6:1;             
    u8  OVD7:1; 
};

// 0x0C: SCONF9
union SH36730x0_SCONF9_union {
    u8                              all;
    struct  SH36730x0_SCONF9_bits   bit;
};



// 0x0D: SCONF10
struct SH36730x0_SCONF10 {
    u8  PINn;            //进低功耗控制位 先写入PIN[7:0] =0x33 后写入PD_EN=1 AFE关闭LDO 进入低功耗
};



// 0x0E~0x21: CELLn   10个寄存器保持单体电压
struct SH36730x0_CELLn {
    u8  CELLnH;            //[0:3]
    u8  CELLnL;            //[0:7]
};


// 0x22~0x25: TSn   温度寄存器   2个寄存器    当数据转换完成后，数据更新为TSn管脚上的电压分压比对应数值
struct SH36730x0_TSn {
    u8  TSnH;            //[0:3]
    u8  TSnL;            //[0:7]
};

// 0x26~0x29: TEMPn   温度寄存器   2个寄存器   当数据转换完成后，数据更新为内部温度检测值
struct SH36730x0_TEMPn {
    u8  TEMPnH;            //[0:3]
    u8  TEMPnL;            //[0:7]
};

// 0x2A~0x2B: CUR   电流寄存器     当数据转换完成后，数据更新（RS2-RS1电流采集管脚）电压对应的码值
struct SH36730x0_CUR  {
    u8  CURH;            //[0:4]  //CUR[12]为符号位 1表示放电 0表示充电
    u8  CURL;            //[0:7]
};


//------------------------------------------------------------------------------------

struct SH36730x0_Register {
    union SH36730x0_FLAG1_union       FLAG1;                   // 0x00: 系统标志寄存器
    union SH36730x0_FLAG2_union       FLAG2;                   // 0x01：系统标志寄存器
    union SH36730x0_BSTATUS_union     BSTATUS;                 // 0x02  状态寄存器
    union SH36730x0_INT_EN_union      INT_EN;                  // 0x03  中断请求标志位

    union SH36730x0_SCONF1_union      SCONF1;                  // 0x04  系统控制寄存器1
    union SH36730x0_SCONF2_union      SCONF2;                  // 0x05   
    union SH36730x0_SCONF3_union      SCONF3;                  // 0x06
    union SH36730x0_SCONF4_union      SCONF4;                  // 0x07
    union SH36730x0_SCONF5_union      SCONF5;                  // 0x08
    union SH36730x0_SCONF6_union      SCONF6;                  // 0x09
    union SH36730x0_SCONF7_union      SCONF7;                  // 0x0A
	union SH36730x0_SCONF8_union      SCONF8;                  // 0x0B
    union SH36730x0_SCONF9_union      SCONF9;                  // 0x0C 
    struct SH36730x0_SCONF10          SCONF10;                 // 0x0D   系统控制寄存器10

    struct SH36730x0_CELLn            CELL[10];                // 0x0E~0x21  单体电芯电压寄存器
    struct SH36730x0_TSn              TS[2];                   // 0x22~0x25  温度寄存器
    struct SH36730x0_TEMPn            TEMP[2];                 // 0x26~0x29  温度寄存器
    struct SH36730x0_CUR              CUR;                     // 0x2A~0x2B  电流寄存器 
    
};



typedef struct AO_SH36730x0Tag {                      
    QActive super;                                                     // derive from QActive
    struct  AO_SH36730x0_State        State;                           // 模块状态
    struct  AO_SH36730x0_Parameter    Parameter;                       // SH36730x0 芯片参数
    struct  AO_SH36730x0_Variable     Variable;                        // 模块临时变量
    struct  AO_SH36730x0_Output       Output;                          // 输出采样值
    struct  SH36730x0_Register        SH36730x0Register;               // SH36730x0系列AFE芯片寄存器结构体
    u16                               Record[SH36730x0_RECORD_LENGTH]; // 状态跳转记录
} AO_SH36730x0;


//extern AO_SH36730x0 g_AO_SH36730x0; 


// ------------------------------ 声明清除函数 ---------------------------------
    void AO_SH36730x0_ctor(AO_SH36730x0 * const me);
                                                  
// ------------------------------ 声明活动状态 ---------------------------------
	static QState AO_SH36730x0_initial(AO_SH36730x0 * const me);                         // 初始状态
	static QState AO_SH36730x0_StartWait(AO_SH36730x0 * const me);                       // 1.启动等待状态
	static QState AO_SH36730x0_Normal(AO_SH36730x0 * const me);                          // 2.正常状态，通信无错误
	static QState AO_SH36730x0_ReadParameter(AO_SH36730x0 * const me);                   // 3.读取参数配置，读取寄存器0x00-0x0B
	static QState AO_SH36730x0_ClearSystemState(AO_SH36730x0 * const me);                // 301.清除系统状态，故障状态SYS_STAT
	static QState AO_SH36730x0_ReadParameter2(AO_SH36730x0 * const me);                  // 31.读取参数配置2，读取寄存器0x50-0x51
	static QState AO_SH36730x0_ReadParameter3(AO_SH36730x0 * const me);                  // 32.读取参数配置3，读取寄存器0x59
	static QState AO_SH36730x0_WaitWriteParameter(AO_SH36730x0 * const me);              // 321.参数读取完成，等待EEPROM读取系统参数完成，方可对BQ769芯片进行配置
	static QState AO_SH36730x0_WriteParameter(AO_SH36730x0 * const me);                  // 33.设置系统参数
	static QState AO_SH36730x0_Idle(AO_SH36730x0 * const me);                            // 4.空闲状态
	static QState AO_SH36730x0_ReadValue(AO_SH36730x0 * const me);                       // 9.读取采样信息
	static QState AO_SH36730x0_UpdateControl(AO_SH36730x0 * const me);                   // 10.更新控制值，均衡电流，开关充放电回路
	static QState AO_SH36730x0_EnterSHIP(AO_SH36730x0 * const me);                       // 12.进入低功耗状态
	static QState AO_SH36730x0_Fault(AO_SH36730x0 * const me); 
    static QState AO_SH36730x0_CloseBalance(AO_SH36730x0 * const me);	                 //读取电压前关闭均衡
    static QState AO_SH36730x0_ReadConfig(AO_SH36730x0 * const me);                     //读取配置状态
   
// End 
#endif
