/* ==================================================================================

 File name:     AO_SH36730x0.c
 Originator:    BLJ
 Description:   SH36730x0通信及控制相关状态机，主要处理I2C通信及寄存器读写
 Take care：    所有状态活动对象，定时器1用于周期性定时

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 07-14-2016     Version 0.9.1           增加CRC校验错误处理，待验证。
 07-01-2016     Version 0.9.0           代码整理完成，部分功能可再优化，待长期测试
 01-02-2015     Version 0.0.1           测试功能通过
-----------------------------------------------------------------------------------*/

#include "stm32f0xx.h"                          // MM32器件寄存器定义头文件
#include "string.h"                             // 调用memset函数用
#include "system_core.h"                        // 系统核心头文件
#include "qpn_port.h"                           // 状态机头文件
#include <math.h>


//u8 DATA_DEBUG[30]={0};

/*
*   状态机运行记录
*   调试用
*/

void AO_SH36730x0_Record(AO_SH36730x0 * const me, u16 i)
{
    u16 n = 0;

    // 所有记录向后移动一位
    for (n = (SH36730x0_RECORD_LENGTH - 1); n > 0; n--)
    {
        me->Record[n] = me->Record[n-1];
    }

    // 填入新记录
    me->Record[0] = i;
}


/*
*  内部调用函数声明
*/

void SH36730x0_ParameterUpdate(AO_SH36730x0 * const me);// 根据系统参数配置，更新SH36730芯片寄存器参数
void SH36730x0_SampleUpdate(AO_SH36730x0 * const me);  // SH36730x0芯片采样值处理更新函数    
void SH36730x0_UpdateControl(AO_SH36730x0 * const me);// SH36730x0芯片控制更新函数

/*
*  清除函数
*/ 
void AO_SH36730x0_ctor(AO_SH36730x0 * const me) {
    QActive_ctor(&me->super, Q_STATE_CAST(&AO_SH36730x0_initial));
}

/*
*   初始转换
*/
QState AO_SH36730x0_initial(AO_SH36730x0 * const me) {
    return Q_TRAN(&AO_SH36730x0_StartWait);
}

/*
*   1.等待EEPROM读取完成,启动AFE配置 
*/


QState AO_SH36730x0_StartWait(AO_SH36730x0 * const me) {   
  
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {  
			//SH36730x0_ParameterUpdate(me);//=============================================测试用------------------
            //QACTIVE_POST((QActive *)&g_AO_SH36730x0, START_SIG, 0);//=========================
            AO_SH36730x0_Record(me, 1);
            me->Variable.ClearSystemStateCount = 0; // 清除系统临时变量   
			
            status = Q_HANDLED(); 
        }
        break;
		
        case Q_EXIT_SIG:
        {
            status = Q_HANDLED();
        }
        break;

        case START_SIG:   //AO_EEPROM中函数编号6 61 63  发送  POST                             
        {      
		    
			SH36730x0_ParameterUpdate(me);//SH36730芯片初始配置数据初始化
			
			//DATA_DEBUG[0]++;//==================================================================================	
            me->Variable.Variable_bits.bit.WriteParameterAllow = 1;  
		    //QACTIVE_POST((QActive *)&g_AO_SH36730x0, START_SIG, 0); 
			
            status = Q_TRAN(&AO_SH36730x0_Normal); 
        }
        break;		
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);            
        }
        break;
    }
    
    return status;
}
	


/* 
*     2.通信正常状态 
*     多数状态的父状态,负责处理通讯超时及通讯错误事件
*/

u8 count_set=0;    //配置AFE计数
u8 updata_count=0; //更新控制值计数
u8 close_step=0;   //读取前关闭均衡计数
u8 Read_times = 0; //读取采样值计数
u8 SHIP_step=0;    //低功耗步骤
u8 Balance_time=0; //均衡时间
u8 Delay_time=0;   //均衡时间
u8 Balance_flag=0; //均衡时间
QState AO_SH36730x0_Normal(AO_SH36730x0 * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            AO_SH36730x0_Record(me, 2);
	
            status = Q_HANDLED();           
        }
        break;
        
        case Q_INIT_SIG: 
        {   
			//DATA_DEBUG[1]++;//==================================================================================
            status = Q_TRAN(&AO_SH36730x0_ReadParameter); // 默认跳转至读取寄存器状态   
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);                 // 关闭定时器0
			
            status = Q_HANDLED();
        }
        break;
        
        case Q_TIMEOUT_SIG:  //定时器0 1s 超时计时
        {
			//DATA_DEBUG[2]++;//===============================================================
            QActive_disarmX((QActive *)me, 0U);   //关闭定时器
			//================================================
			//通讯错误或者超时时清除步骤标志
			 count_set=0;
			 updata_count=0;
			 close_step=0;
			 Read_times = 0;
			 SHIP_step=0;
			 Balance_time=0;
			//================================================			
			
			me->Variable.I2CFaultRetryCount++;
			
			if (me->Variable.I2CFaultRetryCount <= 8)//通讯失败重试次数小于
			{
				 
				//I2C_BQ769xx_DeviceInit();//通信失败重新初始化硬件
				
                me->Variable.I2CTryRecoverFlag = 1;  //尝试修复I2C总线标志位   				
         
				QActive_armX((QActive *)me, 1U, 100U);//定时器1
				
				status = Q_HANDLED();				
			}
			else
			{
				Protect_SetFaultCodeLv0(&g_Protect, FAULT_BQ769_I2C_OVERTIME);// 设置I2C通信超时标志位
				
				status = Q_TRAN(&AO_SH36730x0_Fault); //跳转至通信故障状态
			}
        }
        break;  
		
		
        case Q_TIMEOUT1_SIG:  //定时器0超时后重新初始化引脚 100ms后重新配置
        {	
			QActive_disarmX((QActive *)me, 1U); //关闭定时器1
			
			status = Q_TRAN(&AO_SH36730x0_ReadParameter); 
        }
        break;  
		
		
        case I2C_SH36730x0_FAIL_SIG:// 芯片通讯失败//应答失败或CRC校验失败
        {
            //DATA_DEBUG[3]++;//==============================================================================
			QActive_disarmX((QActive *)me, 0U); //关闭定时器0
			
            me->Variable.I2CFaultRetryCount++;  
			
			//================================================
			//通讯错误或者超时时清除步骤标志
			 count_set=0;
			 updata_count=0;
			 close_step=0;
			 Read_times = 0;
			 SHIP_step=0;
			//================================================	
			
			if (me->Variable.I2CFaultRetryCount <= 8) //连续失败8次 
			{
				I2C_BQ769xx_DeviceInit();//SH3673xx芯片I2C总线通信模块初始化			
				
				QActive_armX((QActive *)me, 1U, 100U); //开启定时器1，100ms
				
				status = Q_TRAN(&AO_SH36730x0_ReadParameter); 
			}
			else
			{

				Protect_SetFaultCodeLv0(&g_Protect, FAULT_BQ769_I2C_FAIL);
				// 通信失败，跳转至通信故障状态
				status = Q_TRAN(&AO_SH36730x0_Fault);
			}
        }
        break;   		 
                                 
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);                        // 父状态为top状态       
        }
        break;
    }
    
    return status;
}


/*
*    3.读取FLAG1 FLAG2参数
*/

