/* ==================================================================================

 File name:     protect.h
 Originator:    BLJ
 Description:   ����ģ�飬��ϵͳ����

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 01-10-2015     Version 0.0.1           ���Թ���
-----------------------------------------------------------------------------------*/

#include "target.h"								// Include header of DSP select
#include "system_core.h"
//#include "protect.h"							// Include header of average.c


// --------------------------- Lv0�������ú��� -----------------------------------
void Protect_SetFaultCodeLv0(Protect_structDef* v, u16 FaultCode)
{
	if (v->FaultCode.SystemLockLevel0 == 0)
	{
		v->FaultCode.SystemLockLevel0 = FaultCode;
	}
}

// --------------------------- Lv1�������ú��� -----------------------------------
void Protect_SetFaultCodeLv1(Protect_structDef* v, u16 FaultCode)
{
	if (v->FaultCode.OutputDisableLevel1 == 0)
	{
		v->FaultCode.OutputDisableLevel1 = FaultCode;
	}
}

// --------------------------- Lv2�������ú��� -----------------------------------
void Protect_SetFaultCodeLv2(Protect_structDef* v, u16 FaultCode)
{
	if (v->FaultCode.DriveLimitLevel2 == 0)
	{
		v->FaultCode.DriveLimitLevel2 = FaultCode;
	}
}

// --------------------------- Lv2�������������1ms����һ��----------------------------------
void Protect_ClearFaultCodeLv2(Protect_structDef* v)
{
	switch (v->FaultCode.DriveLimitLevel2)
	{
		case WARNING_BATTERY_OVER_TEMPERATURE:
		{
			if (g_AO_BMS.Output.BatteryTemperature[0] < (g_SystemParameter.BMS.Battery.OverTemperature * 10 - 60)
				&& g_AO_BMS.Output.BatteryTemperature[1] < (g_SystemParameter.BMS.Battery.OverTemperature * 10 - 60))
			{
				v->Variable.Count.FaultLv2RecoverCnt++;
			//	v->FaultCode.DriveLimitLevel2 = 0;
			}
			else
			{
				v->Variable.Count.FaultLv2RecoverCnt = 0;
			}	
		}
		break;
        case CHARGE_TEMPERATURE_FAULT:
        {
            if(g_SystemState.State.bit.ChargeTempError == 0)
            {
                v->Variable.Count.FaultLv2RecoverCnt++;
            }
        }
        break;
        case FAULT_CELL_UNBALANCE:
        {
            if(v->Output.CellUnBalance == 0)
            {
                v->Variable.Count.FaultLv2RecoverCnt++;
            }
            else
            {
                v->Variable.Count.FaultLv2RecoverCnt = 0;
            }
        }
        break;
		
		case WARNING_BATTERY_UNDER_TEMPERATURE:
		{
			if (g_AO_BMS.Output.BatteryTemperature[0] > (g_SystemParameter.BMS.Battery.UnderTemperature * 10 + 60)
				&& g_AO_BMS.Output.BatteryTemperature[1] > (g_SystemParameter.BMS.Battery.UnderTemperature * 10 + 60))
			{
				v->Variable.Count.FaultLv2RecoverCnt++;
			//	v->FaultCode.DriveLimitLevel2 = 0;
			}
			else
			{
				v->Variable.Count.FaultLv2RecoverCnt = 0;
			}				
		}
		break;
		
		case WARNING_BMS_OVER_TEMPERATURE:			// BMS������¶ȹ��
		{
			if (g_AO_BMS.Output.BMSTemperature[0] < (g_SystemParameter.BMS.Warning.BMSOverTemperature * 10 - 120) 
                && g_AO_BMS.Output.BMSTemperature[1] < (g_SystemParameter.BMS.Warning.BMSOverTemperature * 10 - 120))
			{
				v->Variable.Count.FaultLv2RecoverCnt++;
			//	v->FaultCode.DriveLimitLevel2 = 0;
			}
			else
			{
				v->Variable.Count.FaultLv2RecoverCnt = 0;
			}					
		}
		break;
		
		case WARNING_BATTERY_NTC_FAILURE:			// ����¶�NTC��·
		{
			
#if (CONTROLLER_TARGET == BMS_EP_200_B2_1)			// ��Ӳ���汾����·����¶ȴ��Ї�

			if (g_ADC.Output.ADCResult[0] < 4000
				&& g_ADC.Output.ADCResult[1] < 4000)
			{
				v->Variable.Count.FaultLv2RecoverCnt++;
			}
			else
			{
				v->Variable.Count.FaultLv2RecoverCnt = 0;
			}
	
#else
	
			if (g_ADC.Output.ADCResult[0] < 4000)
			{
				v->Variable.Count.FaultLv2RecoverCnt++;
			}
			else
			{
				v->Variable.Count.FaultLv2RecoverCnt = 0;
			}
	
#endif			
	
		}
		break;
			
		case WARNING_BMS_NTC_FAILURE:			// BMS���¶�NTC��·
		{
			
#if (CONTROLLER_TARGET == BMS_EP_20_REV1_1_2TEMP)
			
			if (g_ADC.Output.ADCResult[1] < 4000)
			{
				v->Variable.Count.FaultLv2RecoverCnt++;
			}
			else
			{
				v->Variable.Count.FaultLv2RecoverCnt = 0;
			}

#elif (CONTROLLER_TARGET == BMS_EP_200_B2_1)		// ��Ӳ���汾BMS������¶Ȳ���LMT87���,ʹ��ADCͨ��3
	
			if (g_ADC.Output.ADCResult[2] < 4000 && g_ADC.Output.ADCResult[2] > 100)
			{
				v->Variable.Count.FaultLv2RecoverCnt++;
			}
			else
			{
				v->Variable.Count.FaultLv2RecoverCnt = 0;
			}
			
#endif
				
		}
		break;
		
		default:			// ��FaultLv2د��������������FaultLv2��Ϊ0���������޻ָ�����Ĺ����룬�����ָ�����
		{
		//	v->FaultCode.DriveLimitLevel2 = 0;		
			v->Variable.Count.FaultLv2RecoverCnt = 0;
		}
		break;
	}
	
	// 1000 * 1ms��ʱ�����FaultLv2
	if (v->Variable.Count.FaultLv2RecoverCnt >= 1000)
	{
		v->Variable.Count.FaultLv2RecoverCnt = 0;
		
		v->FaultCode.DriveLimitLevel2 = 0;
	}
}

