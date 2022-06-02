/* ==================================================================================

 File name:     LED_flicker.c
 Originator:    BLJ
 Description:   控制单个LED灯，实现1�?�?级故障码输出功能

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 04-26-2016		Version 1.0.0			正式版本
-----------------------------------------------------------------------------------*/


#include "system_core.h"								// 系统核心头文�?


// 点亮相应LED
void LED_Set(void)
{
	
#if (CONTROLLER_TARGET == BMS_EP_A5 || CONTROLLER_TARGET == BMS_EP_20_REV1_1_2TEMP || CONTROLLER_TARGET == BMS_EP_ALPHA || CONTROLLER_TARGET == BMS_EP_200_A2 || CONTROLLER_TARGET == BMS_EP_200_B1 || CONTROLLER_TARGET == BMS_EP_200_B2_1)					
			
	Output_LED_GREEN_Update(&g_Output, 1);
	
#endif				

}

// 熄灭相应LED
void LED_Clear(void)
{
	
#if (CONTROLLER_TARGET == BMS_EP_A5 || CONTROLLER_TARGET == BMS_EP_20_REV1_1_2TEMP || CONTROLLER_TARGET == BMS_EP_ALPHA || CONTROLLER_TARGET == BMS_EP_200_A2 || CONTROLLER_TARGET == BMS_EP_200_B1 || CONTROLLER_TARGET == BMS_EP_200_B2_1)					
			
	Output_LED_GREEN_Update(&g_Output, 0);
	
#endif					

}

// ------------------------------ LED闪烁输出更新 ------------------------------------
void LED_Flicker_Update(LED_Flicker_structDef* v)
{
	v->DelayCnt++;

	if (v->DelayCnt > v->DelayLength)
	{
		if (v->LEDStep < v->Input.LEDLength1)
		{
			if (v->LEDState == LED_OFF)
			{
				LED_Set();
				v->DelayLength = v->LEDONLength;			//Delay 600ms

				v->LEDState = LED_ON;
			}
			else
			{
				LED_Clear();
				v->DelayLength = v->LEDOFFLength;			//Delay 400ms

				v->LEDState = LED_OFF;
				v->LEDStep++;
			}
		}
		else if (v->LEDStep == v->Input.LEDLength1)
		{
			LED_Clear();
			v->DelayLength = v->LEDWaitLength1;				//Delay 1000ms

			v->LEDStep++;
		}
		else if (v->LEDStep <  v->Input.LEDLength1 + v->Input.LEDLength2 + 1)
		{
			if (v->LEDState == LED_OFF)
			{
				LED_Set();
				v->DelayLength = v->LEDONLength;			//Delay 600ms

				v->LEDState = LED_ON;
			}
			else
			{
				LED_Clear();
				v->DelayLength = v->LEDOFFLength;			//Delay 400ms

				v->LEDState = LED_OFF;
				v->LEDStep++;
			}
		}
//		else if (v->LEDStep == v->Input.LEDLength1 + v->Input.LEDLength2 + 1)
//		{
//			LED_Clear();
//			v->DelayLength = v->LEDWaitLength1;				//Delay 1000ms

//			v->LEDStep++;
//		}		
//		else if (v->LEDStep <  v->Input.LEDLength1 + v->Input.LEDLength2 + v->Input.LEDLength3 + 2)
//		{
//			if (v->LEDState == LED_OFF)
//			{
//				LED_Set();
//				v->DelayLength = v->LEDONLength;			//Delay 600ms

//				v->LEDState = LED_ON;
//			}
//			else
//			{
//				LED_Clear();
//				v->DelayLength = v->LEDOFFLength;			//Delay 400ms

//				v->LEDState = LED_OFF;
//				v->LEDStep++;
//			}
//		}		
		else
		{
			LED_Clear(); 
			v->DelayLength = v->LEDWaitLength2;				//Delay 3s

			v->LEDStep = 0;
		}

		v->DelayCnt = 0;
	}
}


// End of LED_Flicker.c