QState AO_SH36730x0_ReadParameter(AO_SH36730x0 * const me)
 {  
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
			//DATA_DEBUG[4]++;//==================================================================================
            AO_SH36730x0_Record(me, 3);
			//读取芯片FLAG故障标志  一次读取两个                                                                                         
			I2C_BQ769x0_Read(&g_I2C_SH36730x0,Addr_ZYBQ,(u8*)&me->SH36730x0Register.FLAG1,0x00 ,1);
			 
            QActive_armX((QActive *)me, 0U, 1000U);//1s定时限制 超时在父状态处理
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);                 // 关闭定时器0
            
            status = Q_HANDLED();
        }
        break;
        
        case I2C_SH36730x0_FINISH_SIG:
        {
            //DATA_DEBUG[5]++;//==================================================================================	
            QActive_disarmX((QActive *)me, 0U);
            me->Variable.CRCCheckRetryCount = 0;//收到正确数据帧时会清除CRC校验错误次数 
			
            //判断SH36730芯片无报错 即没有发生通讯超时 看门狗溢出 短路保护 过充保护
            if (me->SH36730x0Register.FLAG1.all == 0x00) 
            {	
				//DATA_DEBUG[6]++;//==================================================================================
				if (me->Variable.Variable_bits.bit.WriteParameterAllow > 0)
				{      
                //DATA_DEBUG[7]++;//==================================================================================					
					//EEPROM系统参数读取完成，开始配置SH36730x0芯片
					status = Q_TRAN(&AO_SH36730x0_WriteParameter);                
				}
				else
				{
					//DATA_DEBUG[8]++;//==================================================================================
					//EEPROM系统参数未读取完成 ，跳转至等待设置系统参数状态
					status = Q_TRAN(&AO_SH36730x0_WaitWriteParameter); 
				}  
             
            }
            else  // SH36730芯片故障标志清除
            {
                if (me->Variable.ClearSystemStateCount < 5)
                {
                    me->Variable.ClearSystemStateCount++;
                    
                    status = Q_TRAN(&AO_SH36730x0_ClearSystemState);//清除FLAG1标志位
					
                }
                else
                {
					//DATA_DEBUG[10]++;//==================================================================================
                    me->Variable.ClearSystemStateCount = 0;   
                    
                    Protect_SetFaultCodeLv0(&g_Protect, SYS_STAT_NOT_CLEAR);// 设置故障标志位,芯片系统报错无法恢复  

                    status = Q_TRAN(&AO_SH36730x0_Fault);// 跳转至故障状态          
                } 
            }
        }
        break;
 

        default: 
        {
            status = Q_SUPER(&AO_SH36730x0_Normal);               // 父状态为Normal状态   
        }
        break;
    }
    
    return status; 

 }	
 

/*
*    配置SH36730芯片
*/
//u8 count_set=0;
QState AO_SH36730x0_WriteParameter(AO_SH36730x0 * const me) 
{
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {           
            AO_SH36730x0_Record(me,33); 
			
            //SH36730x0_ParameterUpdate(me);//SH36730芯片初始配置数据初始化		
			         
			switch(count_set) //将新参数写入SH36730x0寄存器
			{
				default:
				case 0:         //充电检测使能 硬件过充保护使能 看门狗使能 使能CTLD管脚优先控制放电MOS
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x03, (u8*)&me->SH36730x0Register.INT_EN.all, 1);
				    break;
				case 1:         //充电检测使能 硬件过充保护使能 看门狗使能 使能CTLD管脚优先控制放电MOS
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x04, (u8*)&me->SH36730x0Register.SCONF1.all, 1);
				    break;
			    case 2:         //使能复位外部MCU功能(默认)  设置ALARM硬件持续输出低电平
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x05, (u8*)&me->SH36730x0Register.SCONF2.all, 1);
				    break;
				case 3:         //VADC模块使能 设置VADC转换周期(50ms默认) CADC模块使能 设置CADC：连续采集 精度 13bit 电流转换时间62.5ms
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x06, (u8*)&me->SH36730x0Register.SCONF3.all, 1);
					break;
					
			    case 4:         //关闭均衡
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x07, (u8*)&me->SH36730x0Register.SCONF4.all, 1);
				    break;
				case 5:         //关闭均衡
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x08, (u8*)&me->SH36730x0Register.SCONF5.all, 1);
					break;
				case 6:         //设置硬件短路保护延时  硬件短路保护电压设置 复位外部MCU脉冲宽度选择 CADC采集范围设置(200mv)
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x09, (u8*)&me->SH36730x0Register.SCONF6.all, 1);
				    break;
				case 7:         //看门狗溢出时间设置(30s) 充放电状态检测阈值设置  硬件过充保护延时选择
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x0A, (u8*)&me->SH36730x0Register.SCONF7.all, 1);
					break;
			   	case 8:         //硬件过充保护阈值设置
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x0B, (u8*)&me->SH36730x0Register.SCONF8.all, 1);
					break;
			  	case 9:         //硬件过充保护阈值设置 (SCONF8 SCONF9 )*5.86mv
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x0C, (u8*)&me->SH36730x0Register.SCONF9.all, 1);
					break;
				case 10:         //进入低功耗控制位
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x0D, (u8*)&me->SH36730x0Register.SCONF10.PINn, 1);
					break;
			
			 }
            
            QActive_armX((QActive *)me, 0U, 1000U); 
            //DATA_DEBUG[13]++;//==================================================================================
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);           
            
            status = Q_HANDLED();
        }
        break;
        
        case I2C_SH36730x0_FINISH_SIG:  
        {
			//DATA_DEBUG[14]++;//==================================================================================
            QActive_disarmX((QActive *)me, 0U);            
			count_set++;
			
			if(count_set>10)
			{
				//DATA_DEBUG[15]++;//==================================================================================
				count_set=0;
				
				QACTIVE_POST((QActive *)&g_AO_BMS, START_SIG,0);  //SH367730配置成功，发送信号启动BMS对象,查看驱动BMS后会给SH36730对象发布哪些信号来再次配置AFE芯片
                 
				//status = Q_TRAN(&AO_SH36730x0_Idle); //跳转至空闲状态 
                status = Q_TRAN(&AO_SH36730x0_ReadConfig);               
			}
			else status = Q_TRAN(&AO_SH36730x0_WriteParameter);    
				
        }
        break;

        default: 
        {
            status = Q_SUPER(&AO_SH36730x0_Normal);               // 父状态为Normal状态   
        }
        break;
    }
    
    return status;
 }	 

 
 
/*
*配置读取状态函数  暂时没有进行校验只是读取一次
*如有需要可以定时读取校验配置是否有异常
*
*/
 
 u8 read_config=0;
