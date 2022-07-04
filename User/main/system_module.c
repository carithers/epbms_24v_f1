/* ==================================================================================

 File name:     system_module.c
 Originator:    BLJ
 Description:   ϵͳ������ģ��

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 04-1-2015     Version 0.0.1           ���Թ���
-----------------------------------------------------------------------------------*/

#include "system_core.h"   //ϵͳ����ͷ�ļ�
#include "system_module.h"


u16	t_ChargeStateDelay = 0;
u16	t_LED_TEST_Delay = 0;
u16 t_LEDDelay = 0;
u16 t_FAULTDelay = 0;
// ---------------------------------- LED��ˢ�º�������������1ms ---------------------------------
void System_LED_Update(void)
{
#if (CONTROLLER_TARGET == BMS_EP_A5 || BMS_EP_20_REV1_1_2TEMP || BMS_EP_ALPHA)

    // 2017-09-14
    // ��������·�����������һյ�ƣ���ơ������߼����£�
    // ����״̬����������2s
    // ����Ƿѹ����1�£�ʱ��СһЩ�� ͣ2S ��2�� ͣ3S��ѭ��
    // �����ѹ����1�£�ʱ��СһЩ�� ͣ2S ��3�� ͣ3S��ѭ��
    // ����¶ȹ��ߣ���1�£�ʱ��СһЩ�� ͣ2S ��4�� ͣ3S��ѭ��
    // ����¶ȹ��ͣ���1�£�ʱ��СһЩ�� ͣ2S ��5�� ͣ3S��ѭ��
    // ������϶���ɫ����,����0.4s  
    if(g_Input.Output.DIN_bits.bit.KEY > 0)
    {
        if (Protect_GetFaultCode(&g_Protect) > 0)
        {
            switch (Protect_GetFaultCode(&g_Protect))
            {
                case FAULT_BQ769_UV:
                {
                    Output_LED_BLUE_Update(&g_Output, 0);
                    Output_LED_YELLOW_Update(&g_Output, 0);
                    g_LED_Flicker.Input.LEDLength1 = 1;
                    g_LED_Flicker.Input.LEDLength2 = 2;
                    LED_Flicker_Update(&g_LED_Flicker);	                
                }
                break;
                
                case FAULT_BQ769_OV:
                {
                    Output_LED_BLUE_Update(&g_Output, 0);
                    Output_LED_YELLOW_Update(&g_Output, 0);
                    g_LED_Flicker.Input.LEDLength1 = 1;
                    g_LED_Flicker.Input.LEDLength2 = 3;
                    LED_Flicker_Update(&g_LED_Flicker);	                
                }
                break;    
                
                case FAULT_BQ769_SCD     :
                {
                    Output_LED_BLUE_Update(&g_Output, 0);
                    Output_LED_YELLOW_Update(&g_Output, 0); 
                    g_LED_Flicker.Input.LEDLength1 = 1;
                    g_LED_Flicker.Input.LEDLength2 = 4;
                    LED_Flicker_Update(&g_LED_Flicker);	
                }
                break;
               
                case FAULT_BQ769_OCD:
                {
                    Output_LED_BLUE_Update(&g_Output, 0);
                    Output_LED_YELLOW_Update(&g_Output, 0);
                    g_LED_Flicker.Input.LEDLength1 = 1;
                    g_LED_Flicker.Input.LEDLength2 = 5;
                    LED_Flicker_Update(&g_LED_Flicker);	            
                }
                break;

                case FAULT_BAT_OT:
                {
                    Output_LED_BLUE_Update(&g_Output, 0);
                    Output_LED_YELLOW_Update(&g_Output, 0);
                    g_LED_Flicker.Input.LEDLength1 = 2;
                    g_LED_Flicker.Input.LEDLength2 = 3;
                    LED_Flicker_Update(&g_LED_Flicker);	                
                }
                break;  

                case FAULT_BAT_UT:
                {
                    Output_LED_BLUE_Update(&g_Output, 0);
                    Output_LED_YELLOW_Update(&g_Output, 0);
                    g_LED_Flicker.Input.LEDLength1 = 2;
                    g_LED_Flicker.Input.LEDLength2 = 4;
                    LED_Flicker_Update(&g_LED_Flicker);	                
                }
                break;  

                default:
                {
                    Output_LED_BLUE_Update(&g_Output, 0);
                    Output_LED_GREEN_Update(&g_Output, 0);
                    t_LED_TEST_Delay++;
                    if (t_LED_TEST_Delay < 500)
                    {
                        Output_LED_YELLOW_Update(&g_Output, 0);
                    }
                    else if (t_LED_TEST_Delay < 1000)
                    {
                        Output_LED_YELLOW_Update(&g_Output, 1);					
                    }
                    else
                    {
                        t_LED_TEST_Delay = 0;
                    }	                
                }
                break;
            }
            
        }
        else
        {
    		// Ŀǰû��ͨѶȷ�ϣ���ʱ���ݵ����жϳ�ŵ�״̬����������Ϊ������������5s���ϣ���Ϊ������״̬
    		
    		if (g_AO_SH36730x0.Output.BatteryCurrent > 500)			// ����������100mA
    		{
    			if (t_ChargeStateDelay < 11000)
    			{
    				t_ChargeStateDelay++;
    			}
    		}
    		else
    		{
    			t_ChargeStateDelay = 0;
    		}
    		
    		if (t_ChargeStateDelay > 10000)
    		{
    			// ���״̬
    			// ���ʱ������������100%���̵���
    			if (g_AO_BMS.Output.SOC >= 1000)
    			{
    				Output_LED_GREEN_Update(&g_Output, 1);
    				Output_LED_BLUE_Update(&g_Output, 0);		// ��ɫ�ݶ�Ϊ��ɫ
    				Output_LED_YELLOW_Update(&g_Output, 0);
    			}
    			else
    			{
    				Output_LED_GREEN_Update(&g_Output, 0);
    				Output_LED_BLUE_Update(&g_Output, 1);		// ��ɫ�ݶ�Ϊ��ɫ
    				Output_LED_YELLOW_Update(&g_Output, 0);				
    			}			
    		}
    		else
    		{
    			// �ŵ�״̬
    			// �ŵ�ʱ������С��15%�����2s��˸
    			// ����С��30%���Ƶ���
    			// ��������30%���̵���
    			if (g_AO_BMS.Output.SOC < g_SystemParameter.BMS.Warning.BDILowLimitSpeed * 10)
    			{
    				t_LEDDelay++;
    				if (t_LEDDelay < 500)
    				{
    					Output_LED_GREEN_Update(&g_Output, 0);
    					Output_LED_BLUE_Update(&g_Output, 0);		// ��ɫ�ݶ�Ϊ��ɫ
    					Output_LED_YELLOW_Update(&g_Output, 0);
    				}
    				else if (t_LEDDelay < 1000)
    				{
    					Output_LED_GREEN_Update(&g_Output, 0);
    					Output_LED_BLUE_Update(&g_Output, 1);		// ��ɫ�ݶ�Ϊ��ɫ
    					Output_LED_YELLOW_Update(&g_Output, 0);					
    				}
    				else
    				{
    					t_LEDDelay = 0;
    				}				
    			}
                else if(g_AO_BMS.Output.SOC < g_SystemParameter.BMS.Warning.BDILowLevel2Percent * 10)
                {
    				t_LEDDelay++;
    				if (t_LEDDelay < 1000)
    				{
    					Output_LED_GREEN_Update(&g_Output, 0);
    					Output_LED_BLUE_Update(&g_Output, 0);		// ��ɫ�ݶ�Ϊ��ɫ
    					Output_LED_YELLOW_Update(&g_Output, 0);
    				}
    				else if (t_LEDDelay < 2000)
    				{
    					Output_LED_GREEN_Update(&g_Output, 0);
    					Output_LED_BLUE_Update(&g_Output, 1);		// ��ɫ�ݶ�Ϊ��ɫ
    					Output_LED_YELLOW_Update(&g_Output, 0);					
    				}
    				else
    				{
    					t_LEDDelay = 0;
    				}

                }
    			else if (g_AO_BMS.Output.SOC < g_SystemParameter.BMS.Warning.BDILowLevel1Percent * 10)
    			{
    				Output_LED_GREEN_Update(&g_Output, 0);
    				Output_LED_BLUE_Update(&g_Output, 0);		// ��ɫ�ݶ�Ϊ��ɫ
    				Output_LED_YELLOW_Update(&g_Output, 1);				
    			}				
    			else
    			{

    				Output_LED_GREEN_Update(&g_Output, 1);
    				Output_LED_BLUE_Update(&g_Output, 0);		// ��ɫ�ݶ�Ϊ��ɫ
    				Output_LED_YELLOW_Update(&g_Output, 0);					
    			}
                
    		}
    	}
    }
    else
    {
    	Output_LED_GREEN_Update(&g_Output, 0);
    	Output_LED_BLUE_Update(&g_Output, 0);		// ��ɫ�ݶ�Ϊ��ɫ
    	Output_LED_YELLOW_Update(&g_Output, 0);	        
    }

#endif					
	
}

