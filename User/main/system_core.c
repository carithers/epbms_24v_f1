/* ==================================================================================

 File name:     system_core.c
 Originator:    BLJ
 Description:   ϵͳ�����ļ���ȫ���������ӡ�����ʵ�������ж�ѭ������ԭvehicle.c

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 11-24-2014     Version 0.0.1           ���Թ���
-----------------------------------------------------------------------------------*/

#include "system_core.h"                    	// ϵͳ����ͷ�ļ�
#include "CRC8.h"								// CRCУ��ֵģ��
#include <math.h>

// ----------------------------- �ڲ��ɵ��ú��� ------------------------------------
void System_10msTimerUpdate(void);						// 10ms����ʱ�����º���

// ----------------------------- ���ݱ����ṹ��ʵ���� -------------------------------
//CAN_structDef       		g_CAN;                      // CANģ��ṹ��ʵ����
//I2C_BQ769x0_structDef		g_I2C_BQ769x0;				// I2C EEPROMģ��
I2C_BQ769x0_structDef		g_I2C_SH36730x0;		    // I2C EEPROMģ��
flash_EEPROM_structDef		g_flash_EEPROM;				// flashģ��EEPROM
USART_MODBUS_structDef		g_USART_MODBUS;				// ����MODBUSͨ��ģ��
Protect_structDef			g_Protect;					// Protectģ��

Input_structDef				g_Input;					// ��������ģ��
Output_structDef			g_Output;					// ��������ģ��
ADC_structDef				g_ADC;						// ADC����ģ��
LED_Flicker_structDef		g_LED_Flicker;				// LED��˸ģ��
//Contactor_structDef			g_Contactor;				// ���Ӵ�������ģ��

Comm_structDef      		g_communication;            // �豸��ͨ�����ݽṹ��ʵ����

SYSTEM_STATE_structDef		g_SystemState;				// ϵͳ״̬
SYSTEM_PARAMETER_structDef	g_SystemParameter;			// ϵͳ����
SYSTEM_MONITOR_structDef	g_SystemMonitor;			// ϵͳ����ֵ
SYSTEM_RECORD_structDef		g_SystemRecord;				// �豸���м�¼
FAULT_HISTORY_structDef		g_SystemFaultHistory;		// ϵͳ���ϼ�¼//���ϼ�¼


// -------------------------------- ϵͳȫ�ֱ��� ----------------------------------


// -------------------------------- �ڲ��ֲ����� ----------------------------------
u16	l_10msTimerFlag = 0;						// 10msʱ�����±�־λ
u16	l_200msTimerFlag = 0;						// 200msʱ�����±�־λ
u16	l_1sTimerFlag = 0;							// 1sʱ�����±�־λ

// -------------------------------- �ڲ���ʱ���� ----------------------------------

u16 t_ADC[5];

//u16	l_KEYState = 0;

u16 t_LCDtest[5] ={0, 0, 0, 0, 0};

u32	t_EEPROMtest[5] = {0, 0x09, 1, 0, 0};
u16	t_EEPROMtest2[20];
u16	t_EEPROMtest3[20];

u8	t_EERPOMCRC[5] = {0x10, 0x09, 0xBD, 0, 0};
u8	t_BQTest[5];

u8	t_DSGEnable[5];



u16	l_FaultSIGSendFlag = 0;

u16	t_flashEEPROMtest[5] = {0, 0, 0, 0, 0};

u16	t_OutTest[5];
u16	t_CANtest3[5];

u16	l_ChargeCurrentLimitDivision = 1;		// ���������Ʒ�Ƶ������2����0.5C
u16	l_ChargeCurrentLimit = 0;				// ����������ֵ����λ0.1A

s32	t_SingleMaxVoltage = 3900;

u16	l_CANNoChargerStateCount = 0;
//u16	l_ChargerFaultFlag = 0;

u16 t_LED[5];

u16 t_OutputCmdDisMatchCnt = 0;
u16 t_ResetAsk = 0;
u16 t_ResetCnt = 0;