QState AO_SH36730x0_ReadConfig(AO_SH36730x0 * const me) 
{
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {           			         
			switch(read_config) //将写入配置读取一遍做校验
			{
				default:
				case 0:         //充电检测使能 硬件过充保护使能 看门狗使能 使能CTLD管脚优先控制放电MOS
					me->SH36730x0Register.INT_EN.all=0;
				    I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.INT_EN.all,0x03,1);//FLAG1
				    break;
				case 1:         //充电检测使能 硬件过充保护使能 看门狗使能 使能CTLD管脚优先控制放电MOS
					me->SH36730x0Register.SCONF1.all=0;
				    I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.SCONF1.all,0x04,1);
				    break;
			    case 2:         //使能复位外部MCU功能(默认)  设置ALARM硬件持续输出低电平
					me->SH36730x0Register.SCONF2.all=0;
				    I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.SCONF2.all,0x05,1);
				    break;
				case 3:         //VADC模块使能 设置VADC转换周期(50ms默认) CADC模块使能 设置CADC：连续采集 精度 13bit 电流转换时间62.5ms
					me->SH36730x0Register.SCONF3.all=0;
				    I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.SCONF3.all,0x06,1);	
				break;
					
			    case 4:         //关闭均衡
					me->SH36730x0Register.SCONF4.all=0;
				I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.SCONF4.all,0x07,1);
				    break;
				case 5:         //关闭均衡
					me->SH36730x0Register.SCONF5.all=0;
				I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.SCONF5.all,0x08,1);
					break;
				case 6:         //设置硬件短路保护延时  硬件短路保护电压设置 复位外部MCU脉冲宽度选择 CADC采集范围设置(400mv)
					me->SH36730x0Register.SCONF6.all=0;
				I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.SCONF6.all,0x09,1);
				    break;
				case 7:         //看门狗溢出时间设置(30s) 充放电状态检测阈值设置  硬件过充保护延时选择
					me->SH36730x0Register.SCONF7.all=0;
				I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.SCONF7.all,0x0A,1);
					break;
			   	case 8:         //硬件过充保护阈值设置
					me->SH36730x0Register.SCONF8.all=0;
				I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.SCONF8.all,0x0B,1);
					break;
			  	case 9:         //硬件过充保护阈值设置 (SCONF8 SCONF9 )*5.86mv
					me->SH36730x0Register.SCONF9.all=0;
				I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.SCONF9.all,0x0C,1);
					break;
				case 10:         //进入低功耗控制位
					me->SH36730x0Register.SCONF10.PINn=0;
				I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.SCONF10.PINn,0x0D,1);
					break;
			
			 }
            
            QActive_armX((QActive *)me, 0U, 1000U); 
            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);           
            
            status = Q_HANDLED();
        }
        break;
        
        case I2C_SH36730x0_FINISH_SIG:  
        {
			
            QActive_disarmX((QActive *)me, 0U);            
			read_config++;
			
			if(read_config>10)
			{
				
				read_config=0;
				
				QACTIVE_POST((QActive *)&g_AO_BMS, START_SIG,0);  

                status = Q_TRAN(&AO_SH36730x0_Idle);               //跳转至空闲状态
			}
			else status = Q_TRAN(&AO_SH36730x0_ReadConfig);    
				
        }
        break;

        default: 
        {
            status = Q_SUPER(&AO_SH36730x0_Normal);               // 父状态为Normal状态   
        }
        break;
    }
    
    return status;
 }
 
//---------------------------------------------------------------------------------------------------------------
 
 

/*
*   空闲函数  
*/ 
 
QState AO_SH36730x0_Idle(AO_SH36730x0 * const me) 
{ 
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {            
            AO_SH36730x0_Record(me, 4);
					
            // 延时200ms，开始下一次数据读取
      		
            QActive_armX((QActive *)me, 1U,200U);
            status = Q_HANDLED();           
        }
        break;
				
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 1U);                 // 关闭定时器1
			
                                                                
            status = Q_HANDLED();
        }
        break;
		

        case Q_TIMEOUT1_SIG:                                    // 周期定时器1
        {      
			    //DATA_DEBUG[16]++;//==================================================================================
			    QActive_disarmX((QActive *)me, 1U);
			
			
            if (me->State.EnterSHIPModeAsk== 0)            
            {               
				
			    status = Q_TRAN(&AO_SH36730x0_UpdateControl);   // 跳转至控制更新状态  
				//status = Q_TRAN(&AO_SH36730x0_CloseBalance);
            }
            else                                                //进入低功耗模式请求
            {
                status = Q_TRAN(&AO_SH36730x0_EnterSHIP);       // 跳转至进入低功耗状态
            }
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_SH36730x0_Normal);               // 父状态为Normal状态   
        }
        break;
    }
    
    return status;
 }

/*
*   更新控制值，均衡  开关充放电回路
*/

 QState AO_SH36730x0_UpdateControl(AO_SH36730x0 * const me)
{
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            AO_SH36730x0_Record(me, 10);
            
            
			 SH36730x0_UpdateControl(me);//清除系统故障标志位，更新均衡控制及充放电MOS控制值//根据BMS系统标志来控制充放电MOS 及均衡状态
			switch(updata_count)
			{
				case 0:         //充放电MOS控制 更新函数也放这
					
				     me->SH36730x0Register.SCONF2.bit.DSG_C=me->State.DSGControl;
				     me->SH36730x0Register.SCONF2.bit.CHG_C=me->State.CHGControl;
				   
				   //--------------------------------均衡和电压读取时序，修改可设置单次均衡时间--------------------------------------------------			    
				    if((me->State.BatteryBalanceEnable ==1&&((me->SH36730x0Register.SCONF4.all+me->SH36730x0Register.SCONF5.all)>0))||(me->State.test_ForceBatteryBalancePointer>0))
					{
						if(Balance_time<5)//均衡1s内不在更新 不读电压
						{
						  Balance_flag=0;
						  updata_count=3;
						}
						else if(Balance_time==5)//关闭均衡
						{
						 me->SH36730x0Register.SCONF4.all=0x00;
						 me->SH36730x0Register.SCONF5.all=0x00;
						  
						  updata_count=1;
						  Balance_flag=0;
						}
						else if(Balance_time==6)//不写均衡寄存器
						{	  
						 me->SH36730x0Register.SCONF4.all=0x00;
						 me->SH36730x0Register.SCONF5.all=0x00;
						  updata_count=1;
						  Balance_flag=0;
						}

						else if(Balance_time==7)//读电压
                       { 
						  Balance_flag=1;
						  updata_count=3;
						}
					   else if(Balance_time==8)//读电压
                       { 
						  Balance_time=0;
						  Balance_flag=0;
						  updata_count=1;
						}
							
					  Delay_time=0;
					  Balance_time++;
					}
					else
					{  
						Balance_time=0;//关闭均衡
						if(Delay_time<=1)
						{
					    me->SH36730x0Register.SCONF4.all=0x00;
						me->SH36730x0Register.SCONF5.all=0x00;
						Balance_flag=0;	
						updata_count=1;
						}
						else if(Delay_time==2)//不写均衡寄存器
						{
						updata_count=3;
						Balance_flag=0;
						}
						else if(Delay_time==3)//开始读取电压
						{
						updata_count=3;
						Balance_flag=1;
						Delay_time=0;
						}

						Delay_time++;
					}
					//-------------------------------------------------------------------------------------------------------
					I2C_BQ769x0_Write(&g_I2C_SH36730x0, Addr_ZYBQ, 0x05,(u8*)&me->SH36730x0Register.SCONF2.all,1);//更新MOS
					
					break;
					
				case 1:         //均衡控制
					if(Balance_time==7||Balance_time==6||Delay_time==2||Delay_time==3)//关闭均衡
					{
						 me->SH36730x0Register.SCONF4.all=0x00;
						 me->SH36730x0Register.SCONF5.all=0x00;
					}
					I2C_BQ769x0_Write(&g_I2C_SH36730x0, Addr_ZYBQ, 0x07,(u8*)&me->SH36730x0Register.SCONF4.all,1);
				    updata_count=2;
					break;
				case 2:         //均衡控制
					if(Balance_time==7||Balance_time==6||Delay_time==2||Delay_time==3)//关闭均衡
					{
						 me->SH36730x0Register.SCONF4.all=0x00;
						 me->SH36730x0Register.SCONF5.all=0x00;
					}
					I2C_BQ769x0_Write(&g_I2C_SH36730x0, Addr_ZYBQ, 0x08,(u8*)&me->SH36730x0Register.SCONF5.all,1);
					updata_count=3;
					break;

				
				default:
					break;
			}

            //DATA_DEBUG[17]++;//==================================================================================        
            QActive_armX((QActive *)me, 0U, 1000U);             // 开启定时器0,1000ms

            status = Q_HANDLED();
        }
        break;
                                                                
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);                 // 关闭定时器0
            
            status = Q_HANDLED();
        }
        break;
        
        case I2C_SH36730x0_FINISH_SIG:
        {
			 //DATA_DEBUG[18]++;//================================================================================
           QActive_disarmX((QActive *)me, 0U);        
			
					
     if(updata_count==3)
		 {
			//DATA_DEBUG[19]++;//==================================================================================
		    updata_count=0;	 
			status = Q_TRAN(&AO_SH36730x0_CloseBalance);  
			
		 }        
		 else
			status = Q_TRAN(&AO_SH36730x0_UpdateControl);
        }
        break;

        default: 
        {
            status = Q_SUPER(&AO_SH36730x0_Normal);           //父状态为模块通信中状态   
        }
        break;
    }
    
    return status;
 

 }