u16	t_testMainLoop[5];
u16	t_protect[5];
u16 t_CellBalanceCheckDelay = 0;
u16 t_CellVoltageUnBalanceCnt = 0;
// ---------------------- y???????��??ms?????��?m?????? ------------------------
void Protect_SlowUpdate(Protect_structDef* v)
{
    v->Variable.Count.SlowProtectUpdateCnt++;
	// Take care: ?????��?????????��???��???��??????��????s??��???????
	if (g_SystemParameter.BMS.Battery.BatteryTemperatureCheckMode > 0)
	{
        if(g_SystemState.State.bit.ChargeOnFlag > 0 && g_Input.Output.DIN_bits.bit.KEY > 0)
        {
            if((g_AO_BMS.Output.BatteryTemperature[0] < 1000 && g_AO_BMS.Output.BatteryTemperature[0] > g_SystemParameter.BMS.Charge2.ChargeLimitTemperature1 *10)
                || (g_AO_BMS.Output.BatteryTemperature[1] < 1000 && g_AO_BMS.Output.BatteryTemperature[1] > g_SystemParameter.BMS.Charge2.ChargeLimitTemperature1 *10)
                || (g_AO_BMS.Output.BatteryTemperature[0] > -500 && g_AO_BMS.Output.BatteryTemperature[0] < g_SystemParameter.BMS.Charge2.ChargeLimitTemperature4 *10)
                || (g_AO_BMS.Output.BatteryTemperature[1] > -500 && g_AO_BMS.Output.BatteryTemperature[1] < g_SystemParameter.BMS.Charge2.ChargeLimitTemperature4 *10)
                || ((g_AO_BMS.Output.BatteryTemperature[0] > 1000 || g_AO_BMS.Output.BatteryTemperature[0] < -500) && ((g_AO_BMS.Output.BatteryTemperature[1] > 1000) || g_AO_BMS.Output.BatteryTemperature[1] < -500))
            )
            {
                if (v->Variable.Count.temp_c_cnt > 10000)
                {
                    g_SystemState.State.bit.ChargeTempError = 1;
                    Protect_SetFaultCodeLv2(&g_Protect,CHARGE_TEMPERATURE_FAULT);
                } else v->Variable.Count.temp_c_cnt++;
            } else {
                if(v->Variable.Count.temp_c_cnt)v->Variable.Count.temp_c_cnt--;
                else g_SystemState.State.bit.ChargeTempError = 0;
            }
        } else {
            if(g_AO_BMS.Output.BatteryTemperature[0] > 1000)
            {
                v->Variable.Count.temp1_cnt++;
                if (v->Variable.Count.temp1_cnt > 10000)
                {
                    v->Variable.Count.temp1_flg = 1;
                }
            } else if (g_AO_BMS.Output.BatteryTemperature[0] > (g_SystemParameter.BMS.Battery.OverTemperature * 10))
            {
                v->Variable.Count.temp1_cnt++;
                if (v->Variable.Count.temp1_cnt > 10000)
                {
                    Protect_SetFaultCodeLv1(v, FAULT_BAT_OT);
                }
            } else if (g_AO_BMS.Output.BatteryTemperature[0] < -500)
            {
                v->Variable.Count.temp1_cnt++;
                if (v->Variable.Count.temp1_cnt > 10000)
                {
                    v->Variable.Count.temp1_flg = 1;
                }
            } else if (g_AO_BMS.Output.BatteryTemperature[0] < (g_SystemParameter.BMS.Battery.UnderTemperature * 10))
            {
                v->Variable.Count.temp1_cnt++;
                if (v->Variable.Count.temp1_cnt > 10000)
                {
                    Protect_SetFaultCodeLv1(v, FAULT_BAT_UT);
                }
            } else {
                v->Variable.Count.temp1_cnt = 0;
                v->Variable.Count.temp1_flg = 0;
            }
            
            if(g_AO_BMS.Output.BatteryTemperature[1] > 1000)
            {
                v->Variable.Count.temp2_cnt++;
                if (v->Variable.Count.temp2_cnt > 10000 && v->Variable.Count.temp1_flg)
                {
                    Protect_SetFaultCodeLv1(v, WARNING_BATTERY_NTC_FAILURE);
                }
            } else if (g_AO_BMS.Output.BatteryTemperature[1] > (g_SystemParameter.BMS.Battery.OverTemperature * 10))
            {
                v->Variable.Count.temp2_cnt++;
                if (v->Variable.Count.temp2_cnt > 10000)
                {
                    Protect_SetFaultCodeLv1(v, FAULT_BAT_OT);
                }
            } else if (g_AO_BMS.Output.BatteryTemperature[1] < -500)
            {
                v->Variable.Count.temp2_cnt++;
                if (v->Variable.Count.temp2_cnt > 10000 && v->Variable.Count.temp1_flg)
                {
                    Protect_SetFaultCodeLv1(v, WARNING_BATTERY_NTC_FAILURE);
                }
            } else if (g_AO_BMS.Output.BatteryTemperature[1] < (g_SystemParameter.BMS.Battery.UnderTemperature * 10))
            {
                v->Variable.Count.temp2_cnt++;
                if (v->Variable.Count.temp2_cnt > 10000)
                {
                    Protect_SetFaultCodeLv1(v, FAULT_BAT_UT);
                }
            } else {
                v->Variable.Count.temp2_cnt = 0;
            }
        }
	}

#if (CONTROLLER_TARGET == BMS_EP_20_REV1_1_2TEMP || CONTROLLER_TARGET == BMS_EP_200_B2_1)		// ��Ӳ���汾֧��BMS������¶Ȳɖ�
			
	// ��������BMS�¶ȼ��ʹ��ʱ���ż��BMS�¶�
	if (g_SystemParameter.BMS.Warning.BMSOTemperatureCheckEnable > 0)			// BMS������¶ȼ��ʹ�v
	{
		if (g_AO_BMS.Output.BMSTemperature[0] > (g_SystemParameter.BMS.Warning.BMSOverTemperature * 10))
		{
			v->Variable.Count.BMSOverTemperatureCnt++;
			
			if (v->Variable.Count.BMSOverTemperatureCnt > 5000)
			{
				v->Variable.Count.BMSOverTemperatureCnt = 0;
				
				Protect_SetFaultCodeLv2(v, WARNING_BMS_OVER_TEMPERATURE);
			}			
		}
        else if (g_AO_BMS.Output.BMSTemperature[0] > (g_SystemParameter.BMS.Warning.BMSOverTemperature * 10 - 50))
        {

			v->Variable.Count.BMSOverTemperatureCnt++;
			
			if (v->Variable.Count.BMSOverTemperatureCnt > 5000)
			{
				v->Variable.Count.BMSOverTemperatureCnt = 0;
				Protect_SetFaultCodeLv2(v, WARNING_BMS_OVER_TEMPERATURE);
			}	
        }
        else if (g_AO_BMS.Output.BMSTemperature[0] > (g_SystemParameter.BMS.Warning.BMSOverTemperature * 10 - 100))
        {
			v->Variable.Count.BMSOverTemperatureCnt++;
			
			if (v->Variable.Count.BMSOverTemperatureCnt > 5000)
			{
				v->Variable.Count.BMSOverTemperatureCnt = 0;
                Protect_SetFaultCodeLv2(v, WARNING_BMS_OVER_TEMPERATURE);
			}	
        }
	}	
					// CONTROLLER_TARGET == BMS_EP_20_REV1_1_2TEMP
#elif (CONTROLLER_TARGET == BMS_EP_ALPHA)		// ��Ӳ���汾֧��BMS������¶Ȳɖ�
                
        // ��������BMS�¶ȼ��ʹ��ʱ���ż��BMS�¶�
        if (g_SystemParameter.BMS.Warning.BMSOTemperatureCheckEnable > 0)           // BMS������¶ȼ��ʹ�v
        {
            if (g_AO_BMS.Output.BMSTemperature[0] > (g_SystemParameter.BMS.Warning.BMSOverTemperature * 10)
                || g_AO_BMS.Output.BMSTemperature[1] > (g_SystemParameter.BMS.Warning.BMSOverTemperature * 10))
            {
                v->Variable.Count.BMSOverTemperatureCnt++;
                
                if (v->Variable.Count.BMSOverTemperatureCnt > 5000)
                {
                    v->Variable.Count.BMSOverTemperatureCnt = 0;
                    
                    Protect_SetFaultCodeLv2(v, WARNING_BMS_OVER_TEMPERATURE);
                }           
            }
            else if (g_AO_BMS.Output.BMSTemperature[0] > (g_SystemParameter.BMS.Warning.BMSOverTemperature * 10 - 50)
                || g_AO_BMS.Output.BMSTemperature[1] > (g_SystemParameter.BMS.Warning.BMSOverTemperature * 10 - 50))
            {
    
                v->Variable.Count.BMSOverTemperatureCnt++;
                
                if (v->Variable.Count.BMSOverTemperatureCnt > 5000)
                {
                    v->Variable.Count.BMSOverTemperatureCnt = 0;
                    Protect_SetFaultCodeLv2(v, WARNING_BMS_OVER_TEMPERATURE);
                }   
            }
            else if (g_AO_BMS.Output.BMSTemperature[0] > (g_SystemParameter.BMS.Warning.BMSOverTemperature * 10 - 100)
                || g_AO_BMS.Output.BMSTemperature[1] > (g_SystemParameter.BMS.Warning.BMSOverTemperature * 10 - 100))
            {
                v->Variable.Count.BMSOverTemperatureCnt++;
                
                if (v->Variable.Count.BMSOverTemperatureCnt > 5000)
                {
                    v->Variable.Count.BMSOverTemperatureCnt = 0;
                    Protect_SetFaultCodeLv2(v, WARNING_BMS_OVER_TEMPERATURE);
                }   
            }
        }   
        
#endif				// CONTROLLER_TARGET == BMS_EP_20_REV1_1_2TEMP

			   
	// �ŵ�������󣬳�����·����                               380.1000
	if (g_AO_SH36730x0.Output.BatteryCurrent < -((s32)g_SystemParameter.BMS.Battery.ShortCutDownCurrent * 1000)
		|| g_AO_SH36730x0.Output.BatteryCurrent > ((s32)g_SystemParameter.BMS.Battery.ShortCutDownCurrent * 1000))
	{
		Protect_SetFaultCodeLv1(v, FAULT_BQ769_SCD);
	}
	
	//�ŵ�������󣬳�����������
	if (g_AO_SH36730x0.Output.BatteryCurrent < -((s32)g_SystemParameter.BMS.Battery.OverCutDownCurrent * 1000)
		|| g_AO_SH36730x0.Output.BatteryCurrent > (s32)g_SystemParameter.BMS.Battery.OverCutDownCurrent * 1000)
	{
		v->Variable.Count.OverCurrentCnt++;
		if (v->Variable.Count.OverCurrentCnt > 1000)			// ???????��?1s
		{			
			Protect_SetFaultCodeLv1(v, FAULT_BQ769_OCD);
			
			v->Variable.Count.OverCurrentCnt = 0;
		}
	}	
	else
	{
		v->Variable.Count.OverCurrentCnt = 0;
	}
    
	// ���������󣬳���������Ƶ�����������һ��ʱ�����ϣ��������������󱨾�
    // ע�⣬��������������ʱ�������ù�С����ֹ���������ƶ�ʱ�����˱���
	if (g_AO_SH36730x0.Output.BatteryCurrent > (s32)g_SystemParameter.BMS.Charge.ChargeOverCurrent * 1000)//???��??0A
	{
		v->Variable.Count.ChargeOverCurrentCnt++;
		if (v->Variable.Count.ChargeOverCurrentCnt > ((s32)g_SystemParameter.BMS.Charge.ChargeOverCurrentDelay * 1000))	 // ?????��?????60s
		{			
			Protect_SetFaultCodeLv1(v, FAULT_BAT_CHARGE_OVER_CURRENT);
			
			v->Variable.Count.ChargeOverCurrentCnt = 0;
		}	
	}
	else
	{
		v->Variable.Count.ChargeOverCurrentCnt = 0;
	}    
	
	// �����ѭ���Ƿ�����
	if (g_SystemState.Output.SystemMainLoopCount == v->Variable.Count.MainLoopCountOld)
	{
		v->Variable.Count.MainLoopSameCount++;
		
		// for test
		if (v->Variable.Count.MainLoopSameCount > t_testMainLoop[0])
		{
			t_testMainLoop[0] = v->Variable.Count.MainLoopSameCount;
		}
			
		if (v->Variable.Count.MainLoopSameCount > 1000)
		{
			v->Variable.Count.MainLoopSameCount = 0;
			
			g_SystemState.State.bit.IWDGReloadEnable = 0;	// ����ѭ��δִ�У���Ҫǿ��ֹͣι�����������Ź���λϵͳ
																
			Protect_SetFaultCodeLv1(v, FAULT_SOFTWARE_FAILURE);			
		}
	}
	else
	{
		v->Variable.Count.MainLoopSameCount = 0;
	}
	
	v->Variable.Count.MainLoopCountOld = g_SystemState.Output.SystemMainLoopCount;
	    // �����ѹѹ�������
    if (g_AO_BMS.Output.BatteryCurrent < 5000 && g_AO_BMS.Output.BatteryCurrent > -5000)        // +-5A����
    {     
        t_CellBalanceCheckDelay++;
        
        if (t_CellBalanceCheckDelay >= 5000)        // ��ʱ5s
        {       
            t_CellBalanceCheckDelay = 5000;
            
            if (g_AO_SH36730x0.Output.SingleMaxVoltage < 3400 && g_AO_SH36730x0.Output.SingleMinVoltage > 3050)        // �����ѹ��00mV����
            { 
                if((g_AO_SH36730x0.Output.SingleMaxVoltage - g_AO_SH36730x0.Output.SingleMinVoltage) > 400)
                {
                    t_CellVoltageUnBalanceCnt++;
                
                    if (t_CellVoltageUnBalanceCnt >= 5000)      // ��ʱ3s
                    {
                        t_CellVoltageUnBalanceCnt = 0;
                        
                        v->Output.CellUnBalance = 1;
                        
                        Protect_SetFaultCodeLv2(v, FAULT_CELL_UNBALANCE);	    // �����ѹ������    
                    }
                }
                else
                {
                    t_CellVoltageUnBalanceCnt = 0;
                    
                    if ((g_AO_SH36730x0.Output.SingleMaxVoltage - g_AO_SH36730x0.Output.SingleMinVoltage) < 350)        //ѹ��150mv??
                    {
                        v->Output.CellUnBalance = 0;          // ֱ�ӻָ������ٱ�����
                    }
                }
            }
            else
            {
                if((g_AO_SH36730x0.Output.SingleMaxVoltage - g_AO_SH36730x0.Output.SingleMinVoltage) > 300)
                {
                    t_CellVoltageUnBalanceCnt++;
                
                    if (t_CellVoltageUnBalanceCnt >= 5000)      // ��ʱ3s
                    {
                        t_CellVoltageUnBalanceCnt = 0;
                        
                        v->Output.CellUnBalance = 1;
                        //??????
                        Protect_SetFaultCodeLv2(v, FAULT_CELL_UNBALANCE);	        // ???????   
                    }
                }
                else
                {
                    t_CellVoltageUnBalanceCnt = 0;
                    
                    if ((g_AO_SH36730x0.Output.SingleMaxVoltage - g_AO_SH36730x0.Output.SingleMinVoltage) < 250)        // ѹ��150mV����
                    {
                        v->Output.CellUnBalance = 0;        //ֱ�ӻָ������ٱ���
                    }
                }
            }
        }
    }
    else
    {
        t_CellBalanceCheckDelay = 0;
        v->Output.CellUnBalance = 0;
    }
    
	// ����ȥǮ���������
	Protect_ClearFaultCodeLv2(v);
}