u16	t_I2CFailCount = 0;

// --------------------- BQ769x0 I2Cͨ��״̬������¼����ͺ�������������ѭ���� ----------------------
void I2C_BQ769x0_SIG_Update(void)
{
	// ģ�����,���͹����¼���AO_EEPROM״̬��
	if (g_I2C_SH36730x0.State.Fault_bits.all > 0)							
	{		
        g_SystemMonitor.System.Reverse2.Data[3] = g_I2C_SH36730x0.State.Fault_bits.all;
        
        if (g_I2C_SH36730x0.State.Fault_bits.bit.OverTimeFlag1 > 0)
        {
            g_SystemMonitor.System.Reverse2.Data[4]++;//��¼ͨ��ʧ�ܴ���������ԭ����������
        }
        
        if (g_I2C_SH36730x0.State.Fault_bits.bit.OverTimeFlag2 > 0)
        {
            g_SystemMonitor.System.Reverse2.Data[5]++;
        }
        
        if (g_I2C_SH36730x0.State.Fault_bits.bit.OverTimeFlag3 > 0)
        {
            g_SystemMonitor.System.Reverse2.Data[6]++;
        }
        
        if (g_I2C_SH36730x0.State.Fault_bits.bit.OverTimeFlag4 > 0)
        {
            g_SystemMonitor.System.Reverse2.Data[7]++;
        }
        
        if (g_I2C_SH36730x0.State.Fault_bits.bit.OverTimeFlag5 > 0)
        {
            g_SystemMonitor.System.Reverse2.Data[8]++;
        }
        
        if (g_I2C_SH36730x0.State.Fault_bits.bit.OverTimeFlag6 > 0)
        {
            g_SystemMonitor.System.Reverse2.Data[9]++;
        }
        
        if (g_I2C_SH36730x0.State.Fault_bits.bit.OverTimeFlag7 > 0)
        {
            g_SystemMonitor.System.Reverse2.Data[10]++;
        }
        
        if (g_I2C_SH36730x0.State.Fault_bits.bit.OverTimeFlag8 > 0)
        {
            g_SystemMonitor.System.Reverse2.Data[11]++;
        }
        
        if (g_I2C_SH36730x0.State.Fault_bits.bit.OverTimeFlag9 > 0)
        {
            g_SystemMonitor.System.Reverse2.Data[12]++;
        }
        
        if (g_I2C_SH36730x0.State.Fault_bits.bit.OverTimeFlag10 > 0)
        {
            g_SystemMonitor.System.Reverse2.Data[13]++;
        }
        
		QACTIVE_POST((QActive *)&g_AO_SH36730x0, I2C_SH36730x0_FAIL_SIG, 0);//����ͨѶʧ���ź�//Ӧ�����г�ʱ
		
		t_I2CFailCount++;
		
		g_I2C_SH36730x0.State.Fault_bits.all = 0; //v->State.Fault_bits.bit.ByteSendAckHi = 1;//Ӧ��ʧ����λΪ1 
	}
	
	// EEPROMָ�����ɣ�����EEPROMָ������¼���AO_EEPROM�����
	if (g_I2C_SH36730x0.State.State_bits.bit.CommandFinish > 0)  // ��дָ��������ɣ��ڲ���λ�����ⲿ���
	{
		if (g_I2C_SH36730x0.State.State_bits.bit.CRCCheckFail == 0)
		{
			QACTIVE_POST((QActive *)&g_AO_SH36730x0, I2C_SH36730x0_FINISH_SIG, 0);	
		}
		else
		{
			QACTIVE_POST((QActive *)&g_AO_SH36730x0, I2C_SH36730x0_FAIL_SIG, 0);	//���㴦���Ӧ��ʧ�ܺ�У��ʧ��ͳһΪͨ��ʧ��
			
			g_I2C_SH36730x0.State.State_bits.bit.CRCCheckFail = 0;
		}
		
		g_I2C_SH36730x0.State.State_bits.bit.CommandFinish = 0;
	}
}


// -------------------------- flashģ��EEPROMģ���¼��������������ڲ���״̬����Ҫ���¼� ------------------------
void flash_EEPROM_SIG_Update(void)
{
	// flashģ��EEPROMģ�����,���͹����¼���AO_EEPROM״̬��
	if (g_flash_EEPROM.State.Fault_bits.all > 0)							
	{		
		// for test
		if (g_flash_EEPROM.State.Fault_bits.bit.FlashWriteCheckFail > 0)
		{
			// Lv1: flashд�����ݼ�����
			Protect_SetFaultCodeLv0(&g_Protect, EEPROM_PARAMETER_WRONG);
		}
		
		if (g_flash_EEPROM.State.Fault_bits.bit.FlashWriteForbid > 0)
		{
			// Lv1: ��д������Ϊ0xFFFF����ֹд��
			Protect_SetFaultCodeLv0(&g_Protect, FLASH_WRITE_FORBID);
		}
		
		if (g_flash_EEPROM.State.Fault_bits.bit.FlashWriteFail > 0)
		{
			// Lv1: д��ʧ��
			Protect_SetFaultCodeLv0(&g_Protect, FLASH_WRITE_FAIL);
		}		
		
		QACTIVE_POST((QActive *)&g_AO_EEPROM, EEPROM_FAIL_SIG, 0);	
		
		g_flash_EEPROM.State.Fault_bits.all = 0;
	}
	
	// EEPROMָ�����ɣ�����EEPROMָ������¼���AO_EEPROM�����
	if (g_flash_EEPROM.State.State_bits.bit.CommandFinish > 0)
	{
		if (g_flash_EEPROM.State.State_bits.bit.LRCCheckFail == 0)
		{
			QACTIVE_POST((QActive *)&g_AO_EEPROM, EEPROM_FINISH_SIG, 0);	
		}
		else
		{
			QACTIVE_POST((QActive *)&g_AO_EEPROM, EEPROM_LRC_FAIL_SIG, 0);	
			
			g_flash_EEPROM.State.State_bits.bit.LRCCheckFail = 0;
		}
		
		g_flash_EEPROM.State.State_bits.bit.CommandFinish = 0;
	}	
}