/*
*  读取电压前关闭均衡  
*/
// u8 close_step=0;

 QState AO_SH36730x0_CloseBalance(AO_SH36730x0 * const me)
 {
	QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {		 
			  
        switch(close_step)
			 {
				case 0://0x02	//系统状态标志读取
				    I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.BSTATUS,0x02,1);//BSTATUS
				    break;
				case 1://0x00	//故障标志读取
					I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.FLAG1,0x00,1);//FLAG1
				    break;
				case 2://AFE内部温度读取 误差较大。。。。。。
					I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.TEMP[0],0x26,1);//0.17*W-270//==============================================
				    break;
				case 3:
					I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.TEMP[1],0x28,1);//0.17*W-270//==============================================
				    break;
				default:
					break;
			  }
			
            QActive_armX((QActive *)me,0U,1000U);
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);
            
            status = Q_HANDLED();
        }
        break;
        
        case I2C_SH36730x0_FINISH_SIG:
        {
			//DATA_DEBUG[20]++;//==================================================================================
            QActive_disarmX((QActive *)me, 0U);
			
            close_step++;			
            if(close_step>3)
			{   
			    //DATA_DEBUG[21]++;//==================================================================================
				close_step=0;
				
				status = Q_TRAN(&AO_SH36730x0_ReadValue);//关闭均衡成功跳转到电压电流读取
			}

			else                                             
				status = Q_TRAN(&AO_SH36730x0_CloseBalance);
        }
        break;

        default: 
        {
            status = Q_SUPER(&AO_SH36730x0_Normal);               // 父状态为Normal状态   
        }
        break;
    }
    
    return status;
 
 }
 
 
/*
*    读取采样值
*/
// u8 Read_times = 0;
 QState AO_SH36730x0_ReadValue(AO_SH36730x0 * const me)
{
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {       
		
            AO_SH36730x0_Record(me, 9);  
			//DATA_DEBUG[28]++;//==================================================================================
			//读取SH36730x0 单体电压数据，0x0EH-0x21H, 只用了8串所以从0x12H开始读取 //一次读2个寄存器
			switch(Read_times)
			{
				//case 0://0x10
				case 0://0x12 
				case 1://0x14
				case 2://0x16
				case 3://0x18
				case 4://0x1A
				case 5://0x1C
				case 6://0x1E
			    case 7://0x20
					if((me->SH36730x0Register.FLAG2.bit.VADC>0)&&Balance_flag==1)//&&Balance_flag==1
					{
						I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.CELL[Read_times],0x12+(Read_times)*2,1);
					 
					}
					else if(me->SH36730x0Register.FLAG2.bit.VADC>0)
					{
						//I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.TEMP[0],0x26,1);
						I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.BSTATUS,0x02,1);//BSTATUS
						Read_times=7;
					}
			         break;

				case 8://0x2A
				     Balance_flag=0;
					 if(me->SH36730x0Register.FLAG2.bit.CADC>0)
					 {
						 
						I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.CUR.CURH,0x2A,1);//读取充放电电流值
					 }
					 else 
					 {
						I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.BSTATUS,0x02,1);//BSTATUS
					 }

			         break;
					
				default:
					break;
			}

            
            QActive_armX((QActive *)me, 0U, 1000U);             // 开启定时器0，1000ms
            status = Q_HANDLED();           
        }
        break;
		   
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);                 // 关闭定时器0
            
            status = Q_HANDLED();
        }
        break;
        
        case I2C_SH36730x0_FINISH_SIG:
        {   
		   //DATA_DEBUG[23]++;//==================================================================================		
            QActive_disarmX((QActive *)me, 0U);                 // 关闭定时器0
            
            me->State.SampleUpdateCount++;
						
            // 若I2C重试次数不为零，允许每10次采样成功减少一次
            // 需确保频繁采样失败时，仍有触发I2C错误的能力。
            if (me->Variable.I2CFaultRetryCount > 0)
            {
                if ((me->State.SampleUpdateCount % 10) == 0)
                {
                    me->Variable.I2CFaultRetryCount--;
                }
            }
             
            //采样值更新函数，故障检测，采样值处理等//传递参数到 Output
			
            //SH36730x0_SampleUpdate(me);   //考虑到底是全部读取完更新还是读一个更新一个
            
            //采样次数++，此值初始为0，最大为30000，超出后重置为100，注意不要小于20（前20次采样电压不做欠压/过压检测）
            me->Variable.SampleUpdateCount++;                   
            if (me->Variable.SampleUpdateCount > 100)
            {
                me->Variable.SampleUpdateCount = 100;
            }                                     
		   Read_times++; 
       if(Read_times>8)
			{
				
				//DATA_DEBUG[24]++;//==================================================================================
			    Read_times = 0;
				
				SH36730x0_SampleUpdate(me);  //更新 
				 QACTIVE_POST((QActive *)&g_AO_BMS, BMS_UPDATE_SIG, 0);
				
				status = Q_TRAN(&AO_SH36730x0_Idle);// 跳转至空闲状态
			}
			
			else status = Q_TRAN(&AO_SH36730x0_ReadValue);//没读完继续读
			
        }
        break;  
        
        default: 
        {
            status = Q_SUPER(&AO_SH36730x0_Normal);
        }
        break;
    }
    
    return status;

 } 
 
/*
*    等待接收EEPROM读取完成的启动信号  
*    接收到START_SIG则继续执行
*    5s内没有结果认定EEPROM读取故障
*    应该在此函数设置TIMEOUT_SIG标签处理超时事件
*/

QState AO_SH36730x0_WaitWriteParameter(AO_SH36730x0 * const me)
 {
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {            
            AO_SH36730x0_Record(me, 321);
            //DATA_DEBUG[11]++;//=================================================================
            // 定时5s，若5s内EEPROM历史故障参数仍未读取完成，则肯定出错了//5s等待时间运行主循环和滴答定时器中断直到主循环运行完时发现5秒超时 时间触发开始执行
            QActive_armX((QActive *)me, 0U, 5000U);//

            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);                 // 关闭定时器0
            
            status = Q_HANDLED();
        }
        break;
        
        case START_SIG: // 收到启动事件，允许配置BQ769x0芯片参数
        {
            QActive_disarmX((QActive *)me, 0U);                 // 关闭定时器0
            //DATA_DEBUG[12]++;//=========================================================
            // 置位系统EEPROM参数已读取标志位（允许写入BQ769x0芯片参数）
            me->Variable.Variable_bits.bit.WriteParameterAllow = 1;
            
            // 跳转至设置系统参数状态
            status = Q_TRAN(&AO_SH36730x0_WriteParameter); 
        }
        break;

        default: 
        {
            status = Q_SUPER(&AO_SH36730x0_Normal);               // 父状态为Normal状态   
        }
        break;
    }
    
    return status;
 
 }			 



/*
*  清除FLAG1故障标志位
*/
 u8 clear_step=0;
 QState AO_SH36730x0_ClearSystemState(AO_SH36730x0 * const me)
 {
	QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            AO_SH36730x0_Record(me,301);
			if(clear_step==0)
			{	
				clear_step=1;
				me->SH36730x0Register.SCONF1.bit.LTCLR=1;// 清除SH36730x0芯片故障信息
			}
			else if(clear_step==1)
			{	clear_step=2;
				me->SH36730x0Register.SCONF1.bit.LTCLR=0;	
			}
            I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x04,(u8*)&me->SH36730x0Register.SCONF1.all, 1);
            
            QActive_armX((QActive *)me, 0U, 1000U);
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);
            
            status = Q_HANDLED();
        }
        break;
        
        case I2C_SH36730x0_FINISH_SIG:
        {
			//DATA_DEBUG[9]++;//==================================================================================
            QActive_disarmX((QActive *)me, 0U);             
            if(clear_step==2)
			{
				clear_step=0;
				status = Q_TRAN(&AO_SH36730x0_ReadParameter);//看故障位是不是已经清除了
			}
			else                                             
				status = Q_TRAN(&AO_SH36730x0_ClearSystemState);
        }
        break;

        default: 
        {
            status = Q_SUPER(&AO_SH36730x0_Normal);               // 父状态为Normal状态   
        }
        break;
    }
    
    return status;
 
 }	