u16 t_ttRecord[5];
u16 t_TempLowError = 0;

u16 g_ChargerOfflineCnt = 0;
u16 g_ChargerOnlineFlag = 0;
u16 l_ChargeFaultRecoverDelay = 0;
u16 test_rebuffer[225];
u16 t_BeepDelay = 0;

//u16 boot_flag=0;//==============================================================����
// ----------------------------- ϵͳ��ʱ�жϣ�1ms --------------------------------
void System_1msInterrupt(void)
{
    
////=============================IICͨѶ����=======================================	

//	I2C_BQ769x0_Process(&g_I2C_SH36730x0);//��д�¼�
//	I2C_BQ769x0_SIG_Update();            //���¶�д�¼����

////===============================================================================
	g_SystemState.Output.System1msInterruptCount++;
	
//	// �˴�����һ������ָ������жϣ���ʱ�жϣ������ж�
//	if (g_SystemParameter.BMS.Charge.ChargerCommEnable > 0) //�޴˹���
//	{
//		l_CANNoChargerStateCount++;				
//		if (l_CANNoChargerStateCount > 1000)
//		{
//			l_CANNoChargerStateCount = 1000;
//			
//			// ��ʱ��1s��δ�յ���������������֡
//            g_ChargerOnlineFlag = 0;			
//		}
//        
//        // ����δ������KEYΪ��Чλʱ������������������ϱ�־λ
//        // ��ֹ���������������������KEYδ�Ͽ��������Ӧ���ж��������ʱ����CAN���߶��ߣ�������������ϣ��ᵼ�µ�������������
//        // ����ȵ�KEY�Ͽ���������������ϣ���֤���KEY����ʹ�ܺ󣬵�ز������ٴ����
//        if (g_ChargerOnlineFlag == 0 && g_Input.Output.DIN_bits.bit.KEY == 0)
//        {
//            g_SystemState.State.bit.ChargerFaultFlag = 0;
//        } 
//        if(g_AO_SH36730x0.Output.BatteryCurrent >= g_SystemParameter.BMS.Charge2.ChargeCurrent)
//        {
//            if(g_AO_BMS.Output.BatteryTemperatureLow < g_SystemParameter.BMS.Charge2.ChargeLimitTemperature3*10)//10��
//            {
//                g_AO_BMS.State.OutputAllow = 0; 
//            }
//            else
//            {
//                g_AO_BMS.State.OutputAllow = 1;
//            }
//        }
//	}
//	else
//	{
//		l_CANNoChargerStateCount = 0;
//	}		
//��λ����¼ָ���ȡ
//===========================================================================================
//		//30msִ��һ��
//	if(g_SystemState.Output.System1msInterruptCount%30 == 13)//
//    {
//        //��λ����¼��Ӧָ��				
//        if(g_communication.BootCMD.BYTE0==0x3A && g_communication.BootCMD.BYTE1==0x00 \
//            && g_communication.BootCMD.BYTE2==0x00 && g_communication.BootCMD.BYTE4==0x00 \
//        && g_communication.BootCMD.BYTE5==0x00 && g_communication.BootCMD.BYTE6==0xF0)
//        {
//			boot_flag++;//===============================
//	  //���¸���һ�ݳ�����ƫ�Ʋ��ԣ��Ƿ���Խ�����λ��ָ�����
//          
//        }
//				 
//    }
	
//============================================================================================
	
	// ϵͳ����ʱ��,ע������ȡ��ֵͬ����ֹʱ����ͬʱ����	
	if ((g_SystemState.Output.System1msInterruptCount % 200) == 0)//
	{
		l_200msTimerFlag = 1;	        
		// ϵͳ��ʱͨ�Ÿ���,������ʱ����㲥���״̬
		// SystemComm_Update();		
	}
	if ((g_SystemState.Output.System1msInterruptCount % 1000) == 5)//
	{
		l_1sTimerFlag = 1;
		
		// Take care: 3�Ŷ�ʱ��ʱ����1s
		QF_tickXISR(3U);          				// process time events at rate 2 	
		// --------------------- 1s����һ��ϵͳ״̬ -----------------------
		SystemMonitor_Update();			
        
	}	
	
    if(g_SystemParameter.BMS.Fan.FanMode == 2)//�͵�������
    {
        // �ж��Ƿ���þ���
        if((g_AO_BMS.Output.SOC < g_SystemParameter.BMS.Warning.LowSOCBeep * 10)//17%
            && (g_Input.Output.DIN_bits.bit.KEY > 0))
        {
            // �������ͣ�����������
            Output_BUZZER_Update(&g_Output, 1);
            g_Output.State.bit.FanEnable = 1;
        }
        else
        {
            t_BeepDelay = 0;
            g_Output.State.bit.FanEnable = 0;
            Output_BUZZER_Update(&g_Output, 0);
        }  
    }
    else
    {
        Output_BUZZER_Update(&g_Output, 0);
        g_Output.State.bit.FanEnable = 0;
    }	

	// Ĭ�����в�Ʒ֧�ִ��ڵ��ԣ�����֧�־ɰ��޴��ڵ��Թ���Ӳ��
//#if (CONTROLLER_TARGET == BMS_EP_A5 || BMS_EP_20_REV1_1_2TEMP || CONTROLLER_TARGET == BMS_EP_200_A1 || CONTROLLER_TARGET == BMS_EP_200_A2 || CONTROLLER_TARGET == BMS_XINXIN_A2 || CONTROLLER_TARGET == BMS_EP_200_B1 || CONTROLLER_TARGET == BMS_EP_200_B2_1)				// EP BMS A3�汾�������ڵ��Թ���
	
	// ����MODBUSģ��ʱ��������������1msʱ����
	USART_MODBUS_TimeTick(&g_USART_MODBUS);

	if ((g_SystemState.Output.System1msInterruptCount % 5) == 0)	
	{
		// ����MODBUSģ����̺���
		USART_MODBUS_Process(&g_USART_MODBUS);
	}
	
//#endif						
	
	// ��������ģ����º���
	Input_Update(&g_Input);//����Կ��KEY������������״̬���
    
    // ADC������1msһ��
    ADC_SlowUpdate(&g_ADC);//����ע�ͷ����ϵ�˲����һ�����ܻᵼ�£�ADC������

	// ���ٱ�������
	Protect_SlowUpdate(&g_Protect);
	
#if (CONTROLLER_TARGET == BMS_EP_200_A1 || CONTROLLER_TARGET == BMS_EP_200_A2 || CONTROLLER_TARGET == BMS_EP_200_B1 || CONTROLLER_TARGET == BMS_EP_200_B2_1)
	
	// ���½Ӵ�����������������Ӵ����汾��Ч
	if (g_Protect.FaultCode.SystemLockLevel0 == 0 
		&& g_Protect.FaultCode.OutputDisableLevel1 == 0 
		&& g_Input.Output.DIN_bits.bit.DSG == 1 
		&& g_Input.Output.DIN_bits.bit.CHG == 1)
	{
		g_Contactor.Input.ContactorEnable = 1;		
	}
	else
	{
		g_Contactor.Input.ContactorEnable = 0;
	}
		
	Contactor_Update(&g_Contactor);
	
#endif						// CONTROLLER_TARGET == BMS_EP_200_A1

// LEDˢ��
	System_LED_Update();
//	Output_LED_RED_Update(&g_Output, t_LED[0]);
//	Output_LED_GREEN_Update(&g_Output, t_LED[1]);
//	Output_LED_BLUE_Update(&g_Output, t_LED[2]);
	
    // For test: ���ӹ��ϼ�⣬��ŵ�ָ���뷴����һ��ʱ����
    if (g_AO_SH36730x0.State.DSGControl != g_AO_SH36730x0.SH36730x0Register.SCONF2.bit.DSG_C 
        || g_AO_SH36730x0.State.CHGControl != g_AO_SH36730x0.SH36730x0Register.SCONF2.bit.CHG_C)//CHG_C
    {
        t_ttRecord[0] = g_AO_SH36730x0.State.DSGControl;
        t_ttRecord[1] = g_AO_SH36730x0.SH36730x0Register.SCONF2.bit.CHG_C;
        t_ttRecord[2] = g_AO_SH36730x0.State.CHGControl;
        t_ttRecord[3] = g_AO_SH36730x0.SH36730x0Register.SCONF2.bit.DSG_C;        
        
        t_OutputCmdDisMatchCnt++;
        if (t_OutputCmdDisMatchCnt > 3000) //��⵽3000�β�һ��
        {
            Protect_SetFaultCodeLv0(&g_Protect, FAULT_BQ769_CMD_DISMATCH);
                       
            t_ResetAsk = 1;
            
            t_OutputCmdDisMatchCnt = 0;
        }
    }
    else
    {
        if (t_OutputCmdDisMatchCnt > 0)
        {
            t_OutputCmdDisMatchCnt--;
        }
    }
    
	if (g_SystemState.State.bit.IWDGReloadEnable > 0)			// ����ι��
	{
        if (t_ResetAsk > 0)
        {
            t_ResetCnt++;
            
            if (t_ResetCnt > 3000)
            {
                t_ResetAsk = 0;
                t_ResetCnt = 0;
                
                g_SystemState.State.bit.IWDGReloadEnable = 0;
            }
        }
        
		IWDG_ReloadCounter();
	}
    else
    {
        t_ResetAsk = 0;
        t_ResetCnt = 0;        
    }
			

}
u16 l_BatteryTemperatureCalcCnt = 0;
s32 t_TempHi = 0;
s32 t_TempLow = 0;
u16 l_LEDFlashCnt = 0;
u16 system_reset_cnt = 0;
// ----------------------------- ϵͳ��ѭ������ -------------------------------------
void System_MainLoop(void)
{		
	g_SystemState.Output.SystemMainLoopCount++;			// ��ѭ��ִ�д���++
    
//=============================IICͨѶ����=======================================	

	I2C_BQ769x0_Process(&g_I2C_SH36730x0);//��д�¼�
	I2C_BQ769x0_SIG_Update();            //���¶�д�¼����

//===============================================================================

	// �ⲿKEY������¼�����
	if (g_SystemState.State.bit.KEYState == 0)//
	{
		// KEYʹ�ܣ��ҳ������ϱ�־λδ��λ,(��Կ��������޹���)
		if (g_Input.Output.DIN_bits.bit.KEY > 0 && g_SystemState.State.bit.ChargerFaultFlag == 0)
		{			
			QACTIVE_POST((QActive *)&g_AO_BMS, DIN_SIG, 1);	//ע�����һ������Ϊ1
			
			g_SystemState.State.bit.KEYState = g_Input.Output.DIN_bits.bit.KEY;//
		}
	}
	else
	{
		// KEY��ʹ�ܣ���������ϱ�־λ��λ
		if (g_Input.Output.DIN_bits.bit.KEY == 0 || g_SystemState.State.bit.ChargerFaultFlag > 0)
		{
		//	g_SystemMonitor.System.Reverse2.Data[15]++;
			
			QACTIVE_POST((QActive *)&g_AO_BMS, DIN_SIG, 0);	//ע�����һ������Ϊ0
			
			g_SystemState.State.bit.KEYState = 0;
		}		
	}    
	
	
	// flashģ��EEPROM�¼�����
    flash_EEPROM_SIG_Update();
	
	// ϵͳ���Ը�����غ���
	Debuger_Update();
	
	if (l_200msTimerFlag > 0)			// 200ms��ʱʱ��
	{
		l_200msTimerFlag = 0;
       // g_communication.Test1.Byte[0] = (g_AO_BQ769x0.Output.ExternalTemperature[1] >> 8) & 0x00FF;
       // g_communication.Test1.Byte[1] = (g_AO_BQ769x0.Output.ExternalTemperature[1] & 0x00FF);
       // CAN_Send(&g_CAN, 11);
		// ���յ�����д��EEPROM�������ʱԼ1s����ִ������
		if (g_SystemState.Variable.ParameterWriteAsk > 0)
		{
			if (g_SystemState.Variable.ParameterWriteDelay++ > 5)//��ʱд��
			{		
				// ����EEPROMд���¼�
				QACTIVE_POST((QActive *)&g_AO_EEPROM, EEPROM_WRITE_SIG, EEPROM_BLOCK_PARAMETER);	
				
				g_SystemState.Variable.ParameterWriteDelay = 0;
				g_SystemState.Variable.ParameterWriteAsk = 0;
			}
		}
		else
		{
			g_SystemState.Variable.ParameterWriteDelay = 0;
		}
        
		// 200ms����һ�ι�����Ϣ
		if (Protect_GetFaultCode(&g_Protect) > 0)//����
		{
			QACTIVE_POST((QActive *)&g_AO_BMS, FAULT_SIG, Protect_GetFaultCode(&g_Protect));
		} 

        if (Protect_GetFaultCode(&g_Protect) == 0
            && Protect_GetWarningCode(&g_Protect) == 0)
        {
            // �޹���ʱ2s��˸һ��
            l_LEDFlashCnt++;
            system_reset_cnt = 0;
            
            if (l_LEDFlashCnt >= 5)
            {
                // ���Ե���˸
                if (g_Output.State.bit.LED_RED == 0)
                {
                    Output_LED_RED_Update(&g_Output, 1);
                }
                else
                {
                    Output_LED_RED_Update(&g_Output, 0);
                }
                
                l_LEDFlashCnt = 0;
            }
        }
        else
        {
            if(g_Input.Output.DIN_bits.bit.KEY == 0 &&  Protect_GetFaultCode(&g_Protect) >0)
            {
                if(system_reset_cnt++ > 100)
                    NVIC_SystemReset(); //�����ⳤʱ��û��ûԿ���������λ
            }
            // �й���ʱ0.4s��˸һ��
            // ���Ե���˸
            if (g_Output.State.bit.LED_RED == 0)
            {
                Output_LED_RED_Update(&g_Output, 1);
            }
            else
            {
                Output_LED_RED_Update(&g_Output, 0);
            }            
        }  
		// ����ʹ�õ���¶�NTC�ͺ�Ϊ103AT��Res = 10K��B = 3435
		// ADC�����¶ȣ�200msһ�μ���  
        // ��ذ����¶ȼ�⣬ʹ����· 
        ADC_SlowUpdate(&g_ADC);
        
         if(g_SystemParameter.BMS.Battery.BatteryTemperatureCheckMode > 0)
         {
             l_BatteryTemperatureCalcCnt++;
             if (l_BatteryTemperatureCalcCnt >= g_SystemParameter.BMS.Battery.BatteryTemperatureCheckMode
                 || l_BatteryTemperatureCalcCnt >= 8)
             {
                 l_BatteryTemperatureCalcCnt = 0;
                 
                 g_AO_BMS.Output.BatteryTemperatureHi = t_TempHi;
                 g_AO_BMS.Output.BatteryTemperatureLow = t_TempLow;    
                 t_TempHi = -1000;
                 t_TempLow = 1000;    
             }
                 
             // NTC�ͺţ�103AT
             g_AO_BMS.Output.BatteryTemperature[l_BatteryTemperatureCalcCnt] = (s32)10.0 / (((log(g_ADC.Output.BatteryNTCRes[l_BatteryTemperatureCalcCnt]) - log(10000))/3435) + 1.0/298.15) - 2730;       
             
             if (g_AO_BMS.Output.BatteryTemperature[l_BatteryTemperatureCalcCnt] > t_TempHi)
             {
                 t_TempHi = g_AO_BMS.Output.BatteryTemperature[l_BatteryTemperatureCalcCnt];
             }
             
             if (g_AO_BMS.Output.BatteryTemperature[l_BatteryTemperatureCalcCnt] < t_TempLow)
             {
                 t_TempLow = g_AO_BMS.Output.BatteryTemperature[l_BatteryTemperatureCalcCnt];
             }  
             if (g_AO_BMS.Output.BatteryTemperature[l_BatteryTemperatureCalcCnt] < -500)
             {
                 Protect_SetFaultCodeLv2(&g_Protect, WARNING_BATTERY_NTC_FAILURE);
             }   
        
         }
         else
         {
            g_AO_BMS.Output.BatteryTemperature[0] = 0;
            g_AO_BMS.Output.BatteryTemperature[1] = 0;
         }
			
#if (CONTROLLER_TARGET == BMS_EP_ALPHA)

       // BMS�������¶ȼ�� ʹ����·
        if (g_SystemParameter.BMS.Warning.BMSOTemperatureCheckEnable > 0)           // BMS������¶ȼ��ʹ�ܣ�һ����·
        {
            // 200ms����һ��
            // NTC�ͺţ�Res = 10K�� B = 3950
            g_AO_BMS.Output.BMSTemperature[0] = (s32)10.0 / (((log(g_ADC.Output.BalanceBoardNTCRes[0]) - log(10000))/3950) + 1.0/298.15) - 2730;
            
            if (g_AO_BMS.Output.BMSTemperature[0] < -500)
            {
                Protect_SetFaultCodeLv2(&g_Protect, WARNING_BMS_NTC_FAILURE);
            }   

            // NTC�ͺţ�Res = 10K�� B = 3950
            g_AO_BMS.Output.BMSTemperature[1] = (s32)10.0 / (((log(g_ADC.Output.BalanceBoardNTCRes[1]) - log(10000))/3950) + 1.0/298.15) - 2730;
            
            if (g_AO_BMS.Output.BMSTemperature[1] < -500)
            {
                Protect_SetFaultCodeLv2(&g_Protect, WARNING_BMS_NTC_FAILURE);
            }  
            
            if(g_AO_BMS.Output.BMSTemperature[0] > g_AO_BMS.Output.BMSTemperature[1])
            {
                g_AO_BMS.Output.BMSTemperatureHi = g_AO_BMS.Output.BMSTemperature[0];
            }
            else
            {
                g_AO_BMS.Output.BMSTemperatureHi = g_AO_BMS.Output.BMSTemperature[1];
            }
        }
        else
        {
            g_AO_BMS.Output.BMSTemperature[0] = 0;
            g_AO_BMS.Output.BMSTemperature[1] = 0;
        }     	
#elif (CONTROLLER_TARGET == BMS_EP_20_REV1_1_2TEMP)
		
		if (g_SystemParameter.BMS.Warning.BMSOTemperatureCheckEnable > 0)			// BMS������¶ȼ��ʹ��
		{
			// 20AH�����棬��һ·�¶Ȳ��������ڲɼ�BMS�����¶ȣ���ʱ���ɼ���ʾ����������
			// 200ms����һ��
			// NTC�ͺţ�Res = 10K�� B = 3950
			g_AO_BMS.Output.BMSTemperature[0] = (s32)10.0/(((log(g_ADC.Output.NTCRes2)-log(10000))/3950) + 1.0/298.15) - 2730;
			
			if (g_ADC.Output.ADCResult[1] > 4050)
			{
				Protect_SetFaultCodeLv1(&g_Protect, WARNING_BMS_NTC_FAILURE);
			}	
            g_AO_BMS.Output.BMSTemperatureHi = g_AO_BMS.Output.BMSTemperature[0];
		}
		else
		{
			g_AO_BMS.Output.BMSTemperature[0] = 0;
		}
		
#elif (CONTROLLER_TARGET == BMS_EP_200_B2_1)		// ��Ӳ���汾BMS������¶Ȳ���LMT87���
		
		if (g_SystemParameter.BMS.Warning.BMSOTemperatureCheckEnable > 0)			// BMS������¶ȼ��ʹ��
		{
			// LMT87�¶ȴ�������ת����ʽ��T = -0.0718*LMT87Voltage + 189.16
			// ע���¶ȵ�λΪ0.1���϶�
			g_AO_BMS.Output.BMSTemperature = 1892 - (s32)g_ADC.Output.LMT87Voltage * 718 / 1000;
			
			if (g_ADC.Output.ADCResult[2] > 4050 || g_ADC.Output.ADCResult[2] < 50)
			{
				Protect_SetFaultCodeLv2(&g_Protect, WARNING_BMS_NTC_FAILURE);
			}			
		}
		else
		{
			g_AO_BMS.Output.BMSTemperature = 0;
		}				
		
#endif				// CONTROLLER_TARGET == BMS_EP_20_REV1_1_2TEMP
				
	}
	
	if (l_1sTimerFlag > 0)				// 1s��ʱʱ��
	{
		l_1sTimerFlag = 0;		

#if (CONTROLLER_TARGET == BMS_EP_200_B2_1)		// ��Ӳ��������LED
		
		// ���Ե���˸
		if (g_Output.State.bit.TestLED == 0)
		{
			Output_TestLED_Update(&g_Output, 1);
		}
		else
		{
			Output_TestLED_Update(&g_Output, 0);
		}
		
#endif
		
		
	}
	
	// ����������ʱ����BMS����FAULT_SIG
	if (l_FaultSIGSendFlag == 0)
	{
		if (Protect_GetFaultCode(&g_Protect) > 0)
		{
			QACTIVE_POST((QActive *)&g_AO_BMS, FAULT_SIG, Protect_GetFaultCode(&g_Protect));
			
			l_FaultSIGSendFlag = 1;
		}
	}
	else
	{
		if (Protect_GetFaultCode(&g_Protect) == 0)
		{
			l_FaultSIGSendFlag = 0;
		}
	}
	
	// for test�� ����flash EEPROMģ��
	if (t_flashEEPROMtest[0] > 0)
	{
		t_flashEEPROMtest[0] = 0;
		
		QACTIVE_POST((QActive *)&g_AO_EEPROM, EEPROM_WRITE_SIG, t_flashEEPROMtest[1]);	
	}
						
//	LCD_Process(&g_LCD);					// LCDģ����̺���
	
	flash_EEPROM_Process(&g_flash_EEPROM);		// flashģ��EEPROMģ����̺���
	
	// for test:
/*	if (g_flash_EEPROM.State.State_bits.bit.Command > 0)
	{
		Output_Out1_Update(1);
	}
	else
	{
		Output_Out1_Update(0);
	}*/
	
//	EEPROM_SIG_Update();					// EEPROM���ģ���¼����ͺ���
	
/*	if (l_10msTimerFlag > 0)
	{
	//	I2C_EEPROM_Process(&g_I2C_EEPROM);		// I2C EEPROMģ����̺���
		
		System_10msTimerUpdate();					// 10ms����ʱ�����º���
		
		l_10msTimerFlag = 0;
	}*/
	
	
    // ������bootloader������λ������ENTER_BOOTLOADER_SIG�¼���������˱�־λ
//    if (g_SystemState.State.bit.EnterBootloaderAsk > 0)
//    {
//    
//        // ��BMS״̬�����ͽ���Bootloader�¼�
//        QACTIVE_POST((QActive *)&g_AO_BMS, ENTER_BOOTLOADER_SIG, 0);	

//        // ������ɣ������־λ
//        g_SystemState.State.bit.EnterBootloaderAsk = 0;        
//    }
    
	
	
	
}

// ------------------------ 10ms����ʱ�����º��� -----------------------------
void System_10msTimerUpdate(void)
{

}

// End of system_core.c