// ------------------------- ϵͳ״̬�������º�����1s����1�� ----------------------------------
void SystemMonitor_Update(void)
{
	u16	i = 0;
	
	g_SystemMonitor.BMS.Battery.Voltage = g_AO_SH36730x0.Output.BatteryVoltage / 100;			// ������ѹ����λ0.1V
	
	g_SystemMonitor.BMS.Battery.Current = g_AO_SH36730x0.Output.BatteryCurrent / 100;			// ������������λ0.1A
	g_SystemMonitor.BMS.Battery.CurrentCali = g_AO_SH36730x0.Output.BatteryCurrent % 100;		// ������ܵ���У׼����λ0.001A����Χ0-0.099A


	g_SystemMonitor.BMS.Battery.Capacity = g_AO_BMS.Output.BatteryCapacity / 100;			// �����ʣ���������λ0.1AH
	g_SystemMonitor.BMS.Battery.CapacityCali = g_AO_BMS.Output.BatteryCapacity % 100;		// �����ʣ�����У׼����λ0.001AH����Χ0-0.099AH

	g_SystemMonitor.BMS.Battery.SOC = g_AO_BMS.Output.SOC;									// �����ʣ������ٷֱ�
	
	// �������/��͵�ѹ
	g_SystemMonitor.BMS.Battery.CellMaxVoltage = g_AO_SH36730x0.Output.SingleMaxVoltage;
	g_SystemMonitor.BMS.Battery.CellMaxVoltagePointer = g_AO_SH36730x0.Output.SingleMaxVoltagePointer;
	g_SystemMonitor.BMS.Battery.CellMinVoltage = g_AO_SH36730x0.Output.SingleMinVoltage;
	g_SystemMonitor.BMS.Battery.CellMinVoltagePointer = g_AO_SH36730x0.Output.SingleMinVoltagePointer;	
	/*
	// ����¶�
	g_SystemMonitor.BMS.Battery.Temperature[0] = g_AO_BMS.Output.BatteryTemperature[0];
	g_SystemMonitor.BMS.Battery.Temperature[1] = g_AO_BMS.Output.BatteryTemperature[1];
	g_SystemMonitor.BMS.Battery.Temperature[2] = g_AO_BMS.Output.BatteryTemperature[2];
	*/
	// ���������Ϣ
	g_SystemMonitor.BMS.Battery.CellBalanceEnable = g_AO_SH36730x0.State.BatteryBalanceEnable;		// ��ص������ʹ�ܹ��ܿ���
	g_SystemMonitor.BMS.Battery.CellBalancing = g_AO_SH36730x0.State.BatteryBalancing;				// ��ص��������
	g_SystemMonitor.BMS.Battery.CellBalancePointer = g_AO_SH36730x0.State.BatteryBalancePointer;    // ������ھ��ⵥ����
	
	// ��ص����ѹ
	for (i = 0; i < 15; i++)
	{
		g_SystemMonitor.BMS.Cell.CellVoltage[i] = g_AO_SH36730x0.Output.SingleVoltage[i];
	}
//==============================================Ӧ�ò����ˣ����ڼ��һ��======================================================================	
//	// AFEоƬ�ڲ��¶Ȳ���
//    if (g_AO_SH36730x0.BQ769x0Register.SYS_CTRL1.bit.TEMP_SEL == 0)
//    {
      	for (i = 0; i < 3; i++)
	    {
		    g_SystemMonitor.BMS.AFE.InnerTemperature[i] = g_AO_SH36730x0.Output.InnerTemperature[i] ;
	    }	  
//    }
//    else
//    {
//    	for (i = 0; i < 3; i++)
//    	{
//    		g_SystemMonitor.BMS.AFE.InnerTemperature[i] = g_AO_BQ769x0.Output.ExternalTemperature[i] ;
//    	}	
//    }
//====================================================================================================================	
    // ����ڲ��¶Ȳ���,��8·
	for (i = 0; i < 4; i++)
	{
		g_SystemMonitor.BMS.AFE.BatteryTemperature[i] = g_AO_BMS.Output.BatteryTemperature[i];
	}	

	// ��ŵ�ʹ�ܱ�־λ
	g_SystemMonitor.BMS.Manage.Manage_bits.bit.DischargeEnable = g_AO_SH36730x0.State.DSGControl;
	g_SystemMonitor.BMS.Manage.Manage_bits.bit.ChargeEnable = g_AO_SH36730x0.State.CHGControl;
	
	// ��ŵ翪ʼʵ��״̬,ֱ��ͨ��SH36730x0оƬ�Ĵ������
	g_SystemMonitor.BMS.Manage.Manage_bits.bit.DischargeState = g_AO_SH36730x0.SH36730x0Register.BSTATUS.bit.DSG;//��״̬�Ĵ���BSTATUS����һЩ
	g_SystemMonitor.BMS.Manage.Manage_bits.bit.ChargeState = g_AO_SH36730x0.SH36730x0Register.BSTATUS.bit.CHG;////����//=========================
	
	// ��ŵ����IO������źţ�����MCU����Ӵ��������������
	g_SystemMonitor.BMS.Manage.Manage_bits.bit.DischargeIOState = g_Input.Output.DIN_bits.bit.DSG;
	g_SystemMonitor.BMS.Manage.Manage_bits.bit.ChargeIOState = g_Input.Output.DIN_bits.bit.CHG;	
	
	// KEY�����ź�
	g_SystemMonitor.BMS.Manage.Manage_bits.bit.KEYState = g_Input.Output.DIN_bits.bit.KEY;
	
	// �������������־λ
	g_SystemMonitor.BMS.Manage.Manage_bits.bit.ExtraBalance = g_AO_BMS.State.BatteryExtraBalance;
	
	// ��������ѳ���ʱ�䣬��λmin
	g_SystemMonitor.BMS.Manage.ExtraBalanceLastTime = g_AO_BMS.Variable.ExtraBallanceCount;
	
	// ������Ƶ�������λ0.1A
	g_SystemMonitor.BMS.Manage.ChargeCurrentLimit = g_AO_BMS.Output.ChargeCurrentLimit;
	
	// BMS������¶ȣ���λ0.1���϶�
	for(i = 0; i < 3; i++)
	{
	    g_SystemMonitor.BMS.BMSTemperature.BMSTemperature[i] = g_AO_BMS.Output.BMSTemperature[i];
	}
    // �ڲ�������ѹ����λ0.1V
    g_SystemMonitor.BMS.Manage.DriveVoltage = g_ADC.Output.DriveVoltage;

	// BMS������Ϣ 
	if(Protect_GetFaultCode(&g_Protect) > 0)
	{
    	g_SystemMonitor.BMS.Fault.FaultCodeLv0 = g_Protect.FaultCode.SystemLockLevel0;
    	g_SystemMonitor.BMS.Fault.FaultCodeLv1 = g_Protect.FaultCode.OutputDisableLevel1;
	}   
    else
    {
        g_SystemMonitor.BMS.Fault.FaultCodeLv0 = g_Protect.FaultCode.SystemLockLevel0;
    	g_SystemMonitor.BMS.Fault.FaultCodeLv1 = g_Protect.FaultCode.DriveLimitLevel2;
    }
//================================================================================================================	
//	g_SystemMonitor.BMS.Fault.FaultInf_bits.bit.ShortCurrent = g_AO_SH36730x0.SH36730x0Register.FLAG1.bit.SC;//
	//g_SystemMonitor.BMS.Fault.FaultInf_bits.bit.OverCurrent = g_AO_SH36730x0.SH36730x0Register.FLAG1.bit.SC;;//��ع���
//	g_SystemMonitor.BMS.Fault.FaultInf_bits.bit.CellOverVoltage = g_AO_SH36730x0.SH36730x0Register.FLAG1.bit.OV;
	//g_SystemMonitor.BMS.Fault.FaultInf_bits.bit.CellUnderVoltage = g_AO_SH36730x0.BQ769x0Register.SYS_STAT.bit.UV;

//	g_SystemMonitor.BMS.Fault.FaultInf_bits.bit.OverTemperature = g_AO_BQ769x0.BQ769x0Register.SYS_STAT.bit.SCD;
//	g_SystemMonitor.BMS.Fault.FaultInf_bits.bit.UnderTemperature = g_AO_BQ769x0.BQ769x0Register.SYS_STAT.bit.SCD;
//===================================================================================================================	
	g_SystemMonitor.System.Reverse.Data[14] = g_SystemState.Output.System1msInterruptCount;
	g_SystemMonitor.System.Reverse.Data[15] = AO_Record[0];
    
    // For test: Reverse3��ʾAO_BQ769x0״̬����ת��¼
    for (i = 0; i < 16; i++)
    {
        g_SystemMonitor.System.Reverse3.Data[i] = g_AO_SH36730x0.Record[i];
    }
	
}

//u16	t_DebugCANCount = 0;
//u16	t_Address = 0;
//u16	t_Data = 0;