/*
*   进入低功耗模式
*   连续高温中颖AFE芯片会自动复位进入低功耗
*   因为整个系统电源由AFE输出的LDO来控制
*/ 
u8 WDT_FLAG=0;
QState AO_SH36730x0_EnterSHIP(AO_SH36730x0 * const me) 
{ 
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {            
            AO_SH36730x0_Record(me,12);
                      
			//需要升级时需要关闭AFE看门狗及MCU看门狗，再关闭充放电MOS(实际在进入低功耗前MOS会关闭),之后利用软件复位，进入bootloader
			if(g_SystemState.State.bit.EnterBootloaderAsk ==1)
			{	
				
			 switch(WDT_FLAG)
			 {
				default:
                case 0:
                {
                 me->SH36730x0Register.SCONF1.bit.WDT_EN  = 0;//AFE关闭看门狗
                 I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ,0x04,(u8*)&me->SH36730x0Register.SCONF1.all,1);  // NVIC_SystemReset(); //软件复位
				}
				break;
				
				 case 1:
                {
                 me->SH36730x0Register.SCONF2.bit.CHG_C = 0;
				 me->SH36730x0Register.SCONF2.bit.DSG_C = 0;
                 I2C_BQ769x0_Write(&g_I2C_SH36730x0, Addr_ZYBQ, 0x05,(u8*)&me->SH36730x0Register.SCONF2.all,1);//关闭MOS
				}
				break;
			 }  
			}
            else
			{
				switch (SHIP_step) // 进入低功耗,中间连续发送        
				{
					default:
						
					case 0:
					{
					 me->SH36730x0Register.SCONF2.bit.CHG_C = 0;
					 me->SH36730x0Register.SCONF2.bit.DSG_C = 0;
					 I2C_BQ769x0_Write(&g_I2C_SH36730x0, Addr_ZYBQ, 0x05,(u8*)&me->SH36730x0Register.SCONF2.all,1);//关闭MOS
					 
					 SHIP_step = 1;
					}
					break;
					
					case 1:
					{  
					   me->SH36730x0Register.SCONF10.PINn = 0x33;//更新低功耗参数  
					   I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ,0x0D,(u8*)&me->SH36730x0Register.SCONF10.PINn,1);
						
						SHIP_step = 2;
					}
					break;

					case 2:
					{  me->SH36730x0Register.SCONF1.bit.PD_EN = 1;
					   I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ,0x04,(u8*)&me->SH36730x0Register.SCONF1.all,1);	
						
						SHIP_step = 3;
					}
					break;    

				}       
			
		   }
		
            QActive_armX((QActive *)me, 0U, 1000U);// 开启定时器0，1000ms

            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);                 // 关闭定时器0
            QActive_disarmX((QActive *)me, 1U);                 // 关闭定时器1
			
            status = Q_HANDLED();
        }
        break;
        
        case I2C_SH36730x0_FINISH_SIG:                          //如果成功进入低功耗模式设备断电
        {
            QActive_disarmX((QActive *)me, 0U);                 // 关闭定时器0
			
			
			if(g_SystemState.State.bit.EnterBootloaderAsk ==1){ WDT_FLAG++;}
			     
			if(WDT_FLAG==2)
			{
		    	NVIC_SystemReset(); //软件复位		
			}
			
			if (SHIP_step == 3)  
			{   
				Output_LOCK_Update(&g_Output, 0);//关闭MCU电源，设备关机
				SHIP_step=0;
				QActive_armX((QActive *)me, 1U, 10000U);         // 执行成功
	
				//status = Q_TRAN(&AO_SH36730x0_EnterSHIP);
				status = Q_HANDLED();
			}
			else
			{
				
				status = Q_TRAN(&AO_SH36730x0_EnterSHIP); 
			}
			
        }
        break;

		case Q_TIMEOUT_SIG:
		{  
			SHIP_step=0;
            g_SystemState.State.bit.IWDGReloadEnable = 0;//禁止喂狗
            Output_LOCK_Update(&g_Output, 0);//关闭MCU电源，设备关机
            //NVIC_SystemReset(); //MCU
            status = Q_TRAN(&AO_SH36730x0_EnterSHIP);
           // status = Q_HANDLED();
		}
		break;
		
		case I2C_SH36730x0_FAIL_SIG: //设置失败
		{   Output_LOCK_Update(&g_Output, 0);//关闭MCU电源，设备关机
            g_SystemState.State.bit.IWDGReloadEnable = 0;//禁止MCU喂狗
            
		     //NVIC_SystemReset(); 
            
            status = Q_HANDLED();
		}
		break;		
		

        default: 
        {
            status = Q_SUPER(&AO_SH36730x0_Normal);// 父状态
        }
        break;
    }
    
    return status;
}


/*
*   通讯故障,直接进入低功耗状态,最终关闭设备
*/

QState AO_SH36730x0_Fault(AO_SH36730x0 * const me) {
 
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG:           
        {
            AO_SH36730x0_Record(me, 100);

            //置位BQ769x0芯片失败标志位，理论上故障标志位在故障发生处理时已置位，无需此处处理
            Protect_SetFaultCodeLv0(&g_Protect, FAULT_BQ769_FAIL);
            
            QActive_armX((QActive *)me, 0U, 1000U);     // 开启定时器0，1000ms
            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);         // 关闭定时器0
            
            status = Q_HANDLED();
        }
        break;  
        
        case Q_TIMEOUT_SIG:
        {    
			QActive_armX((QActive *)me, 0U, 1000U);     // 开启定时器0，1000ms
			
			me->Variable.FaultEnterSHIPDelay++;
			
            //若收到进入低功耗模式请求，则跳转至进入低功耗状态，否则跳转至正常读取采样值状态
            if (me->State.EnterSHIPModeAsk == 0)                // 进入低功耗模式请求无效//me->State.EnterSHIPModeAsk = 1
            {               
				if (me->Variable.FaultEnterSHIPDelay > 3)		// 延时3s后进入低功耗状态
				{
					status = Q_TRAN(&AO_SH36730x0_EnterSHIP);   // 跳转至进入低功耗状态
				}
				else
				{
					
					status = Q_HANDLED();
				}
            }
            else  // 进入低功耗模式请求无效，但直接执行进入低功耗函数
            {
                status = Q_TRAN(&AO_SH36730x0_EnterSHIP);      
            }			
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);  // 父状态为top状态       
        }
        break;
    }
    
    return status;
}


 
//==========================================================================================================================================// 
u16 soft_uv_filter_cnt0 = 0;
u16 soft_ov_filter_cnt0 = 0;
u16 bq_ov_filter_cnt0 = 0;//电压采样
u16 bq_uv_filter_cnt0 = 0;
u16 bq_ocd_filter_cnt0 = 0;