// ----------------------- ��ȡ���ϴ��룬���޹��ϣ��򷵻�0 -----------------------------
u16	Protect_GetFaultCode(Protect_structDef* v)
{
	if (v->FaultCode.SystemLockLevel0 > 0)
	{
		return v->FaultCode.SystemLockLevel0;
	}
	else if (v->FaultCode.OutputDisableLevel1 > 0)
	{
		return v->FaultCode.OutputDisableLevel1;
	}
	else
	{
		return 0;
	}
}

// ----------------------- ��ȡ������� -----------------------------
u16	Protect_GetWarningCode(Protect_structDef* v)
{
	if (v->FaultCode.DriveLimitLevel2 > 0)
	{
		return v->FaultCode.DriveLimitLevel2;
	}
	else
	{
		return 0;
	}
}

// ---------------------------- ������� -----------------------------------
//����������Դ��ڣ��򷵻ع���δ����������й�����������������ϴ��벢���ع��������־λ
u16	Protect_ClearFault(Protect_structDef* v)
{
	u16	l_NoFaultFlag = 0;
	
	l_NoFaultFlag = 1;
	
	switch (Protect_GetFaultCode(v))
	{
/*		case FAULT_EEPROM_FAILURE:		// ��ֹ�ָ�
		case FAULT_SOFTWARE_FAILURE:
		case ERROR_ILLEGAL_PARAMETER:
		case FAULT_OSC:
		case SYS_STAT_NOT_CLEAR:
		case FAULT_START_FAIL:
		case FAULT_BQ769_FAIL:
		case FAULT_BQ769_OVRD_ALERT:
		case FAULT_BQ769_I2C_FAIL:
		case BQ769_UPDATE_TOO_SLOW:
		case BQ769_UPDATE_TOO_SLOW2:
		case BQ769_CRC:
		case FLASH_WRITE_FAIL:
		case FLASH_WRITE_FORBID:
		case FLASH_WRITE_CHECK_FAIL:
		case EEPROM_PARAMETER_LRC_FAIL:
		case EEPROM_PARAMETER_WRONG:
		case EEPROM_COMMAND_FAIL:
		case EEPROM_COMMAND_OVERTIME:
		case EEPROM_COMMAND_OVER_FLOW:			
		{
			// ��ֹ�ָ�
			l_NoFaultFlag = 1;
		}
		break;*/
		
//		case FAULT_BQ769_UV:
//		{
//			if (g_AO_SH36730x0.Output.SingleMinVoltage > g_SystemParameter.BMS.Battery.CellUnderVoltage)
//			{
//				l_NoFaultFlag = 0;
//			}
//		}
//		break;
//		
//		case FAULT_BQ769_OV:
//		{
//			if ( g_AO_SH36730x0.Output.SingleMaxVoltage < g_SystemParameter.BMS.Battery.CellOverVoltage)
//			{
//				l_NoFaultFlag = 0;
//			}
//		}
//		break;	

//		case FAULT_BQ769_SCD:
//		{
//			if (g_AO_SH36730x0.SH36730x0Register.FLAG1.bit.SC== 0)                //?????��??????
//			{
//				l_NoFaultFlag = 0;		
//			}					
//		}
//		break;
//		
//		case FAULT_BQ769_OCD://???��  ??????MOS
//		{
////			if (g_AO_SH36730x0.SH36730x0Register.SYS_STAT.bit.OCD == 0)  //-------SH36730��?????����?????m---------
//			if((g_AO_SH36730x0.Output.BatteryCurrent<0)&&((-g_AO_SH36730x0.Output.BatteryCurrent)> (g_SystemParameter.BMS.Battery.OverCutDownCurrent*1000)))
//			{
//				l_NoFaultFlag = 0;		
//			}				
//		}
//		break;
//		
//		case FAULT_BAT_OT:
//		case FAULT_BAT_UT:
//		{
//			if (g_AO_BMS.Output.BatteryTemperature[0] < (g_SystemParameter.BMS.Battery.OverTemperature * 10 - 50)
//				&& g_AO_BMS.Output.BatteryTemperature[0] > (g_SystemParameter.BMS.Battery.UnderTemperature * 10 + 50))
//			{
//				l_NoFaultFlag = 0;	
//			}
//		}
//		break;
		
/*		case WARNING_BATTERY_OVER_TEMPERATURE:	//
		case WARNING_BATTERY_UNDER_TEMPERATURE:
		case WARNING_BMS_OVER_TEMPERATURE:
		case WARNING_BATTERY_NTC_FAILURE:
		case WARNING_BMS_NTC_FAILURE:
		{
			
		}
		break;*/
	}
	

    // ����ȫ�������������ϱ�־λ
    // BMS�������й��ϻָ�
	if (l_NoFaultFlag == 0)
	{
		v->FaultCode.SystemLockLevel0 = 0;
		v->FaultCode.OutputDisableLevel1 = 0;
		
		return 0;	//���������	
	}
	else
	{
		return 1;		//����δ���
	}
	
	
}