// -------------------------- ϵͳ�����ô���������������ѭ���� ------------------------------
void Debuger_Update(void)
{
    u16 i = 0;    	
	// ����MODBUSģ����Զ������ȡд��ָ��
	// ������д��ָ���յ������ⲿ��������д��EEPROM�Ĳ���
	if (g_USART_MODBUS.Output.ReceiveBufferUpdated[1] > 0)			// ����1��ϵͳ����д����±�־λ
	{
		g_USART_MODBUS.Output.ReceiveBufferUpdated[1] = 0;
	
        if (g_SystemParameter.System.Test.test_Shutdown > 0)			// ����λ������ǿ�Ƶ������ָ��
        {
            g_AO_SH36730x0.State.EnterSHIPModeAsk = 1;	// �������SHIP�͹���ģʽ
        }
        else if (g_SystemParameter.System.Test.test_ClearFaultHistory > 0)      // ���������ʷ��¼
        {
            
            for (i = 0; i < FAULT_HISTORY_LENGTH; i++)
            {
                g_SystemFaultHistory.FaultHistory[i].all = 0;
            }
            
            for (i = 0; i < FAULT_COUNT_LENGTH; i++)
            {
                g_SystemFaultHistory.FaultCount[i].all = 0;
            }  
            
            // ���͹��ϼ�¼д���¼�
            QACTIVE_POST((QActive *)&g_AO_EEPROM, EEPROM_WRITE_SIG, EEPROM_BLOCK_FAULT_HISTORY);
        }
        else if(g_SystemParameter.System.Test.test_ClearSOC > 0)
        {
            g_SystemParameter.System.Test.test_ClearSOC = 0;

            g_SystemRecord.BatteryCapacity = (g_AO_BMS.Parameter.BatteryDesignCapacity *3) / 10;  
//            g_SystemRecord.SOC = 300;        
            g_AO_BMS.Output.BatteryCapacity = g_SystemRecord.BatteryCapacity;
//            g_AO_BMS.Output.SOC = g_SystemRecord.SOC;   

            g_AO_EEPROM.State.RecordWriteAsk = 1;           // Ask  to write record to EEPROM.
        }
        else
        { 
            // �˲������洢����������Ч
            g_AO_SH36730x0.State.test_ForceBatteryBalancePointer = g_SystemParameter.System.Test.test_BalancePointer;
            g_SystemParameter.System.Test.test_BalancePointer = 0;

            g_SystemState.Variable.ParameterWriteDelay = 0;
            g_SystemState.Variable.ParameterWriteAsk = 1;
            
            // �������º��Զ���ʼ��һ��
            System_ParameterSet();
        }
	}
}