/*
*   AFE芯片上电配置数据初始化
*   
*/
void SH36730x0_ParameterUpdate(AO_SH36730x0 * const me)
{    
    //中断请求标志位
    me->SH36730x0Register.INT_EN.all = 0x00;//0x6C 110 1100
    
	//系统功能使能设置
	me->SH36730x0Register.SCONF1.bit.LTCLR   = 0;//1->0清除FLAG1
	me->SH36730x0Register.SCONF1.bit.CTLD_EN = 0;//CTLD功能使能控制位	放电MOS控制端//===========================================================关闭
	me->SH36730x0Register.SCONF1.bit.PD_EN   = 0;//低功耗状态控制位
	me->SH36730x0Register.SCONF1.bit.WDT_EN  = 1;//看门狗使能控制位	   任何有效通讯都可以喂狗  溢出(默认30s)时关闭充放电MOS 
    me->SH36730x0Register.SCONF1.bit.SC_EN   = 1;//硬件短路使能保护控制位
	me->SH36730x0Register.SCONF1.bit.OV_EN   = 1;//硬件过充使能保护控制位
	me->SH36730x0Register.SCONF1.bit.CHGR_EN = 0;//充电器检测使能控制位//
	me->SH36730x0Register.SCONF1.bit.LOAD_EN = 0;//负载检测使能控制位//
                                                   //二级保护
	me->SH36730x0Register.SCONF2.bit.RESETorPF = 0;//默认为0 当看门狗溢出后且在50ms内无通讯RESET管脚发出复位信号,1二级保护功能
	me->SH36730x0Register.SCONF2.bit.ALARM_C   = 0;//硬件无连接暂时不管 只输出低电平
	me->SH36730x0Register.SCONF2.bit.DSG_C     = 0;//放电MOS控制位//=====
	me->SH36730x0Register.SCONF2.bit.CHG_C     = 0;//充电MOS控制位//=====
	
    //VADC、VADC 采集配置
	me->SH36730x0Register.SCONF3.bit.VADC_EN = 1;//CADC使能控制位
	me->SH36730x0Register.SCONF3.bit.VADC_C  = 1;//VADC只采集电压
    me->SH36730x0Register.SCONF3.bit.CBIT_C  = 1;//CADC采集精度选择 13 bit
	me->SH36730x0Register.SCONF3.bit.CADC_M  = 1;//CADC采集方式设置为连续采集  
	me->SH36730x0Register.SCONF3.bit.CADC_EN = 1;//VADC使能控制位
	me->SH36730x0Register.SCONF3.bit.SCAN_C  = 1;//VADC转换周期选择位	50ms
	
	//启动时禁止均衡
	me->SH36730x0Register.SCONF4.all = 0x00;   //3~10 8串电池包       
    me->SH36730x0Register.SCONF5.all = 0x00;   
	 
	me->SH36730x0Register.SCONF6.bit.SCVn  = 1;//硬件短路保护电压设置选择位[2:3]   200mv         //0 ->100  1->200 2->300 3->400
	me->SH36730x0Register.SCONF6.bit.RSNSn = 1;//采集范围0~ 200mv   电阻0.5毫欧 电流范围0~400A   //0->400 1->200  2->100 3->50
    me->SH36730x0Register.SCONF6.bit.SCTn  = 0;//硬件短路保护保护延时设置选择位[0:1] 50us	
	me->SH36730x0Register.SCONF6.bit.RSTn  = 3;//复位脉冲时间1S
	  
    me->SH36730x0Register.SCONF7.bit.SHSn  = 0;//充放电状态检测阈值设置位
	me->SH36730x0Register.SCONF7.bit.WDTtn = 1;//看门狗时间设置 0:30s  1:10s 2:2s 3:500ms 默认值就是0//考虑配置成10s
    me->SH36730x0Register.SCONF7.bit.OVTn  = 7;//硬件过充保护延时
	
	me->SH36730x0Register.SCONF8.all  = 0x02;  //硬件过充保护阈值//大于3.8V//========
	me->SH36730x0Register.SCONF9.all  = 0x7F;  //硬件过充保护阈值
	
	me->SH36730x0Register.SCONF10.PINn= 0x00;  //进低功耗控制位


}


/*
*SH36730x0芯片采样值计算更新函数
*包含硬件短路故障，通讯故障及软件短路过流故障触发
*/

