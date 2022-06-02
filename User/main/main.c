/* ==================================================================================

 File name:     main.c
 Originator:    BLJ
 Description:   main启动函数，状态机活动对象实例化

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 12-02-2014     Version 0.1.0		增加状态机,测试通过    
 10-06-2014     Version 0.0.1       测试功能，GPIO
-----------------------------------------------------------------------------------*/

#include "stm32f0xx.h"                     // STM32器件寄存器定义头文件
#include "target.h"                         // 目标板选择
#include "system_core.h"                    // 系统核心头文件

//注意此项目STM32F030驱动库中有部分函数被屏蔽--------------------------------------------

// ------------------------------ 状态机活动对象实例化 ----------------------------------
// 定义活动对象
AO_EEPROM 	  g_AO_EEPROM;						// EEPROM活动对象结构体
AO_SH36730x0  g_AO_SH36730x0; 					// SH36730x0 I2C 通信控制状态机
AO_BMS		  g_AO_BMS;							// BMS管理状态机




// 定义事件池
static QEvt l_EEPROMQSto[10];					// Event queue storage for EEPROM
static QEvt l_SH36730x0QSto[10];				// Event queue storage for BQ769x0
static QEvt	l_BMSQSto[10];						// Event queue storage for BMS


// 分配活动对象地址空间
QActiveCB const Q_ROM QF_active[] = {
    { (QActive *)0,           	 (QEvt *)0,        0U                     },
    { (QActive *)&g_AO_EEPROM,   l_EEPROMQSto,     Q_DIM(l_EEPROMQSto)    },
	{ (QActive *)&g_AO_SH36730x0,l_SH36730x0QSto,  Q_DIM(l_SH36730x0QSto) },
	{ (QActive *)&g_AO_BMS,  	 l_BMSQSto,    	   Q_DIM(l_BMSQSto) 	  },
};

// Take care, 确认活动对象个数,QF_MAX_ACTIVE为活动对象个数，需相应修改
// make sure that the QF_active[] array matches QF_MAX_ACTIVE in qpn_port.h
Q_ASSERT_COMPILE(QF_MAX_ACTIVE == Q_DIM(QF_active) - 1);

//0x0800EC00
// ----------------------------- 主函数，程序启动入口 ----------------------------------
u8 UID_check(void);
const u32 MCU_ID_head  = 0xFFFFFFFF;
const u32 MCU_ID_trail = 0xFFFFFFFF;
const u16 MCU_ID_check = 0xFFFF;
//int tm=0;
//u16 ID=0; //0x0800E800;
int main(void)
{	
//必须先执行系统初始化，其中有临时变量初始化
    System_Init();                      		
	__enable_irq();//开启全局中断
   if(!UID_check())
   {
     Protect_SetFaultCodeLv0(&g_Protect, ERROR_CODE_COPY);
   }
   
   
   AO_EEPROM_ctor(&g_AO_EEPROM);        		// 初始化状态机，清除变量 
   AO_SH36730x0_ctor(&g_AO_SH36730x0);   		// 初始化状态机，清除变量
   AO_BMS_ctor(&g_AO_BMS);   					// 初始化状态机，清除变量
	
   return QF_run();                         	// 初始化完成开始运行QF-nano
}

//End of main.c
//FLASH_ProgramWord
//FLASH_ProgramHalfWord(u32 address, u16 data)
/*
软件加密
MCU ID 96位 取前32位与最后32位作为校验 
校验之前还需要校验用户自己设置的校验码
校验码设置规则 大小为16位 十六进制 内容任意 如20218 = 0x4EFA
*/
u8 UID_check(void)
{
	if(*(u16*)&MCU_ID_check==0xFFFF)//
	{
	  FLASH_ProgramHalfWord((u32)&MCU_ID_check,0x4EFA);//写入校验码：校验码相当于用户密码
		
	  if((*(u32*)&MCU_ID_head==0xFFFFFFFF)&&(*(u32*)&MCU_ID_trail==0xFFFFFFFF))//第一次下载
	  { 
	   FLASH_ProgramWord((u32)&MCU_ID_head,*(__IO u32*)(MCU_ID_NUMBER + 0));
       FLASH_ProgramWord((u32)&MCU_ID_trail,*(__IO u32*)(MCU_ID_NUMBER + 8));  
	  return 1;
	  }
	  else
	  {
		if(*(u32*)&MCU_ID_head == *(__IO u32*)(MCU_ID_NUMBER + 0))
		{
			if(*(u32*)&MCU_ID_trail == *(__IO u32*)(MCU_ID_NUMBER + 8))
			{
				return 1;
			}
		}
		return 0;
	  }

	}
	else if(*(u16*)&MCU_ID_check == 0x4EFA)
	{
		if(*(u32*)&MCU_ID_head == *(__IO u32*)(MCU_ID_NUMBER + 0))
		{
			if(*(u32*)&MCU_ID_trail == *(__IO u32*)(MCU_ID_NUMBER + 8))
			{
				return 1;
			}
		}
		return 0;
	}
	else
		return 0;
}
	