// ----------------------------- ϵͳ������ʼ������ֵΪĬ�ϲ��� ------------------------------
void SystemParameter_Init(void)
{

#if (CONTROLLER_TARGET == BMS_EP_A5 || CONTROLLER_TARGET == BMS_EP_20_REV1_1_2TEMP || CONTROLLER_TARGET == BMS_EP_ALPHA)						// �豸ѡ������BMS��A2/3��
	
    // �������
	g_SystemParameter.System.Information.DeviceName[0] = 'E';
	g_SystemParameter.System.Information.DeviceName[1] = 'P';
	g_SystemParameter.System.Information.SerialNumber1 = 12345;
	g_SystemParameter.System.Information.SerialNumber2 = 54321;
	g_SystemParameter.System.Information.ManufactureData = 1824;			// 2018���44��
	g_SystemParameter.System.Information.HardwareVersion = 600;
	g_SystemParameter.System.Information.SoftwareVersion = CONST_SOFTWARE_VERSION;				// ����汾��
	g_SystemParameter.System.Information.PrototalVersion = 0;
    g_SystemParameter.System.Information.ParameterVersion = CONST_PARAMETER_VERSION;
    g_SystemParameter.System.Test.test_ChargeReqVol = 2;
	// �����������Ŵ������ƫ�ã���У׼
	// CCֵ��λΪ8.44uV����������0.001ŷķ������CC = 1��Ӧ����Ϊ1 * 8.44 / 0.001 = 8440	
	// �����������Ŵ�����������λmA����ֵ1000	
	// Ĭ��1mŷķ�������裬��100Aʱ��Ӧ��ѹ100mV

    g_SystemParameter.System.Calibration.CurrentSensorMaxCurrent = 4000;		// 1000A  //400A
	g_SystemParameter.System.Calibration.CurrentSensorMaxVoltage = 1933;		// 500mV  //200mv
	g_SystemParameter.System.Calibration.BMSIdleCurrent = 15;				// BMS������ĵ�������λmA��ʵ��õ���������������
    g_SystemParameter.System.Calibration.InnerDriveSensorGain = 251;        // �ڲ�������ѹ�������Ŵ��������λ0.1kOhm
    g_SystemParameter.System.Calibration.InnerDriveSensorOffset = 0;        // �ڲ�������ѹ������ƫ�ã���λ0.1V  

		
	g_SystemParameter.BMS.Battery.AFESelect = BQ76930;

    g_SystemParameter.BMS.Battery.SeriesNumber = 8;                     // ����ܴ���Ϊ8��   
    g_SystemParameter.BMS.BatteryType.BatteryType = 1;                   //������ͣ�0����Ԫ 1�����
    g_SystemParameter.BMS.BatteryType.BatteryTypeReal = 1;    

    #if (BATTERY_VERSION == 121)
    g_SystemParameter.BMS.Battery.FullCapacity = 350;						// �����ȫ����������λ0.1AH
    g_SystemParameter.BMS.Battery.DesignCapacity = 350;						// ������ʹ�õ�������λ0.1AH
    g_SystemParameter.BMS.Battery.ShortCutDownCurrent = 380;
    g_SystemParameter.BMS.Battery.OverCutDownCurrent = 150;
    g_SystemParameter.BMS.Battery.ChargeCurrentLimit = 15;					// ���������������15A����Ӧ20AH���	
    g_SystemParameter.BMS.Battery.BatteryTemperatureCheckMode = 2;		// ʹ�ܵ���¶ȼ��

    g_SystemParameter.BMS.Output.AutoCutoffCurrent = 600;					// ����Զ��жϵ�����600mA
    
    g_SystemParameter.BMS.Charge.ChargeOverCurrent = 22;                    // ����������20A        
    #else       
    g_SystemParameter.BMS.Battery.FullCapacity = 460;					// �����ȫ����������λ0.1AH
    g_SystemParameter.BMS.Battery.DesignCapacity = 450;						// ������ʹ�õ�������λ0.1AH
    g_SystemParameter.BMS.Battery.ShortCutDownCurrent = 380;
    g_SystemParameter.BMS.Battery.OverCutDownCurrent = 150;
    g_SystemParameter.BMS.Battery.ChargeCurrentLimit = 30;					// ���������������15A����Ӧ20AH���
    g_SystemParameter.BMS.Battery.BatteryTemperatureCheckMode = 2;		// ʹ�ܵ���¶ȼ��
    
    g_SystemParameter.BMS.Output.AutoCutoffCurrent = 600;					// ����Զ��жϵ�����600mA

    g_SystemParameter.BMS.Charge.ChargeOverCurrent = 35;                    // ����������20A                
    #endif
    
    g_SystemParameter.BMS.Battery.CellOverVoltage = 3700;
    g_SystemParameter.BMS.Battery.CellUnderVoltage = 2200;
    g_SystemParameter.BMS.Battery.ShortCutDownDelay = 500;            //50 100 200 500us
    g_SystemParameter.BMS.Battery.OverCutDownDelay = 700;
    g_SystemParameter.BMS.Battery.CellChargeStopVoltage = 3600;             // ��������3.6V��ֹ
    g_SystemParameter.BMS.Battery.ChargeStopDelay = 60;                     // ��س�������ʱ����λ1s
   // g_SystemParameter.BMS.Battery.BatteryTemperatureCheckMode = 2;          // ʹ�ܵ���¶ȼ��
        
    g_SystemParameter.BMS.Output.AutoCutoffDelay = 900;                     // ����Զ��ж���ʱ��900s
    g_SystemParameter.BMS.Output.SleepDelay = 600;                          // BMS����͹���״̬ǰ��ʱ����λs
    g_SystemParameter.BMS.Output.SleepDelayLong = 0;                       // BMS����͹���״̬ǰ��ʱ����ʱ����λh����SleepDelay�ۼ�
    g_SystemParameter.BMS.Output.OutputDelay = 500;                        // ���ذ���1000ms���������        
    
    g_SystemParameter.BMS.Charge.ChargeVoltageStep1 = 3400;                 // �������׶�1��������ߵ�ѹ���ڴ˵�ѹ�����������1C��Ϊ0.5C����λ1mV
    g_SystemParameter.BMS.Charge.ChargeVoltageStep2 = 3500;                 // �������׶�2��������ߵ�ѹ���ڴ˵�ѹ�����������0.5C��Ϊ0.25C����λ1mV 
    g_SystemParameter.BMS.Charge.ChargeVoltageStep3 = 3540;                 // �������׶�3��������ߵ�ѹ���ڴ˵�ѹ�����������0.25C��Ϊ0.125C����λ1mV   
    g_SystemParameter.BMS.Charge.ChargeForceStopVoltage = 3660;             // ���ǿ�ƽ�����ѹ����λmV
    g_SystemParameter.BMS.Charge.ChargeFinishMinVoltage = 3500;           // ��������С�����ѹ����λmV
    g_SystemParameter.BMS.Charge.ChargeOverCurrentDelay = 30;               // ��������ʱ30s 

    g_SystemParameter.BMS.Charge2.ChargeLimitTemperature1 = 60;
    g_SystemParameter.BMS.Charge2.ChargeLimitTemperature2 = 45;
    g_SystemParameter.BMS.Charge2.ChargeLimitTemperature3 = 10;
    g_SystemParameter.BMS.Charge2.ChargeLimitTemperature4 = -2;
    
   // g_SystemParameter.BMS.ChargeGB.BalanceVoltage = 3100; 
    
    g_SystemParameter.BMS.Discharge.DischargeForceStopVoltage = 2800;       // ǿ�ƽ����ŵ��ѹ
    g_SystemParameter.BMS.Discharge.DischargeStopVoltage = 3160;            // �ŵ��ֹ��ѹ�趨Ϊ3.0V

     
    g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage1 = 2700;               // У׼��ѹ1����Ӧ����1%
    g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit1 = 2550;       // У׼��ѹ1���������ֵ
    g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage2 = 2800;               // У׼��ѹ2����Ӧ����3.3%
    g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit2 = 2650;       // У׼��ѹ2���������ֵ
    g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage3 = 3000;               // У׼��ѹ3����Ӧ����10%
    g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit3 = 2750;       // У׼��ѹ3���������ֵ
    g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage4 = 3100;               // У׼��ѹ4����Ӧ����20%
    g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit4 = 2800;       // У׼��ѹ4���������ֵ   

    g_SystemParameter.BMS.BatteryType.BatteryType = 1;                      // ������ͣ�0����Ԫ��1���������
    g_SystemParameter.BMS.BatteryType.BatteryTypeReal = 1;


    g_SystemParameter.BMS.Fan.FanEnableTemperature = 0;                     // Ĭ�ϲ��������½�ֹ��繦��
    g_SystemParameter.BMS.Fan.FanCurrent = 25;                              // �̵�����������25mA
    g_SystemParameter.BMS.Fan.FanMode = 2;
    g_SystemParameter.BMS.Warning.BDILowLevel1Percent = 30;					// ��ص����Ͱٷֱȣ���λ%
    g_SystemParameter.BMS.Warning.BDILowLevel2Percent = 15;					// ��ص������ع��Ͱٷֱȣ���λ%
    g_SystemParameter.BMS.Warning.BDILowLimitLift = 15;						// ��ص����Ͱٷֱȣ����ƾ�������λ%
    g_SystemParameter.BMS.Warning.BDILowLimitSpeed = 7;						// ��ص����Ͱٷֱȣ����Ƴ��٣���λ%
    g_SystemParameter.BMS.Warning.BMSOTemperatureCheckEnable = 0;			// Ĭ��ʹ��BMS������¶ȼ��
    g_SystemParameter.BMS.Warning.BMSOverTemperature = 95;
    g_SystemParameter.BMS.Warning.LowSOCBeep = 17;
   // g_SystemParameter.BMS.Protect.CellSoftOverVoltage = 3680;               // ���������ѹ��ѹ����λ1mV
   // g_SystemParameter.BMS.Protect.CellSoftOverVoltageDelay = 10;             // ���������ѹ��ʱ����λ1s
    g_SystemParameter.BMS.Protect.CellHardwareOverVoltageDelay = 1;         // ����Ӳ����ѹ��ʱ����λ1s
    g_SystemParameter.BMS.Protect.CellUnderVoltageDelay = 15;                // ����Ƿѹ��ʱ����λs
   // g_SystemParameter.BMS.Protect.ShortCurrentTimeFactor = 50;

   	g_SystemParameter.BMS.Battery.OverTemperature = 65;
	g_SystemParameter.BMS.Battery.UnderTemperature = -25; 
    g_SystemParameter.BMS.Output.SleepDelay = 600;							// BMS����͹���״̬ǰ��ʱ����λs
    g_SystemParameter.BMS.Output.SleepDelayLong = 0;                        // BMS����͹���״̬ǰ��ʱ����ʱ����λh����SleepDelay�ۼ�
    
    g_SystemParameter.BMS.Discharge.dsg_cc_low_k = 90;
    g_SystemParameter.BMS.Discharge.dsg_tmp_low_k = 10;
    
    g_SystemParameter.BMS.Contactor.ContactorBaseFrequency = 20;			// ����PWMƵ�ʣ���λHz
	g_SystemParameter.BMS.Contactor.ContactorFullPercentTime = 2;		// �Ӵ�������ʱȫռ�ձȳ���ʱ�䣬��λms
	g_SystemParameter.BMS.Contactor.ContactorLongLastPercent = 150;			// �Ӵ�����ʱ�乤��ʱռ�ձȣ���λ0.1%
    
#elif (CONTROLLER_TARGET == BMS_EP_200_A1 || CONTROLLER_TARGET == BMS_EP_200_A2 || CONTROLLER_TARGET == BMS_EP_200_B1 || CONTROLLER_TARGET == BMS_EP_200_B2_1)

	// 200ϵ��BMS�����ⲿ���������ⲿ�Ӵ�����������ͨ����ƣ�������60AH����﮵��
	g_SystemParameter.System.Information.DeviceName[0] = 'E';
	g_SystemParameter.System.Information.DeviceName[1] = 'P';
	g_SystemParameter.System.Information.SerialNumber1 = 12345;
	g_SystemParameter.System.Information.SerialNumber2 = 54321;
	g_SystemParameter.System.Information.ManufactureData = 1611;
	g_SystemParameter.System.Information.HardwareVersion = 100;
	g_SystemParameter.System.Information.SoftwareVersion = CONST_SOFTWARE_VERSION;				// ����汾��1.1.3
	g_SystemParameter.System.Information.PrototalVersion = 0;

	// �����������Ŵ������ƫ�ã���У׼
	// CCֵ��λΪ8.44uV����������0.001ŷķ������CC = 1��Ӧ����Ϊ1 * 8.44 / 0.001 = 8440	
	// �����������Ŵ�����������λmA����ֵ1000	
	// Ĭ��1mŷķ�������裬��100Aʱ��Ӧ��ѹ100mV
	g_SystemParameter.System.Calibration.CurrentSensorMaxCurrent = 1000;		// 100A
	g_SystemParameter.System.Calibration.CurrentSensorMaxVoltage = 750;		// 75mV
	g_SystemParameter.System.Calibration.BMSIdleCurrent = 15;				// BMS������ĵ�������λmA��ʵ��õ���������������
		
	g_SystemParameter.BMS.Battery.AFESelect = BQ76930;
	g_SystemParameter.BMS.Battery.SeriesNumber = 7;							// ����ܴ���Ϊ7��
	g_SystemParameter.BMS.Battery.FullCapacity = 600;						// �����ȫ����������λ0.1AH
	g_SystemParameter.BMS.Battery.DesignCapacity = 600;						// ������ʹ�õ�������λ0.1AH
	g_SystemParameter.BMS.Battery.CellOverVoltage = 4250;
	g_SystemParameter.BMS.Battery.CellUnderVoltage = 2600;
	g_SystemParameter.BMS.Battery.ShortCutDownCurrent = 200;
	g_SystemParameter.BMS.Battery.ShortCutDownDelay = 400;
	g_SystemParameter.BMS.Battery.OverCutDownCurrent = 150;
	g_SystemParameter.BMS.Battery.OverCutDownDelay = 500;
	g_SystemParameter.BMS.Battery.CellChargeStopVoltage = 4195;				// ��������4.195V��ֹ
	g_SystemParameter.BMS.Battery.ChargeCurrentLimit = 30;					// ���������������30A����Ӧ60AH���
	g_SystemParameter.BMS.Battery.BatteryTemperatureCheckEnable = 1;		// ʹ�ܵ���¶ȼ��
	g_SystemParameter.BMS.Battery.OverTemperature = 65;
	g_SystemParameter.BMS.Battery.UnderTemperature = -25;
	
	g_SystemParameter.BMS.Output.AutoCutoffCurrent = 50;					// ����Զ��жϵ�����50mA
	g_SystemParameter.BMS.Output.AutoCutoffDelay = 600;						// ����Զ��ж���ʱ��600s
	g_SystemParameter.BMS.Output.SleepDelay = 600;							// BMS����͹���״̬ǰ��ʱ����λs

	g_SystemParameter.BMS.Contactor.ContactorBaseFrequency = 1000;			// ����PWMƵ�ʣ���λHz
	g_SystemParameter.BMS.Contactor.ContactorFullPercentTime = 2000;		// �Ӵ�������ʱȫռ�ձȳ���ʱ�䣬��λms
	g_SystemParameter.BMS.Contactor.ContactorLongLastPercent = 750;			// �Ӵ�����ʱ�乤��ʱռ�ձȣ���λ0.1%

	g_SystemParameter.BMS.Warning.BDILowLevel1Percent = 15;					// ��ص����Ͱٷֱȣ���λ%
	g_SystemParameter.BMS.Warning.BDILowLevel2Percent = 7;					// ��ص������ع��Ͱٷֱȣ���λ%
	g_SystemParameter.BMS.Warning.BDILowLimitLift = 15;						// ��ص����Ͱٷֱȣ����ƾ�������λ%
	g_SystemParameter.BMS.Warning.BDILowLimitSpeed = 7;						// ��ص����Ͱٷֱȣ����Ƴ��٣���λ%
	g_SystemParameter.BMS.Warning.BMSOTemperatureCheckEnable = 1;			// Ĭ��ʹ��BMS������¶ȼ��	
	g_SystemParameter.BMS.Warning.BMSOverTemperature = 90;					// Ĭ��BMS���������¶�Ϊ85���϶�

#elif (CONTROLLER_TARGET == BMS_XINXIN_A1)					// �豸ѡ����꿻���BMS��A1��
		
	g_SystemParameter.System.Information.DeviceName[0] = 1;
	g_SystemParameter.System.Information.DeviceName[1] = '2';
	g_SystemParameter.System.Information.SerialNumber1 = 12345;
	g_SystemParameter.System.Information.SerialNumber2 = 54321;
	g_SystemParameter.System.Information.ManufactureData = 1611;
	g_SystemParameter.System.Information.HardwareVersion = 100;
	g_SystemParameter.System.Information.SoftwareVersion = 33;				// ����汾��0.3.3
	g_SystemParameter.System.Information.PrototalVersion = 0;

	// �����������Ŵ������ƫ�ã���У׼
	// CCֵ��λΪ8.44uV����������0.001ŷķ������CC = 1��Ӧ����Ϊ1 * 8.44 / 0.001 = 8440	
	// �����������Ŵ�����������λmA����ֵ1000	
	// Ĭ��1mŷķ�������裬��100Aʱ��Ӧ��ѹ100mV
	g_SystemParameter.System.Calibration.CurrentSensorMaxCurrent = 1000;		// 100A
	g_SystemParameter.System.Calibration.CurrentSensorMaxVoltage = 1000;		// 100mV
	g_SystemParameter.System.Calibration.BMSIdleCurrent = 10;				// BMS������ĵ�������λmA��ʵ��õ���������������
		
	g_SystemParameter.BMS.Battery.AFESelect = BQ76930;
	g_SystemParameter.BMS.Battery.SeriesNumber = 6;							// 6��
	g_SystemParameter.BMS.Battery.FullCapacity = 80;						// �����ȫ����������λ0.1AH
	g_SystemParameter.BMS.Battery.DesignCapacity = 80;						// ������ʹ�õ�������λ0.1AH
	g_SystemParameter.BMS.Battery.CellOverVoltage = 4230;
	g_SystemParameter.BMS.Battery.CellUnderVoltage = 2800;
	g_SystemParameter.BMS.Battery.ShortCutDownCurrent = 100;
	g_SystemParameter.BMS.Battery.ShortCutDownDelay = 400;
	g_SystemParameter.BMS.Battery.OverCutDownCurrent = 40;
	g_SystemParameter.BMS.Battery.OverCutDownDelay = 500;
	g_SystemParameter.BMS.Battery.CellChargeStopVoltage = 4200;				// ��������4.2V��ֹ
	g_SystemParameter.BMS.Battery.ChargeCurrentLimit = 5;					// ���������������5A����Ӧ8AH���
	g_SystemParameter.BMS.Battery.BatteryTemperatureCheckEnable = 0;		// ��ʹ�ܵ���¶ȼ��
	g_SystemParameter.BMS.Battery.OverTemperature = 65;
	g_SystemParameter.BMS.Battery.UnderTemperature = -25;
	
	g_SystemParameter.BMS.Output.AutoCutoffCurrent = 50;					// ����Զ��жϵ�����500mA
	g_SystemParameter.BMS.Output.AutoCutoffDelay = 600;						// ����Զ��ж���ʱ��600s
	g_SystemParameter.BMS.Output.SleepDelay = 600;							// BMS����͹���״̬ǰ��ʱ����λs
	g_SystemParameter.BMS.Output.OutputDelay = 1000;						// ���ذ���1s���������
	g_SystemParameter.BMS.Output.PreDischargeTime = 500;					// Ԥ�ŵ�ʱ�䣬��λms
		
	g_SystemParameter.BMS.Warning.BDILowLevel1Percent = 15;					// ��ص����Ͱٷֱȣ���λ%
	g_SystemParameter.BMS.Warning.BDILowLevel2Percent = 5;					// ��ص������ع��Ͱٷֱȣ���λ%
	g_SystemParameter.BMS.Warning.BMSOTemperatureCheckEnable = 0;			// Ĭ�ϲ�ʹ��BMS������¶ȼ��
	g_SystemParameter.BMS.Warning.BMSOverTemperature = 90;					// Ĭ��BMS���������¶�Ϊ85���϶�
	
#elif (CONTROLLER_TARGET == BMS_XINXIN_A2)					// �豸ѡ����꿻���BMS��A2��
		
		
	g_SystemParameter.System.Information.DeviceName[0] = 'X';
	g_SystemParameter.System.Information.DeviceName[1] = 'I';
	g_SystemParameter.System.Information.DeviceName[2] = 'N';
	g_SystemParameter.System.Information.DeviceName[3] = ' ';
	g_SystemParameter.System.Information.DeviceName[4] = 'X';
	g_SystemParameter.System.Information.DeviceName[5] = 'I';
	g_SystemParameter.System.Information.DeviceName[6] = 'N';

	g_SystemParameter.System.Information.SerialNumber1 = 12345;
	g_SystemParameter.System.Information.SerialNumber2 = 54321;
	g_SystemParameter.System.Information.ManufactureData = 1611;
	g_SystemParameter.System.Information.HardwareVersion = 100;
	g_SystemParameter.System.Information.SoftwareVersion = 44;				// ����汾��0.4.4
	g_SystemParameter.System.Information.PrototalVersion = 0;

	// �����������Ŵ������ƫ�ã���У׼
	// CCֵ��λΪ8.44uV����������0.001ŷķ������CC = 1��Ӧ����Ϊ1 * 8.44 / 0.001 = 8440	
	// �����������Ŵ�����������λmA����ֵ1000	
	// Ĭ��1mŷķ�������裬��100Aʱ��Ӧ��ѹ100mV
	g_SystemParameter.System.Calibration.CurrentSensorMaxCurrent = 1000;		// 100A
	g_SystemParameter.System.Calibration.CurrentSensorMaxVoltage = 2000;		// 200mV
	g_SystemParameter.System.Calibration.BMSIdleCurrent = 10;				// BMS������ĵ�������λmA��ʵ��õ���������������
		
	g_SystemParameter.BMS.Battery.AFESelect = BQ76930;
	g_SystemParameter.BMS.Battery.SeriesNumber = 6;							// 6��
	g_SystemParameter.BMS.Battery.FullCapacity = 80;						// �����ȫ����������λ0.1AH
	g_SystemParameter.BMS.Battery.DesignCapacity = 80;						// ������ʹ�õ�������λ0.1AH
	g_SystemParameter.BMS.Battery.CellOverVoltage = 4250;
	g_SystemParameter.BMS.Battery.CellUnderVoltage = 2600;
	g_SystemParameter.BMS.Battery.ShortCutDownCurrent = 60;
	g_SystemParameter.BMS.Battery.ShortCutDownDelay = 500;
	g_SystemParameter.BMS.Battery.OverCutDownCurrent = 30;
	g_SystemParameter.BMS.Battery.OverCutDownDelay = 500;
	g_SystemParameter.BMS.Battery.CellChargeStopVoltage = 4200;				// ��������4.2V��ֹ
	g_SystemParameter.BMS.Battery.ChargeCurrentLimit = 6;					// ���������������6A����Ӧ10AH���
	g_SystemParameter.BMS.Battery.BatteryTemperatureCheckEnable = 0;		// ��ʹ�ܵ���¶ȼ��
	g_SystemParameter.BMS.Battery.OverTemperature = 65;
	g_SystemParameter.BMS.Battery.UnderTemperature = -25;
	
	g_SystemParameter.BMS.Output.AutoCutoffCurrent = 50;					// ����Զ��жϵ�����100mA
	g_SystemParameter.BMS.Output.AutoCutoffDelay = 60;						// ����Զ��ж���ʱ��60s
	g_SystemParameter.BMS.Output.SleepDelay = 600;							// BMS����͹���״̬ǰ��ʱ����λs
	g_SystemParameter.BMS.Output.OutputDelay = 1000;						// ���ذ���1s���������
	g_SystemParameter.BMS.Output.PreDischargeTime = 500;					// Ԥ�ŵ�ʱ�䣬��λms
		
	g_SystemParameter.BMS.Warning.BDILowLevel1Percent = 15;					// ��ص����Ͱٷֱȣ���λ%
	g_SystemParameter.BMS.Warning.BDILowLevel2Percent = 5;					// ��ص������ع��Ͱٷֱȣ���λ%
	g_SystemParameter.BMS.Warning.BMSOTemperatureCheckEnable = 0;			// Ĭ�ϲ�ʹ��BMS������¶ȼ��
	g_SystemParameter.BMS.Warning.BMSOverTemperature = 90;					// Ĭ��BMS���������¶�Ϊ85���϶�
	
#endif						// CONTROLLER_TARGET == BMS_EP_A5

}