//s32 CUR_data_test = 0; // OutputDisableLevel1
//int JR_count=0,all_count=0;
u16 UV_RECHARGE=0;//欠压充电标志
void SH36730x0_SampleUpdate(AO_SH36730x0 * const me)
{   s32 CUR_data = 0; // OutputDisableLevel1
	u16 t_data = 0;
	s16 temp_data=0; //温度转换临时变量
    s32 t_BatteryMaxVoltage = 0;                                // 最高单体电压，单位mV
    s32 t_BatteryMinVoltage = 0;                                // 最低单体电压，单位mV
    u16 t_BatteryMaxVoltagePointer = 0;                         // 最高电压单体编号
    u16 t_BatteryMinVoltagePointer = 0;                         // 最低电压单体编号                                                          
    //u16 t_data1=0,t_data2=0;
	
    u8 i=0;
      

  //  all_count++;//总测试----------------------------------------------------------------------
	
    // ------------------------- FLAG1-2寄存器更新故障信息 -----------------------//注意FLAG2 复位标志位 复位值为1
    if (((me->SH36730x0Register.FLAG1.all & 0xF)) > 0)//+(me->SH36730x0Register.FLAG2.all & 0x7)暂时不加FLAG2
    {
		g_SystemMonitor.System.Reverse.Data[8]++;  
      
        if (me->SH36730x0Register.FLAG1.bit.TWI > 0)//通讯发生超时
        {
            Protect_SetFaultCodeLv0(&g_Protect, FAULT_BQ769_I2C_OVERTIME);
			
			g_SystemMonitor.System.Reverse.Data[11]++;  
        }
        
        //看门狗溢出 ,重启等待一点点时间如果没有恢复正常通讯喂狗就会复位系统，全部重启（包括外部MCU）
        if (me->SH36730x0Register.FLAG1.bit.WDT > 0)
        {   //如果设备重启没必要发送故障码,
            Protect_SetFaultCodeLv0(&g_Protect, FAULT_BQ769_OVRD_ALERT);
		    g_SystemMonitor.System.Reverse.Data[12]++;  
        }
        
		    // SH36730x0 过充电保护 硬件
        if (me->SH36730x0Register.FLAG1.bit.OV > 0)//任意一个电芯电压大于设置阈值触发该标志
        {
			if (me->Variable.SampleUpdateCount > 20)//忽略启动前20次防止误触
            {
                Protect_SetFaultCodeLv1(&g_Protect, FAULT_BQ769_OV);
            }
			g_SystemMonitor.System.Reverse.Data[10]++;  
        }  

           // SH36730x0 放电回路短路 硬件
        if (me->SH36730x0Register.FLAG1.bit.SC > 0)
        {
            Protect_SetFaultCodeLv1(&g_Protect, FAULT_BQ769_SCD);
			
			g_SystemMonitor.System.Reverse.Data[10]++;  //g_SystemMonitor.System.Reverse.Data[15]
        }       

    }
//==============================================电流处理====================================================================================	
 
// 临时措施，每次都根据参数计算一次电流放大比例，实现电流比例实时更改功能	 //比例值会有小数所以直接扩大1000倍取小数处理
	me->Parameter.CCGain = (s32)g_SystemParameter.System.Calibration.CurrentSensorMaxCurrent*1000 /g_SystemParameter.System.Calibration.CurrentSensorMaxVoltage;//当做采样阻值倒数
	me->Parameter.CCOffset = 0;

	 if(me->SH36730x0Register.FLAG2.bit.CADC > 0)//CADC转换完成,读取后硬件清零
	 { 
		 me->SH36730x0Register.FLAG2.bit.CADC=0;
		 me->SH36730x0Register.FLAG2.bit.VADC=0;
		 
		 
		// JR_count++;//测试-------------------------------------------
		 
		 
		if(me->SH36730x0Register.CUR.CURH&0x10)//放电                                          
		{                                                                                      //R采样电阻 阻值为2个1mR并联=0.5mR=0.0005R=5/10000R计算时要换算为欧姆计算
			CUR_data=((me->SH36730x0Register.CUR.CURH<<8)+me->SH36730x0Register.CUR.CURL)|0xFFFFE000; 
			
			//CUR_data=(1000000*CUR_data)/8192;
			
			//CUR_data=(1000*CUR_data)/(16384/me->Parameter.CCGain);                                                                                  //0~400mV 公式 I=(1000*CUR)/(8192*R) //mA   //化简公式t_data=(1000000*t_data)/4096;  //0~1000A 精度+-244mA
            CUR_data=(125*CUR_data*me->Parameter.CCGain)/2048;                                                                                   //0~200mV 公式 I=(1000*CUR)/(16384*R)//mA  //化简公式t_data=(1000000*t_data)/8192;  //0~500A  精度+-122mA
		    me->Output.BatteryCurrent=CUR_data;	                                              //0~100mv 公式 I=(1000*CUR)/(32768*R)//mA  //化简公式t_data=(1000000*t_data)/16384; //0~250A  精度+-61mA
		}                                                                                     //0~50mv 公式 I=(1000*CUR)/(65536*R) //mA  //化简公式t_data=(1000000*t_data)/32768;  //0~125A  精度+-31mA
		else                                                                                  //充电                                           
		{	
			CUR_data=((me->SH36730x0Register.CUR.CURH<<8)+me->SH36730x0Register.CUR.CURL)&0xFFF;
			
			//CUR_data=(1000000*CUR_data)/8192;
			
			//CUR_data=(1000*CUR_data)/((16384)/(me->Parameter.CCGain*1000));
			CUR_data=(125*CUR_data*me->Parameter.CCGain)/2048;
			
			me->Output.BatteryCurrent=CUR_data;	
		}
		
//=========================================================================================
		// To be update: 电池0电流附近最好做一个死区，利于电流0偏导致空闲时电量变化过快。
        // 死区电流大小根据电池电量来设定，例如8000mAH电池，设死区电流为8mA左右，即0.1%
        if (CUR_data < 280 && CUR_data > -280)
        {
            me->Output.BatteryCurrent = 0;
        }
        else
        {
            me->Output.BatteryCurrent = CUR_data;
        }
	
	    
//=========================================================================================		
		if((me->SH36730x0Register.CUR.CURH & 0x10) && ((-me->Output.BatteryCurrent)> 2000))//放电大于2A时均衡不开启，注意刹车时电流反向充，所以再次开启时要进行十秒判断(BMS.c L:694)
		{
		  me->State.BatteryBalanceEnable =0;                                               //关闭均衡
		}
		
		//放电电池短路                                                                         380A*1000  = 380000mA
        if((me->SH36730x0Register.CUR.CURH&0x10)&&((-me->Output.BatteryCurrent)> (g_SystemParameter.BMS.Battery.ShortCutDownCurrent*1000)))
		{
             if(bq_ocd_filter_cnt0++ > 5) Protect_SetFaultCodeLv1(&g_Protect,FAULT_BQ769_SCD);//短路故障
		}
        //放电过流设置                                                                  //阈值150A	*1000=150000   
        else if ((me->Output.BatteryCurrent<0)&&((-me->Output.BatteryCurrent)> (g_SystemParameter.BMS.Battery.OverCutDownCurrent*1000))) 
		{ 
				                                                    
          if(bq_ocd_filter_cnt0++ > 40) Protect_SetFaultCodeLv1(&g_Protect,FAULT_BQ769_OCD);//超过5次发送故障 
			
		  g_SystemMonitor.System.Reverse.Data[9]++;  
			
        }//充电电流限制 20A    
		else if((me->Output.BatteryCurrent>0)&&((me->Output.BatteryCurrent)> ( g_SystemParameter.BMS.Charge.ChargeOverCurrent*1000)))
		{
		
		  if(bq_ocd_filter_cnt0++ > 100) Protect_SetFaultCodeLv1(&g_Protect,FAULT_BAT_CHARGE_OVER_CURRENT);//充电过流设置
		
		}
		else 
		{
		  bq_ocd_filter_cnt0 = 0;
        }
    }
//============================================电流处理结束=======================================================================================



//=============================================温度处理========================================================	

	temp_data=me->SH36730x0Register.TEMP[0].TEMPnH & 0xF;                //0.17*temp-270
    temp_data=temp_data<<8;
    temp_data+=me->SH36730x0Register.TEMP[0].TEMPnL;
	
	temp_data=temp_data*17-27000;
	me->Output.InnerTemperature[0]=temp_data/10;
	
	temp_data=me->SH36730x0Register.TEMP[1].TEMPnH & 0xF;                //0.17*temp-270
    temp_data=temp_data<<8;
    temp_data+=me->SH36730x0Register.TEMP[1].TEMPnL;
	
	temp_data=temp_data*17-27000;
	me->Output.InnerTemperature[1]=temp_data/10;
		 
	 
	
//======================================= 电压处理===============================================================================================
	me->Output.BatteryVoltage=0;//电池总电压
	me->Variable.CellNumberPointer = 0;//已计算单体电压数量
//===========================================================================================================================	
	    for (i = 0; i < 8; i++)        //电压从CEEL3开始读取   
    {                                     
        if (me->Parameter.CellSelect[i] == 1)   //CellSelect8串初始值做了修改,对应硬件连接AFE的VC3~VC10对应  1~8串 
        {
            t_data = me->SH36730x0Register.CELL[i].CELLnH &0xF; //高四位
            t_data = t_data << 8;
            t_data += me->SH36730x0Register.CELL[i].CELLnL;     //低八位
                                                    
            me->Output.SingleVoltage[me->Variable.CellNumberPointer] = (t_data*6*1000)/4096 ;
		    //单体电压求和计算总电压
            me->Output.BatteryVoltage += me->Output.SingleVoltage[me->Variable.CellNumberPointer];
			
            me->Variable.CellNumberPointer++; 
        }
		
        if (me->Variable.CellNumberPointer >= me->Parameter.CellNumber)
        {
			 me->Output.SingleVoltage[8] =  me->Output.SingleVoltage[0] ;//=================测试用,一直把第一串电压显示在第九串
            break;          //读取完实际电芯数 跳出for循环
        }
    }
//===========================大电流第一串电压过滤处理==============================================================		
		if((me->SH36730x0Register.CUR.CURH & 0x10) && ((-me->Output.BatteryCurrent)> 50000))//放电电缆大于50A时过滤第一串电压用VC6	
		{
		 me->Output.SingleVoltage[0] =  me->Output.SingleVoltage[5] ;//用第六串电压代替第一串电压
		
		}
		
//===========================================================================================

    // ---------- 寻找电池组单体最低最高电压值 -----------------------------
    
    t_BatteryMaxVoltage = me->Output.SingleVoltage[0];
    t_BatteryMinVoltage = me->Output.SingleVoltage[0];
    t_BatteryMaxVoltagePointer = 0;
    t_BatteryMinVoltagePointer = 0; 
    
    for (i = 1; i < me->Parameter.CellNumber; i++)// 遍历整个电池组，寻找最高最低电压
    {
         if (me->Output.SingleVoltage[i] > t_BatteryMaxVoltage)
         {
             t_BatteryMaxVoltage = me->Output.SingleVoltage[i];
             t_BatteryMaxVoltagePointer = i;
         }
         
         if (me->Output.SingleVoltage[i] < t_BatteryMinVoltage)
         {
             t_BatteryMinVoltage = me->Output.SingleVoltage[i];
             t_BatteryMinVoltagePointer = i;
         }       
    }
    
    // 将计算结果赋值给输出变量结构体，注意对应第几串为从下往上，实际第几串，例如第1，2，3......串
    me->Output.SingleMaxVoltage = t_BatteryMaxVoltage;//最高电压值
    me->Output.SingleMaxVoltagePointer = t_BatteryMaxVoltagePointer + 1;//串号
   
    me->Output.SingleMinVoltage = t_BatteryMinVoltage;//最低电压值
    me->Output.SingleMinVoltagePointer = t_BatteryMinVoltagePointer + 1;//串号       
	
	
//------------------------------软件保护 -------------------------------------------

	//判断单体最低电压是否小于单体欠压限制值(mv)     欠压判断
    if ((me->Output.SingleMinVoltage < g_SystemParameter.BMS.Battery.CellUnderVoltage))//2400mv
    {    
        g_SystemMonitor.System.Reverse.Data[7]++;
//======================================================================

        if ((me->Variable.SampleUpdateCount > 20)) //过滤上电初次采集的20次数据，连续欠压40次设置欠压故障，约8秒
        {
            g_Protect.Variable.Count.UnderVoltageCount ++;    //电池欠压延时
            if(g_Protect.Variable.Count.UnderVoltageCount > (g_SystemParameter.BMS.Protect.CellUnderVoltageDelay*5))
            {
                Protect_SetFaultCodeLv1(&g_Protect, FAULT_BQ769_UV);//设置欠压故障
            }
        }
	} else {
        g_Protect.Variable.Count.UnderVoltageCount = 0;
    }
    
                                       
    //电池单体过压
    if (me->Output.SingleMaxVoltage > g_SystemParameter.BMS.Battery.CellOverVoltage)
    {   
        if (me->Variable.SampleUpdateCount > 20 && soft_ov_filter_cnt0++ > 40)//考虑加入过压延时
        {
            Protect_SetFaultCodeLv1(&g_Protect, FAULT_BQ769_OV);//设置过压故障
        }
    } 
	else { soft_ov_filter_cnt0 = 0;}


	               
	
    //数据更新次数++，用于标识读取时间，用于电量计算
    me->Variable.ReadValueCount++;
    me->Output.ReadValueTime = g_SystemState.Output.System1msInterruptCount;//1ms时基中断计数 //计算更新时间  	
}


// ------------------------- 定时更新控制量，电池均衡，充放电回路 --------------------------------
void SH36730x0_UpdateControl(AO_SH36730x0 * const me)
{
    // 清除系统状态寄存器
    // 出现故障，对应标志位会置位，直接写入1则清除该标志位
    // 故障位自动清除，读取采样值时，若故障位被置位，则记录故障。此处仅清除对应故障位为1的故障！
    //me->SH36730x0Register.SCONF1.bit.LTCLR = 1; //1->连续配置才能清除FLAG1标志位

    // --------------------------- 根据单体最低最高电压，配置均衡寄存器 ---------------------------------

    // 更新前先清零一下
    me->SH36730x0Register.SCONF4.all = 0x00; //3~10 8串电池包       
    me->SH36730x0Register.SCONF5.all = 0x00;   

	if (me->State.test_ForceBatteryBalancePointer == 0)	//测试用强制指定均衡对象未使能，则正常运行
	{
		if (me->State.BatteryBalanceEnable == 1)// 使能均衡
		{
			// 默认电池单体电压最低最高偏差超过设定值，启动均流
			if ((me->Output.SingleMaxVoltage - me->Output.SingleMinVoltage) > me->Parameter.BallanceErrVoltage)     
			{
				me->State.BatteryBalancing = 1; // 均衡模块开始工作
				me->State.BatteryBalancePointer = me->Output.SingleMaxVoltagePointer;// 被均衡单体编号
				

			switch (me->Output.SingleMaxVoltagePointer)
		    {
			case 8:
			{
				me->SH36730x0Register.SCONF4.bit.CB10 = 1;
			}
			break;
			
			case 7:
			{
				me->SH36730x0Register.SCONF4.bit.CB9 = 1;
			}
			break;
								
			case 6:
			{
				me->SH36730x0Register.SCONF4.bit.CB8 = 1;
			}
			break;      

			case 5:
			{
				me->SH36730x0Register.SCONF4.bit.CB7 = 1;
			}
			break;
			
			case 4:  
			{
				me->SH36730x0Register.SCONF4.bit.CB6 = 1;
			}
			break;
								
			case 3:
			{
				me->SH36730x0Register.SCONF5.bit.CB5 = 1;
			}
			break;      

			case 2:
			{
				me->SH36730x0Register.SCONF5.bit.CB4 = 1;
			}
			break;    

			case 1:
			{
				me->SH36730x0Register.SCONF5.bit.CB3 = 1;
			}
			break;  							
		   }
	     }
	    else
	    {
		  // 单体电压不均衡度未超过限制，不使能均流
		  me->State.BatteryBalancing = 0;
		  me->State.BatteryBalancePointer = 0;            
	    }   
	   }
	   else //均衡使能标志未置位 BatteryBalanceEnable
	   {

		 me->State.BatteryBalancing = 0;//均衡状态标志位 0未开启
		 me->State.BatteryBalancePointer = 0;        
	   }
	}
	else    //强制开启均衡 测试用代码
	{
		me->State.BatteryBalancing = 1;                     // 均流模块开始工作
		me->State.BatteryBalancePointer = me->State.test_ForceBatteryBalancePointer; // 被均流单体编号
		
		// 强制指定均衡对象使能
		// 此处由于BQ769x0芯片电池连接比较特别，需特殊对待，根据不同串数，决定电池均流寄存器配置

		switch (me->State.test_ForceBatteryBalancePointer)
		{
			case 8:
			{
				me->SH36730x0Register.SCONF4.bit.CB10 = 1;
			}
			break;
			
			case 7:
			{
				me->SH36730x0Register.SCONF4.bit.CB9 = 1;
			}
			break;
								
			case 6:
			{
				me->SH36730x0Register.SCONF4.bit.CB8 = 1;
			}
			break;      

			case 5:
			{
				me->SH36730x0Register.SCONF4.bit.CB7 = 1;
			}
			break;
			
			case 4:  
			{
				me->SH36730x0Register.SCONF4.bit.CB6 = 1;
			}
			break;
								
			case 3:
			{
				me->SH36730x0Register.SCONF5.bit.CB5 = 1;
			}
			break;      

			case 2:
			{
				me->SH36730x0Register.SCONF5.bit.CB4 = 1;
			}
			break;    

			case 1:
			{
				me->SH36730x0Register.SCONF5.bit.CB3 = 1;
			}
			break;  

//        if(me->SH36730x0Register.SCONF4.all>0)	
//		{
//		I2C_BQ769x0_Write(&g_I2C_SH36730x0, Addr_ZYBQ, 0x07,(u8*)&me->SH36730x0Register.SCONF4.all,1);
//			me->SH36730x0Register.SCONF4.all=0;
//		}
//         if(me->SH36730x0Register.SCONF5.all>0)	
//		{
//		I2C_BQ769x0_Write(&g_I2C_SH36730x0, Addr_ZYBQ, 0x08,(u8*)&me->SH36730x0Register.SCONF5.all,1);
//			me->SH36730x0Register.SCONF5.all=0;
//		}			

		
		}

	}

	// 控制充放电mosfet
    if (Protect_GetFaultCode(&g_Protect) == 0)    // 无故障发生
    {
        // 电池充电控制使能
        if (me->State.CHGControl == 1)
        {
            me->SH36730x0Register.SCONF2.bit.CHG_C = 1;
        }
        else
        {
            me->SH36730x0Register.SCONF2.bit.CHG_C = 0;
        }
        
        // 电池放电控制使能
        if (me->State.DSGControl == 1)
        {
           me->SH36730x0Register.SCONF2.bit.DSG_C = 1;		
        }
        else
        {
           me->SH36730x0Register.SCONF2.bit.DSG_C = 0;
        }       
    }
    else
    {
        //当有故障发生时，强制关闭输出
        me->SH36730x0Register.SCONF2.bit.CHG_C = 0;
        me->SH36730x0Register.SCONF2.bit.DSG_C = 0;       
    }      
}


// End of AO_SH367930x0.c


 