// -------------------- ����EEPROM����������ϵͳģ����� ------------------------
void System_ParameterSet(void)
{
    // ����ض���ʱ���Ϊ24H
    if (g_SystemParameter.BMS.Output.SleepDelayLong > 24)
    {
        g_SystemParameter.BMS.Output.SleepDelayLong = 24;
    }
    
    g_SystemParameter.System.Test.test_BalancePointer = 0;
    g_SystemParameter.System.Test.test_Shutdown = 0;
    g_SystemParameter.System.Test.test_ClearFaultHistory = 0;    
    
	// ����汾��������Ϣ�����target.h�궨�����������EEPROM�洢����Ӱ��
	g_SystemParameter.System.Information.SoftwareVersion = CONST_SOFTWARE_VERSION;
	g_Protect.ProtectMode.bit.BatteryTemperatureCheck = g_SystemParameter.BMS.Battery.BatteryTemperatureCheckMode;
	
	// �����������Ŵ������ƫ�ã���У׼
	// CCֵ��λΪ8.44uV����������0.001ŷķ������CC = 1��Ӧ����Ϊ1 * 8.44 / 0.001 = 8440
	// �����������Ŵ�����������λmA����ֵ1000	
	g_AO_SH36730x0.Parameter.CCGain = (s32)g_SystemParameter.System.Calibration.CurrentSensorMaxCurrent * 8440 / g_SystemParameter.System.Calibration.CurrentSensorMaxVoltage;										
	g_AO_SH36730x0.Parameter.CCOffset = 0;	
	
	g_AO_SH36730x0.Parameter.SH36730x0_Type = g_SystemParameter.BMS.Battery.AFESelect;	
	
	g_AO_SH36730x0.Parameter.CellNumber = g_SystemParameter.BMS.Battery.SeriesNumber;
	
	// Take care: ���ݲ�ͬ��оƬ�Ͳ�ͬ�������������岻ͬ������
	switch (g_SystemParameter.BMS.Battery.AFESelect)			// ǰ�˲���оƬѡ��
	{
		case BQ76930:
		{
			switch (g_AO_SH36730x0.Parameter.CellNumber)
			{
				case 6:
				{
					g_AO_SH36730x0.Parameter.CellSelect[0] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[1] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[2] = 0;
					g_AO_SH36730x0.Parameter.CellSelect[3] = 0;
					g_AO_SH36730x0.Parameter.CellSelect[4] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[5] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[6] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[7] = 0;
					g_AO_SH36730x0.Parameter.CellSelect[8] = 0;
					g_AO_SH36730x0.Parameter.CellSelect[9] = 1;			
				}
				break;
				
				case 7:
				{
					g_AO_SH36730x0.Parameter.CellSelect[0] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[1] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[2] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[3] = 0;
					g_AO_SH36730x0.Parameter.CellSelect[4] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[5] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[6] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[7] = 0;
					g_AO_SH36730x0.Parameter.CellSelect[8] = 0;
					g_AO_SH36730x0.Parameter.CellSelect[9] = 1;			
				}
				break;

				case 8:
				{
					g_AO_SH36730x0.Parameter.CellSelect[0] = 1;//�޸Ĺ�ע��----------------------------------------------------------------
					g_AO_SH36730x0.Parameter.CellSelect[1] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[2] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[3] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[4] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[5] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[6] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[7] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[8] = 0;
					g_AO_SH36730x0.Parameter.CellSelect[9] = 0;			
				}
				break;
										
				case 10:
				{
					g_AO_SH36730x0.Parameter.CellSelect[0] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[1] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[2] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[3] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[4] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[5] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[6] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[7] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[8] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[9] = 1;			
				}
				break;		
				
				default:
				{
					// �����������ô���
					Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER);					
				}
				break;
			}
		}
		break;
		
		default:						// ǰ�˲���оƬ����ʶ�𣬱���
		{
			// �����������ô���
			Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER);
		}
		break;
	}
    if (g_SystemParameter.BMS.BatteryType.BatteryType != g_SystemParameter.BMS.BatteryType.BatteryTypeReal)
    {
        // ������ͷ����ı䣬δ��Ч
        // �����������ô���
        Protect_SetFaultCodeLv2(&g_Protect, ERROR_ILLEGAL_PARAMETER);   

        g_SystemMonitor.System.Reverse.Data[0] = 4;
    }
    //���õ����ѹ��Ƿѹ��ѹ
	g_AO_SH36730x0.Parameter.SingleOverVoltage = g_SystemParameter.BMS.Battery.CellOverVoltage;			// �����ع�ѹ��3700mV
	g_AO_SH36730x0.Parameter.SingleUnderVoltage = g_SystemParameter.BMS.Battery.CellUnderVoltage;	   // ������Ƿѹ��2400mV

    switch (g_SystemParameter.BMS.BatteryType.BatteryType)
    {
        case 0: // ��Ԫ
        default:
        {
            if (g_SystemParameter.BMS.Battery.CellOverVoltage > 4350 
                || g_SystemParameter.BMS.Battery.CellOverVoltage < 4000)
            {
                // ��Ԫ��ص����ѹ��ѹ���ô���
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER);  
                
                g_SystemMonitor.System.Reverse.Data[0] = 5;
            }
            
            if (g_SystemParameter.BMS.Battery.CellUnderVoltage > 3400 
                || g_SystemParameter.BMS.Battery.CellUnderVoltage < 2000)
            {
                // ��Ԫ��ص���Ƿѹѹ��ѹ���ô���
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER); 

                g_SystemMonitor.System.Reverse.Data[0] = 6;                
            }          

            if (g_SystemParameter.BMS.Battery.CellChargeStopVoltage > 4250 
                || g_SystemParameter.BMS.Battery.CellChargeStopVoltage < 4000)
            {
                // ��Ԫ��س���ֹ��ѹ���ô���
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER); 

                g_SystemMonitor.System.Reverse.Data[0] = 7;
            }
            
            if (g_SystemParameter.BMS.Charge.ChargeVoltageStep1 > 4250
                || g_SystemParameter.BMS.Charge.ChargeVoltageStep1 < 3850
                || g_SystemParameter.BMS.Charge.ChargeVoltageStep2 > 4250
                || g_SystemParameter.BMS.Charge.ChargeVoltageStep2 < 3850
                || g_SystemParameter.BMS.Charge.ChargeVoltageStep3 > 4250
                || g_SystemParameter.BMS.Charge.ChargeVoltageStep3 < 3850
                || g_SystemParameter.BMS.Charge.ChargeForceStopVoltage > 4250
                || g_SystemParameter.BMS.Charge.ChargeFinishMinVoltage < 4000)
            {
                // ��Ԫ��س��׶ε�ѹ���ô���
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER);

                g_SystemMonitor.System.Reverse.Data[0] = 8;
            }
            
            if (g_SystemParameter.BMS.Discharge.DischargeForceStopVoltage < 2500 
                || g_SystemParameter.BMS.Discharge.DischargeStopVoltage < 2900)
            {
                // ��Ԫ��طŵ�������ô���
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER); 

                g_SystemMonitor.System.Reverse.Data[0] = 9;                
            }
            
           /* if (g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage1 < 3000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage1 > 3500
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage2 < 3000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage2 > 3500
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage3 < 3000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage3 > 3500
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage4 < 3000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage4 > 3500
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit1 < 3000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit1 > 3500
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit2 < 3000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit2 > 3500
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit3 < 3000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit3 > 3500
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit4 < 3000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit4 > 3500)
            {
                // ��Ԫ��ص���У׼�������ô���
                Protect_SetFaultCodeLv2(&g_Protect, ERROR_ILLEGAL_PARAMETER);  

                g_SystemMonitor.System.Reverse.Data[0] = 10;                
            }*/
        }
        break;
    
        case 1:                             // �������
        {
            if (g_SystemParameter.BMS.Battery.CellOverVoltage > 3800 
                || g_SystemParameter.BMS.Battery.CellOverVoltage < 3400)
            {
                // ������﮵�ص����ѹ��ѹ���ô���
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER);  
                
                g_SystemMonitor.System.Reverse.Data[0] = 11;
            }
            
            if (g_SystemParameter.BMS.Battery.CellUnderVoltage > 2600 
                || g_SystemParameter.BMS.Battery.CellUnderVoltage < 1800)
            {
                // ������﮵�ص���Ƿѹѹ��ѹ���ô���
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER);  
                
                g_SystemMonitor.System.Reverse.Data[0] = 12;
            }  

            if (g_SystemParameter.BMS.Battery.CellChargeStopVoltage > 3700 
                || g_SystemParameter.BMS.Battery.CellChargeStopVoltage < 3400)
            {
                // ������﮵�س���ֹ��ѹ���ô���
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER);  
                
                g_SystemMonitor.System.Reverse.Data[0] = 13;
            } 

            if (g_SystemParameter.BMS.Charge.ChargeVoltageStep1 > 3700
                || g_SystemParameter.BMS.Charge.ChargeVoltageStep1 < 2600
                || g_SystemParameter.BMS.Charge.ChargeVoltageStep2 > 3700
                || g_SystemParameter.BMS.Charge.ChargeVoltageStep2 < 2600
                || g_SystemParameter.BMS.Charge.ChargeVoltageStep3 > 3700
                || g_SystemParameter.BMS.Charge.ChargeVoltageStep3 < 2600
                || g_SystemParameter.BMS.Charge.ChargeForceStopVoltage > 3760
                || g_SystemParameter.BMS.Charge.ChargeFinishMinVoltage < 3350)
            {
                // ������﮵�س��׶ε�ѹ���ô���
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER); 

                g_SystemMonitor.System.Reverse.Data[0] = 14;                
            }  

            if (g_SystemParameter.BMS.Discharge.DischargeForceStopVoltage < 1600 ||
                 g_SystemParameter.BMS.Discharge.DischargeStopVoltage < 1900)
            {
                // ������﮵�طŵ�������ô���
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER); 

                g_SystemMonitor.System.Reverse.Data[0] = 15;                
            } 

            if (g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage1 < 2000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage1 > 3200
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage2 < 2000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage2 > 3200
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage3 < 2000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage3 > 3200
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage4 < 2000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage4 > 3200
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit1 < 2000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit1 > 3200
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit2 < 2000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit2 > 3200
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit3 < 2000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit3 > 3200
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit4 < 2000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit4 > 3200)
            {
                // ������﮵�ص���У׼�������ô���
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER); 

                g_SystemMonitor.System.Reverse.Data[0] = 16;                
            }            
        }
        break;
    }        
        
	g_AO_BMS.Parameter.BatteryFullCapacity = g_SystemParameter.BMS.Battery.FullCapacity * 100;			// �����ʵ����������λmAH
	g_AO_BMS.Parameter.BatteryDesignCapacity = g_SystemParameter.BMS.Battery.DesignCapacity * 100;		// ��Ƶ��������Ϊ8.4AH����λmAH��ʵ���������ڴ�ֵԼ10%
	g_AO_BMS.Parameter.BMSIdleCurrent = g_SystemParameter.System.Calibration.BMSIdleCurrent;			// BMS�����ص�������λmA�����ڲ����������
    

	//g_AO_SH36730x0.State.BatteryBalanceEnable = 0;				// ��ؾ����ʼ��ʱΪ�رգ���BMS״̬������
	g_AO_SH36730x0.Parameter.BallanceErrVoltage = 2000;				// Ĭ�ϵ�ѹ����20mV����ؾ��⿪��
/*	
	// �������н��õ���¶ȼ��ʱ��protectģ��ֹͣ������¶�
	if (g_SystemParameter.BMS.Battery.BatteryTemperatureCheckMode > 0)
	{
		g_Protect.ProtectMode.bit.BatteryTemperatureCheck = 1;		// ʹ�ܵ���¶ȼ��
	}
	else
	{
		g_Protect.ProtectMode.bit.BatteryTemperatureCheck = 0;		// ���õ���¶ȼ��
	}
*/					// CONTROLLER_TARGET == BMS_EP_200_A1
	    
    // AD������������
    g_ADC.Parameter.VdcSensorGain = g_SystemParameter.System.Calibration.VdcSensorGain;                         // ֱ����ѹ�������Ŵ��������λ0.1kOhm
    g_ADC.Parameter.VdcSensorOffset = g_SystemParameter.System.Calibration.VdcSensorOffset;                     // ֱ����ѹ������ƫ�ã���λ0.1V
    g_ADC.Parameter.InnerDriveSensorGain = g_SystemParameter.System.Calibration.InnerDriveSensorGain;           // �ڲ�������ѹ�������Ŵ��������λ0.1kOhm
    g_ADC.Parameter.InnerDriveSensorOffset = g_SystemParameter.System.Calibration.InnerDriveSensorOffset;       // �ڲ�������ѹ������ƫ�ã���λ0.1V 
    g_AO_BMS.Parameter.FanCurrent = g_SystemParameter.BMS.Fan.FanCurrent;                                       // �������е�������λmA��

}

s32 get_temp_current_v(s16 t, s32 c)
{
    s32 temp_current_v = 0;
    if(c < -1000)temp_current_v = -g_SystemParameter.BMS.Discharge.dsg_cc_low_k * c / g_SystemParameter.BMS.Battery.DesignCapacity / 100;
    if(t < 150)
    {
        if(!g_SystemParameter.BMS.Discharge.dsg_tmp_low_k)g_SystemParameter.BMS.Discharge.dsg_tmp_low_k = 10;
        temp_current_v += (250 - t) / 10 * (250 - t) * g_SystemParameter.BMS.Discharge.dsg_tmp_low_k / 500;
    }
    return temp_current_v;
}


// End of system_module.c

